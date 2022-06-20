#ifndef CHUNK_H
#define CHUNK_H

#include "a.h"

#define MAX_K_CONSTS UINT8_MAX // max constants allowed per chunk

#define GROW_CAPACITY(x) (0 == (x) ? 8 : (x) * 2)

typedef struct chunk {
    // bytecode
    uint8_t  *code;  
    uint16_t codeCount;
    uint16_t codeCapacity;

    // literals that appear in the source code 
    K        *k;      // array of K object (literals)
    uint8_t  kSize;   // capacity of the K array
    uint8_t  kCount;  // number of K literals
    
    // parse
    K        parseTree;

    // compile
    bool     compileError;
} Chunk;

// public function declarations
Chunk* initNewChunk();
void freeChunk(Chunk *chunk);
void growArray(Chunk *chunk, size_t size);

#endif
