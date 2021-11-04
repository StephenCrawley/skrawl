#ifndef OBJECT
#define OBJECT

#include "a.h"
//#include "p.h"

V pk(K x);   // print object
K k(I t,J n);K kj();K kf();K kjc(S s);K kjcn(S s,I n);K kfc(S s);K kfcn(S s,I n); // create object
K kfj(K x);  // cast
V r0(K x);   // destroy object if 0==r
V r1(K x);   // increment refcount
K kerr(S e); // error object

#endif