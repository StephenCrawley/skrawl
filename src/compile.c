#include "a.h"
#include "compile.h"
#include "chunk.h"
#include "object.h"
#include "token.h"

static void addByte(Chunk *chunk, uint8_t code){
    if(chunk->codeCount == chunk->codeCapacity){
        growArray(chunk, sizeof(*chunk->code));
    }
    chunk->code[chunk->codeCount] = code;
    chunk->codeCount++;
}

// add a constant load instruction to the code
// the instruction is followed by the constant's index in chunk->k
static void addConstant(Chunk *chunk, K x){
    addByte(chunk, OP_CONSTANT);
    chunk->k[chunk->kCount] = ref(x);
    addByte(chunk, (uint8_t)chunk->kCount);
    chunk->kCount++;
}

// dyadic arithmetic operators are executed with the OP_DYAD opcode
// the char value is an index into the dyad[] array in verb.c
static void compileDyad(Chunk *chunk, K x){
    uint8_t opcode = (uint8_t) xc[0];
    addByte(chunk, opcode);
}

static void compileMonad(Chunk *chunk, K x){
    if (TOKEN_COMMA == xc[0]){
        addByte(chunk, OP_ENLIST);
        return;
    }
    uint8_t opcode = OP_MONAD_START + (uint8_t) xc[0];
    addByte(chunk, opcode);
}

static void compileLeaf(Chunk *chunk, K x){
    if (KV == xt){
        compileDyad(chunk, x);
    }
    else if (KU == xt){
        compileMonad(chunk, x);
    }
    else if (KI == ABS(xt) || KF == ABS(xt) || KC == ABS(xt) || KS == ABS(xt) || KN == ABS(xt)){
        addConstant(chunk, x);
    }
    else {
        printf("Compile error! Unrecognised op.\n");
        return;
    }
}

static void compileNode(Chunk *chunk, K x){
    // if the input is NOT a general list
    // i.e. it's a terminal value
    if (KK != xt){
        compileLeaf(chunk, x);
        return;
    }
    
    // else we're compiling an expression
    // loop over each element, right to left
    for (int8_t i = xn-1; i >= 0; --i){
        // if general list, recurse
        if (KK == TYPE(xk[i])){
            compileNode(chunk, xk[i]);
        }
        else {
            compileLeaf(chunk, xk[i]);
        }
    }

    // OP_ENLIST immediately encodes the number of elements to pop and enlist after the instruction
    if (OP_ENLIST == chunk->code[chunk->codeCount - 1]){
        addByte(chunk, (uint8_t) xn-1);
    }
}

static void compileExpressions(Chunk *chunk, K x){
    if(KK == xt && -KC == TYPE(xk[0]) && ';' == CHAR(xk[0])[0]){ // if ; separated expressions
        for(uint64_t i = 1; i < xn; ++i)
            compileNode(chunk, xk[i]);
    }
    else {
        compileNode(chunk, x);
    }
}

bool compile(Chunk *chunk){
    compileExpressions(chunk, chunk->parseTree);
    addByte(chunk, OP_RETURN);
    return true;
}
