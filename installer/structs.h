#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
    unsigned char *data;
    int len;
    int alloc_size;
    void* (*memcpy)(void * dest, const void * src, int num);
} file_struct_t;

typedef struct {
    unsigned char *data_elf;
    unsigned int coreinit_handle;
    /* function pointers */
    void* (*memcpy)(void * dest, const void * src, int num);
    void* (*memset)(void * dest, unsigned int value, unsigned int bytes);
    void* (*OSEffectiveToPhysical)(const void*);
    void* (*MEMAllocFromDefaultHeapEx)(unsigned int size, unsigned int align);
    void  (*MEMFreeToDefaultHeap)(void *ptr);
    void  (*DCFlushRange)(const void *addr, unsigned int length);
    void  (*ICInvalidateRange)(const void *addr, unsigned int length);
    void  (*_Exit)(void);

    void* (*curl_easy_init)(void);
    void  (*curl_easy_setopt)(void *handle, unsigned int param, const void *op);
    int   (*curl_easy_perform)(void *handle);
    void  (*curl_easy_getinfo)(void *handle, unsigned int param, void *op);
    void  (*curl_easy_cleanup)(void *handle);
} private_data_t;


#endif // STRUCTS_H
