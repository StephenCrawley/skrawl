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
K key(K x,K y);

// monadic verbs
K flip(K x);

D dyads[8];

#endif
