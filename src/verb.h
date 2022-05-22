#ifndef VERB_H
#define VERB_H

#include "a.h"
#include "object.h"

// dyadic verbs
K add(K x,K y);
K subtract(K x,K y);
K multiply(K x,K y);
K divide(K x,K y);
K max(K x,K y);
K min(K x,K y);
K less(K x,K y);
K more(K x,K y);
K equal(K x,K y);
K match(K x,K y);
K key(K x,K y);
K find(K x,K y);
V dyads[13];

// monadic verbs
K flip(K x);
K first(K x);
K negate(K x);
K squareRoot(K x);
K enumerate(K x);
K reverse(K x);
K where(K x);
K asc(K x);
K desc(K x);
K not(K x);
K enlist(K x); //not currently in monads[] array
U monads[12];

#endif
