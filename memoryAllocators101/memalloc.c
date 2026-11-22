// Linux: gcc -o memalloc.so -fPIC -shared memalloc.c && LD_PRELOAD=$PWD/memalloc.so <program>
// buat tes cross-platform (LD_PRELOAD agak unreliable di macOS): make test
//
// nambahin: tiap panggilan malloc/calloc/realloc/free dicatet ke log, terus
// pas program exit dicek ada blok yang kelupaan di-free apa nggak. intinya
// mau tau: allocator bikinan sendiri ini, cocok gak hitungannya sama
// pemakaian program aslinya?

#include <stdio.h>
#include <stdlib.h>        // ← For size_t, NULL, sbrk()
#include <string.h>        // ← For memset(), memcpy()
#include <pthread.h>       // ← For pthread_mutex_t
#include <unistd.h>        // ← For sbrk(), write()
#include <fcntl.h>          // ← For open()
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

struct header_t {
    size_t size;
    unsigned is_free;
    struct header_t *next;
};

typedef char ALIGN[16];

union header {
    struct{
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    ALIGN stub;
};
typedef union header header_t;

header_t *head, *tail;

pthread_mutex_t global_malloc_lock;

/* ---- telemetry ----
 * logging-nya lewat write(2) langsung ke fd, bukan pake fprintf - soalnya
 * FILE* itu alokasi buffer-nya lewat malloc() juga, which is US, jadi bisa
 * deadlock sendiri di global_malloc_lock. mending raw fd aman. */
static int log_fd = -1;
static unsigned long g_alloc_count = 0;
static unsigned long g_free_count = 0;
static size_t g_bytes_requested = 0;

static void memalloc_log(const char *op, size_t size, void *ptr){
    if(log_fd < 0) return;
    char buf[128];
    int n = snprintf(buf, sizeof(buf), "%s size=%zu ptr=%p\n", op, size, ptr);
    if(n > 0) write(log_fd, buf, (size_t)n);
}

// cek blok mana yang masih is_free=0 pas program mau exit - itu artinya
// kelupaan di-free alias leak
static void memalloc_report(void){
    size_t leaked_bytes = 0;
    unsigned long leaked_blocks = 0;
    header_t *curr = head;
    while(curr){
        if(!curr->s.is_free){
            leaked_bytes += curr->s.size;
            leaked_blocks++;
        }
        curr = curr->s.next;
    }
    char buf[256];
    int n = snprintf(buf, sizeof(buf),
        "[memalloc] mallocs=%lu frees=%lu bytes_requested=%zu leaked_blocks=%lu leaked_bytes=%zu\n",
        g_alloc_count, g_free_count, g_bytes_requested, leaked_blocks, leaked_bytes);
    if(n > 0) write(STDERR_FILENO, buf, (size_t)n);
}

__attribute__((constructor))
static void memalloc_init(void){
    const char *path = getenv("MEMALLOC_LOG");
    if(!path) path = "memalloc.log";
    log_fd = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    atexit(memalloc_report);
}
/* ---- end telemetry ---- */

header_t *get_free_block(size_t size){
    header_t *curr = head;
    while(curr){
        if(curr->s.is_free && curr->s.size >= size) return curr;
        curr = curr->s.next;
    }
    return NULL;
}

void *malloc(size_t size){
    size_t total_size;
    void *block;
    header_t *header;
    if(!size) return NULL;
    pthread_mutex_lock(&global_malloc_lock);
    g_alloc_count++;
    g_bytes_requested += size;
    header = get_free_block(size);
    if(header){
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        memalloc_log("malloc(reuse)", size, (void*)(header + 1));
        return (void*)(header + 1);
    }
    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);
    if(block == (void*)-1){
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if(!head) head = header;
    if(tail) tail->s.next = header;
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);
    memalloc_log("malloc(new)", size, (void*)(header + 1));
    return (void*)(header + 1);
}

void free(void *block){
    header_t *header, *tmp;
    void *programbreak;

    if(!block) return;
    pthread_mutex_lock(&global_malloc_lock);
    g_free_count++;
    header = (header_t*)block - 1;

    programbreak = sbrk(0);
    if((char*)block + header->s.size == programbreak){
        if(head == tail){
            head = tail = NULL;
        }
        else{
            tmp = head;
            while(tmp){
                if(tmp->s.next == tail){
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        sbrk(0 - sizeof(header_t) - header->s.size);
        pthread_mutex_unlock(&global_malloc_lock);
        memalloc_log("free(shrink)", header->s.size, block);
        return;
    }
    header->s.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
    memalloc_log("free(mark)", header->s.size, block);
}

void *calloc(size_t num, size_t nsize){
    size_t size;
    void *block;
    if(!num || !nsize) return NULL;
    size = num * nsize;
    if(nsize != size / num) return NULL;
    block = malloc(size);
    if(!block) return NULL;
    memset(block, 0, size);
    memalloc_log("calloc", size, block);
    return block;
}

void *realloc(void *block, size_t size){
    header_t *header;
    void *ret;
    if(!block || !size) return malloc(size);
    header = (header_t*)block -1;
    if(header->s.size >= size) return block;
    ret = malloc(size);
    if(ret){
        memcpy(ret, block, header->s.size);
        free(block);
    }
    memalloc_log("realloc", size, ret);
    return ret;
}
