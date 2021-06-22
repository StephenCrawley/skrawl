#ifndef COMMON
#define COMMON

#include <stdio.h>
#include <stdlib.h>
typedef char C,*S; typedef int I; typedef long long J; typedef double F; typedef void V;
typedef struct k{C t;J r,n,v[];}*K;                       //<! type,refcount,length,data
#define DO(n,x)    {J i=0,_n=(n);for(;i<_n;++i){x;}}      //<! for
#define DO2(n,x)   {J i=0,j=(n)-1;for(;j>=0;++i,--j){x;}} //<! forward+backward for
#define CS(i,a...) case i:{a;}                            //<! case
#define W(p,x)     while(p){x;}                           //<! while
#define SZ         sizeof
#define Z          static
#define O          printf
#define R          return

#endif