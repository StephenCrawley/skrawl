#ifndef VERB
#define VERB

#include "a.h"
#include "object.h"

// dyadic verbs
K add(K x,K y);
K subtract(K x,K y);
K multiply(K x,K y);

#define DYADIC_INIT \
    int8_t r_type  = MAX(xt, yt);    \
    uint64_t r_count = MAX(xn, yn);  \
    K r = k(r_type, r_count); 

#endif
