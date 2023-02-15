#ifndef VERB_H
#define VERB_H

#include "skrawl.h"

typedef K (*DYAD)(K,K);

K set(K,K);
K add(K,K);
K subtract(K,K);
K multiply(K,K);
K divide(K,K);
K join(K,K);
K find(K,K);
K dotApply(K,K);
K atApply(K,K);
K key(K,K);
K cast(K,K);
K take(K,K);
K drop(K,K);
K fill(K,K);
K min(K,K);
K equal(K,K);
K lessThan(K,K);
K greaterThan(K,K);
K match(K,K);
K max(K,K);
extern DYAD dyad_table[];

#endif
