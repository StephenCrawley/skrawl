#ifndef OBJECT_H
#define OBJECT_H

#include "skrawl.h"

#define UNREF_X(a)   __extension__({__typeof__(a)_a=(a); unref(x), _a;})
#define UNREF_Y(a)   __extension__({__typeof__(a)_a=(a); unref(y), _a;})
#define UNREF_R(a)   __extension__({__typeof__(a)_a=(a); unref(r), _a;})
#define UNREF_XY(a)  __extension__({__typeof__(a)_a=(a); unref(x), unref(y), _a;})
#define UNREF_XR(a)  __extension__({__typeof__(a)_a=(a); unref(x), unref(r), _a;})
#define UNREF_XYR(a) __extension__({__typeof__(a)_a=(a); unref(x), unref(y), unref(r), _a;})

// public object functions
void unref(K);
K ref(K);
K tn(i8,i64);
K j2(K,K);
K jk(K,K);
K va(K);
K k1(K);
K k2(K,K);
K k3(K,K,K);
K kx(u8);
K kx2(u8,u8);
K kc(u8);
K kCn(char*,i64);
K kC0(char*);
K ki(i64);
K kf(double);
K ks(i64);
K kD(K,K);
K kT(K);
K ku(u64);
K kv(u64);
K kuc(char);
K kvc(char);
K kw(i8);
K kwx(i8,K);
K km();
K ke(K);
K kerr(char*);
K squeeze(K);
K expand(K);
K item(i64,K);
K sublist(K,i64,i64);
K printK(K);
u8 cverb(u8 i);
u8 cadverb(u8 i);
u64 iadverb(char c);

static inline K knul(){ return SET_TAG(KN,0); }

#ifdef DBG_WS
extern u64 WS;
extern u64 WT;
#endif

#endif
