#ifndef PARSE_H
#define PARSE_H

#include "a.h"
#include "token.h"
#include "chunk.h"

typedef struct {
    Token previous;
    Token current;
    bool  panic;
} Parser;

bool parse(const char *source, Chunk *chunk);

#endif
