// https://brennan.io/2015/01/16/write-a-shell-in-c/
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void){
    // char *line = NULL:
    // ssize_t bufsize = 0;

    // if(getline(&line, &bufsize, stdin) == -1){
    //     if(feof(stdin)){
    //         exit(EXIT_SUCCESS);
    //     }
    //     else{
    //         perror("readline");
    //         exit(EXIT_FAILURE);
    //     }
    // }
    // return line;
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer){
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        c = getchar();
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        }
        else{
            buffer[position] = c;
        }
        position++;
        if(position >= bufsize){
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line){
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position >= bufsize){
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens){
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args){
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  int i;
  printf("From Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");
  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
  printf("Use the man command for information on other programs.\n");
  return 1;
}

int lsh_exit(char **args){
  return 0;
}

int lsh_launch(char **args){
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0){
        if(execvp(args[0], args) == -1){
            perror("lsh");
        }        
        exit(EXIT_FAILURE);
    }
    else if(pid < 0){
        perror("lsh");
    }
    else{
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

// tutorial aslinya cuma sampe 1 execvp() per baris, belum support pipe.
// jadi "ls | wc -l" bakal error kalo pake kode asli brennan.io. ini nambahin
// biar bisa di-pipe kaya shell beneran.
#define LSH_MAX_PIPE_CMDS 64

int lsh_split_pipeline(char **args, char **commands[LSH_MAX_PIPE_CMDS]){
    int cmd_count = 0;
    commands[cmd_count++] = args;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            if (cmd_count < LSH_MAX_PIPE_CMDS) {
                commands[cmd_count++] = &args[i + 1];
            }
        }
    }
    return cmd_count;
}

// bikin N-1 pipe buat N command, fork semuanya, terus tiap child dicolokin
// stdin/stdout nya ke pipe yang bener pake dup2 sebelum execvp.
int lsh_launch_pipeline(char **commands[], int cmd_count){
    int num_pipes = cmd_count - 1;
    int pipefds[2 * LSH_MAX_PIPE_CMDS];
    pid_t pids[LSH_MAX_PIPE_CMDS];

    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("lsh");
            return 1;
        }
    }

    for (int i = 0; i < cmd_count; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i != 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i != cmd_count - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }
            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipefds[j]);
            }
            if (commands[i][0] != NULL && execvp(commands[i][0], commands[i]) == -1) {
                perror("lsh");
            }
            exit(EXIT_FAILURE);
        } else if (pids[i] < 0) {
            perror("lsh");
        }
    }

    for (int j = 0; j < 2 * num_pipes; j++) {
        close(pipefds[j]);
    }

    int status;
    for (int i = 0; i < cmd_count; i++) {
        waitpid(pids[i], &status, 0);
    }
    return 1;
}

int lsh_execute(char **args){
  int i;
  char **commands[LSH_MAX_PIPE_CMDS];
  int cmd_count = lsh_split_pipeline(args, commands);

  if (args[0] == NULL) {
    return 1;
  }

  if (cmd_count > 1) {
    return lsh_launch_pipeline(commands, cmd_count);
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return lsh_launch(args);
}


void lsh_loop(void){
    char *line;
    char **args;
    int status;

    do{
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);
        free(line);
        free(args);
    } while(status);
}

int main(int argc, char **argv){
    lsh_loop();
    return EXIT_SUCCESS;
}