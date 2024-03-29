#ifndef OBJECT_H
#define OBJECT_H

#include "a.h"

#define KNUL ( k(KN,0) )
#define UNREFTOP(x) { --REFC(x); if (1 > REFC(x)){ free(x); } } 

// increment/decrement refcount
K      ref(K x);
void unref(K x);

// K object gen
K k(int8_t type, uint64_t count);
K Ki(int64_t x);
K Kf(double x);
K Kc(char x);
K Ks(int64_t x);
K Kv(int8_t type, char c);
K Ka(I t, K x);
K Kp(K x,K y);
K Kq(K x, K y);
K Kerr(const char *error);

K squeeze(K x);
K expand(K x);

int8_t rankOf(K x);

// print K object
K printK(K x);
K printKObject(K x, bool printFlat);
K debugPrintK(K x);

// make a symbol from char array
// the chars are encoded directly into a uint64_t, meaning symbols can only be 8 bytes long max
// symbols less than 8 chars are padded with '\0'
#define MAKE_SYMBOL(dest, string, length) \
    dest = 0; \
    for (uint8_t _j = 0, _n = MIN(8, (length)); _j < _n; ++_j) \
        ((char *) &dest)[_j] = string[_j];

#endif
