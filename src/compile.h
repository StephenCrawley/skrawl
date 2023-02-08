#ifndef COMPILE_H
#define COMPILE_H

#include "skrawl.h"

enum {
    OP_MONAD=0x00,
    OP_DYAD=0x20,
    OP_CONSTANT=0x40,
    OP_APPLY_N,
    OP_POP,
    OP_RETURN,
};

K compile(K);

#endif
