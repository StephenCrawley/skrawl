#ifndef OBJECT_H
#define OBJECT_H

#include "skrawl.h"

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
K ki(i64);
K kf(double);
K ks(i64);
K ku(char);
K kv(char);
K kw(i8);
K kwx(i8, K);
K km();
K squeeze(K);
K printK(K);

static inline K ke(){ return SET_TAG(KE,0); }

#ifdef DBG_WS
extern u64 WS;
extern u64 WT;
#endif

#endif
