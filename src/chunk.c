#include "a.h"
#include "chunk.h"
#include "object.h"

// create a chunk to contain compiled bytecode
Chunk* initNewChunk(){
    Chunk *chunk = malloc(sizeof(struct chunk));
    chunk->code = NULL;
    chunk->codeCount = 0;
    chunk->codeCapacity = 0;
    chunk->k = malloc(sizeof(K) * 8); // TODO : make this dynamic
    chunk->kCount = 0;
    chunk->kSize = 8;
    chunk->parseTree = NULL;
    return chunk;
}

void freeChunk(Chunk *chunk){
    // free the array of constants
    //for (int i = 0; i < chunk->kCount; ++i) free(chunk->k[i]);
    free(chunk->k);

    // free the bytecode
    free(chunk->code);

    // free the parseTree
    if(NULL != chunk->parseTree) unref(chunk->parseTree);

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
