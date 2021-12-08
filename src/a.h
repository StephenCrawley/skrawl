#ifndef COMMON
#define COMMON

#include <stdio.h>  // printf
#include <stdlib.h> // malloc
#include <string.h> // strtoll+strtod
typedef char C,*S;typedef unsigned char G;typedef int I;typedef long long J;typedef double F;typedef void V;
typedef struct k{C t;I r;J n;G d[];}*K;              // type,refcount,length,data
#define DO(n,x)    {J i=0,_n=(n);for(;i<_n;++i){x;}} // for
#define W(p,x)     while(p){x;}                      // while
#define CS(i,a)    case i:{a;}                       // case
#define P(x,y)     {if(x)R(y);}                      // panic!
#define OZ(s,m)    ((J)&(((s*)0)->m))                // offsetof
#define SZ         sizeof
#define O          printf
#define R          return
#define MAX(X,Y)   ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y)   ((X) < (Y) ? (X) : (Y))
#define ABS(X)     (0 == (X) ? 0 : MAX((X),-(X)))
#define SGN(X)     ((X) > 0 ? 1 : (X) < 0 ? -1 : 0)

#define ZI static I
#define ZC static C
#define ZV static V
#define ZK static K
// accessors
#define xT(x) ((x)->t)     // type
#define xN(x) ((x)->n)     // count
#define xR(x) ((x)->r)     // refc
#define xD(x) ((x)->d)     // data
#define xJ(x) ((J*)xD(x))  // long
#define xF(x) ((F*)xD(x))  // float
#define xC(x) ((C*)xD(x))  // char
#define xK(x) ((K*)xD(x))  // char
#define xt    xT(x)
#define xn    xN(x)
#define xr    xR(x)
#define yt    xT(y)
#define yn    xN(y)
// types 1 i(int),2 f(float),3 c(char)  nyi: 4 n(name),5 d(date) 
#define KK 0  // generic
#define KJ 1  // long   
#define KF 2  // float
#define KC 3  // char
#define KN 7  // null   
#define KQ 8  // end    
// nulls
#define NJ ((J)0x8000000000000000LL)
#define NF (0/0.0)

#define err(k) (-128==xT(k))
#define E_NYI   kerr("'nyi")
#define E_LEN   kerr("'len")
#define E_TYP   kerr("'typ")
#define E_RNK   kerr("'rnk")

#endif