#include "a.h"
#include "compile.h"
#include "chunk.h"

static void addByte(Chunk *chunk, uint8_t code){
    if(chunk->codeCount == chunk->codeCapacity){
        growArray(chunk, sizeof(*chunk->code));
    }
    chunk->code[chunk->codeCount] = code;
    chunk->codeCount++;
}

// add a constant load instruction to the code
// the instruction instruction is followed 
static void addConstant(Chunk *chunk, K val){
    chunk->k[chunk->kCount] = val;
    addByte(chunk, (uint8_t)chunk->kCount);
    chunk->kCount++;
}

