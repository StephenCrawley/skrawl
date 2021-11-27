#ifndef OBJECT
#define OBJECT

#include "a.h"

V pk(K x);   // print object
K k(I t,J n);K kj();K kf();K kjc(S s);K kjcn(S s,I n);K kfc(S s);K kfcn(S s,I n);K kjx(J x);K kfx(F x); // create object
K kxp(K x);K sqz(K x);
K kfj(K x);  // cast
V r0(K x);   // decrement refcount. destroy object if 0==refcount
K r1(K x);   // increment refcount and return x
K kerr(S e); // error object

#endif