#ifndef OBJECT
#define OBJECT

#include "a.h"

#define KNUL ( k(KN,0) )

// increment/decrement refcount
K      ref(K x);
void unref(K x);

// K object gen
K k(int8_t type, uint64_t count);
K Ki(int64_t x);
K Kf(double x);
K Kc(char x);

K squeeze(K x);

// print K object
void printK(K x);

#endif
