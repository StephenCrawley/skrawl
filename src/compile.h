#ifndef COMPILE
#define COMPILE

#include "a.h"
#include "chunk.h"

// Bytecode Instructions
enum {
    // OP_DYAD_START and OP_DYAD_END is the range containing dyadic operator opcodes
    // opcodes in this range are used as an index into the dyads[] array defined in verbs.c
    // the index is 
    OP_DYAD_START  = 0x00,
    OP_DYAD_END    = 0x0F,
    // OP_MONAD_START and OP_MONAD_END is the range containing monadic operator opcodes
    // opcodes in this range, minus OP_MONAD_START, are used as an index into the monads[] array defined in verbs.c
    // eg monads[opcode - OP_MONAD_START]
    OP_MONAD_START = 0x10,
    OP_MONAD_END   = 0x1F,
    OP_ENLIST,
    OP_CONSTANT,
    OP_RETURN
};

// forward declarations
bool compile(Chunk *chunk);

#endif
