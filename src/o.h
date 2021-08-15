#ifndef OBJECT
#define OBJECT

#include "a.h"

V pk(K x); //<! print object
K k(I t,J n);K kj();K kf();K kjc(S s);K kfc(S s); //<! create object 
//V rf(K x); //<! destroy object
V r0(K x); //<! destroy object if 0==r
V r1(K x); //<! increment refcount
K kerr(S e); //<! error object

#endif