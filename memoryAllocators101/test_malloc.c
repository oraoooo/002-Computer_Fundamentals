// langsung link ke allocator di memalloc.c, ga pake LD_PRELOAD (jadi sama
// aja perilakunya di macOS/Linux). sengaja gak di-free satu blok, biar
// exit report-nya bilang "leaked_blocks=1".
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void){
    char *a = malloc(32);
    strcpy(a, "hello");
    printf("a=%s\n", a);

    int *nums = calloc(10, sizeof(int));
    for (int i = 0; i < 10; i++) nums[i] = i * i;
    printf("nums[9]=%d\n", nums[9]);

    char *b = malloc(8);
    b = realloc(b, 64);
    strcpy(b, "grown block");
    printf("b=%s\n", b);

    char *leaked = malloc(16);
    strcpy(leaked, "oops");

    free(a);
    free(nums);
    free(b);
    // 'leaked' emang sengaja gak di-free

    return 0;
}
