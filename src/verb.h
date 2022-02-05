#ifndef VERB
#define VERB

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
K key(K x,K y);
D dyads[11];

// monadic verbs
K flip(K x);
M monads[1];

#endif
