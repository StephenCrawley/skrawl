#ifndef OBJECT
#define OBJECT

#include "a.h"

V pk(K x);   // print object
K mc(K x,K y); // memcpy
K k(I t,J n);K kj();K kf();K kjc(S s);K kjcn(S s,I n);K kfc(S s);K kcc(S a,I l);K kfcn(S s,I n);K kjx(J x);K kfx(F x);K kcx(C x);K kd(); // create object
K kxp(K x);K sqz(K x);
K kfj(K x);  // cast
V r0(K x);   // decrement refcount. destroy object if 0==refcount
K r1(K x);   // increment refcount and return x
K kerr(S e); // error object

#endif