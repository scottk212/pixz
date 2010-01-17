#include <lzma.h>

#define __USE_LARGEFILE 1

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <pthread.h>


#pragma mark DEFINES

#define CHECK LZMA_CHECK_CRC32
#define MEMLIMIT (64L * 1024 * 1024 * 1024) // crazy high

#define CHUNKSIZE 4096


#pragma mark TYPES

typedef struct file_index_t file_index_t;
struct file_index_t {
    char *name;
    off_t offset;
    file_index_t *next;
};


typedef struct queue_item_t queue_item_t;
struct queue_item_t {
    int type;
    void *data;
    queue_item_t *next;
};

typedef struct {
    queue_item_t *first;
    queue_item_t *last;
    
    pthread_mutex_t mutex;
    pthread_cond_t pop_cond;
} queue_t;


#pragma mark GLOBALS

FILE *gInFile;
lzma_stream gStream;

extern lzma_index *gIndex;
extern file_index_t *gFileIndex, *gLastFile;


#pragma mark FUNCTION DECLARATIONS

void die(const char *fmt, ...);
char *xstrdup(const char *s);

uint64_t xle64dec(const uint8_t *d);
void xle64enc(uint8_t *d, uint64_t n);
size_t num_threads(void);

void decode_index(void);
void *decode_block_start(off_t block_seek);

void read_file_index(void);
void dump_file_index(void);
void free_file_index(void);

queue_t *queue_new(void);
void queue_free(queue_t *q);
// data should be on heap, if present
void queue_push(queue_t *q, int type, void *data);
int queue_pop(queue_t *q, void **datap);
