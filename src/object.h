#ifndef OBJECT_H
#define OBJECT_H

#include "skrawl.h"

// K header accessors
// the header is 16 bytes
// --mtrrrrnnnnnnnn
// - unused, m membucket, t type, r refcount, n count
#define MEM(x)   (( i8*)x)[-14]
#define TYP(x)   (( i8*)x)[-13]
#define REF(x)   ((i32*)x)[-3]
#define CNT(x)   ((i64*)x)[-1]

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
K printK(K);

extern u64 WS;

#endif
