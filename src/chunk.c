#include "a.h"
#include "chunk.h"
#include "object.h"

// create a chunk to contain compiled bytecode
Chunk* initNewChunk(){
    Chunk *chunk = malloc(sizeof(struct chunk));
    chunk->code = NULL;
    chunk->codeCount = 0;
    chunk->codeCapacity = 0;
    chunk->k = malloc(sizeof(K) * MAX_K_CONSTS); 
    chunk->kCount = 0;
    chunk->kSize = MAX_K_CONSTS;
    chunk->parseTree = NULL;
    chunk->compileError = false;
    return chunk;
}

void freeChunk(Chunk *chunk){
    // free the array of constants
    while (chunk->kCount--) unref(chunk->k[chunk->kCount]);
    free(chunk->k);

    // free the bytecode
    free(chunk->code);

    // free the parseTree
    if (NULL != chunk->parseTree) unref(chunk->parseTree);

    // free the chunk
    free(chunk);
}

void growArray(Chunk *chunk, size_t size){
    // newCapacity = 0==oldCapacity ? 8 : 2*old
    uint16_t newCapacity = GROW_CAPACITY(chunk->codeCapacity);
    uint8_t *ptr = realloc(chunk->code, (size_t)newCapacity * size);
    if (NULL == ptr){
        printf("FATAL : couldn't realloc bytecode array.\nExiting...\n");
        exit(1);
    }
    chunk->code = ptr;
    chunk->codeCapacity = newCapacity;
}
