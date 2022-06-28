#ifndef PARSE_H
#define PARSE_H

#include "a.h"
#include "token.h"
#include "chunk.h"

typedef struct {
    Token current;
    K     prefix;
    bool  compose;
    bool  panic;
} Parser;

K parse(const char *source);

#endif
