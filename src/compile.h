#ifndef COMPILE_H
#define COMPILE_H

#include "skrawl.h"

enum {
    OP_MONAD=0x00,
    OP_DYAD=0x14,
    OP_ADVERB=0x28,
    OP_CONSTANT=0x2e,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_APPLY_N,
    OP_POP,
    OP_RETURN,
};

K compile(K);

#endif
