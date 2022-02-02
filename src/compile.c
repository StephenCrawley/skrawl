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
    chunk->k[chunk->kCount] = x;
    addByte(chunk, (uint8_t)chunk->kCount);
    chunk->kCount++;
}

// dyadic arithmetic operators are executed with the OP_DYAD opcode
// the upper 3 bits encode the index into the dyads array of function pointers.
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

// walk the parse tree and emit bytecode
static void compileNode(Chunk *chunk, K x){
    if (KK == xt){
        for (int8_t i = xn-1; i >= 0; --i) compileNode(chunk, xk[i]); // TODO : set limits for source code literals
    }
    else if (KD == xt){
        compileDyad(chunk, x);
        return;
    }
    else if (KM == xt){
        compileMonad(chunk, x);
        return;
    }
    else if (KI == ABS(xt) || KF == ABS(xt) || KC == ABS(xt) || KS == ABS(xt) || KN == ABS(xt)){
        addConstant(chunk, x);
        return;
    }
    else {
        printf("Compile error! Unrecognised op.\n");
        return;
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
