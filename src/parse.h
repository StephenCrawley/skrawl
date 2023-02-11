#ifndef PARSE_H
#define PARSE_H

#include "skrawl.h"

typedef struct {bool compose; K error; const char *src,*current;} Parser;

K parse(const char*);
K readK();

#endif
