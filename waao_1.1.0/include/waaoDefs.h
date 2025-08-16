#ifndef WAAO_DEFS_H
#define WAAO_DEFS_H

#include <stddef.h>
#include <stdint.h>

#define WAAO_SUCCESS 0
#define WAAO_ERROR   -1

#define WAAO_PATH_MAX 4096
#define WAAO_IO_BUFSZ 65536

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} WaaoBuffer;

static inline void waaoBufInit(WaaoBuffer *b){ 
    b->data=NULL; 
    b->len=0; 
    b->cap=0; 
}
static inline void waaoBufFree(WaaoBuffer *b){ 
    if(b->data) {
        free(b->data); 
        b->data=NULL; 
        b->len=b->cap=0; 
    }
}

#endif
