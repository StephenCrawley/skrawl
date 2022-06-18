#ifndef ADVERB_H
#define ADVERB_H

#include "a.h"
#include "object.h"

W adverbs[6];
K over(K f, K x, K y);
K scan(K f, K x, K y);
K eachLeft(K f, K x, K y);
K eachRight(K f, K x, K y);

K getIdentity(K x);

#endif
