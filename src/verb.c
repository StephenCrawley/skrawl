#include "a.h"
#include "verb.h"

// dyadic verbs
K add(K x, K y){
    DYADIC_INIT; // declare return K object r, type rt, count rn
    ri[0] = xi[0] + yi[0];
    unref(x), unref(y);
    return  r;
}
