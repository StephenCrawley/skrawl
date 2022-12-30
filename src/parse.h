#ifndef PARSE_H
#define PARSE_H

#include "skrawl.h"
#include "object.h"

typedef struct {bool error,compose; char *src,*current;} Parser;

K parse(char*);

#endif
