#ifndef OBJECT_H
#define OBJECT_H

#include "skrawl.h"

#define UNREF_X(a) __extension__({__typeof__(a)_a=(a); unref(x), _a;})
#define UNREF_R(a) __extension__({__typeof__(a)_a=(a); unref(r), _a;})

// public object functions
void unref(K);
K ref(K);
K tn(i8, i64);
K j2(K, K);
K jk(K, K);
K va(K);
K k1(K);
K k2(K, K);
K k3(K, K, K);
K kx(u8);
K ki(i64);
K kf(double);
K ks(i64);
K ku(u64);
K kv(u64);
K kuc(char c);
K kvc(char c);
K kw(i8);
K kwx(i8, K);
K km();
K squeeze(K);
K printK(K);
u64 iadverb(char c);

static inline K   ke(){ return SET_TAG(KE,0); }
static inline K knul(){ return SET_TAG(KN,0); }

#ifdef DBG_WS
extern u64 WS;
extern u64 WT;
#endif

#endif
