#ifndef OBJECT_H
#define OBJECT_H

#include "skrawl.h"

// public object functions
void unref(K);
K ref(K);
K tn(i8, i64);
K j2(K, K);
K jk(K, K);
K k1(K);
K k2(K, K);
K k3(K, K, K);
K ki(i64);
K kf(double);
K ks(i64);
K ku(char);
K kv(char);
K kw(i8);
K kwx(i8, K);
K squeeze(K);
K printK(K);

extern u64 WS;

#endif
