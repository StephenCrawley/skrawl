#ifndef VERB_H
#define VERB_H

#include "skrawl.h"

typedef K (*DYAD)(K,K);

K apply(K,K);
K find(K,K);
K key(K,K);
extern DYAD dyad_table[];

#endif
