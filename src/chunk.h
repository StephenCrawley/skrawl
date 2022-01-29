#ifndef CHUNK
#define CHUNK

#include "a.h"

#define GROW_CAPACITY(x) (0 == (x) ? 8 : (x) * 2)

typedef struct chunk {
    // bytecode
    uint8_t  *code;  
    uint16_t codeCount;
    uint16_t codeCapacity;

    // literals that appear in the source code 
    K        *k;      // array of K object (literals)
    uint16_t kSize;   // capacity of the K array
    uint16_t kCount;  // number of K literals
    
    // parse
    K    parseTree;
} Chunk;

// public function declarations
Chunk* initNewChunk();
void freeChunk(Chunk *chunk);
void growArray(Chunk *chunk, size_t size);

#endif
