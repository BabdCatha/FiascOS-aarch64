#include "stdmem.h"

//TODO: Rework this to better handle overlapping memory areas
void memmove(const void* src, void* dst, int size){
    char* cdst = (char*)dst;
    char* csrc = (char*)src;

    int i = 0;

    for(i = 0; i < size; i ++){
        cdst[i] = csrc[i];
    }

    return;
}