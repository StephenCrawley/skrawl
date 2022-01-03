#ifndef COMPILE
#define COMPILE

#include "a.h"
#include "token.h"
#include "chunk.h"

enum {OP_CONSTANT, OP_ADD, OP_RETURN};

typedef struct {
    Token previous;
    Token current;
    bool  panic;
} Parser;

bool compile(const char *source, Chunk *chunk);

#endif
