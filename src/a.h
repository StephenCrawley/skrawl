#ifndef COMMON
#define COMMON

#include <stdio.h>
#include <stdlib.h>
typedef char C,*S; typedef int I; typedef long long J; typedef double F; typedef void V;
typedef struct k{C t;I r;J n,v[];}*K;                     //<! type,refcount,length,data
#define DO(n,x)    {J i=0,_n=(n);for(;i<_n;++i){x;}}      //<! for
#define CS(i,a...) case i:{a;}                            //<! case
#define W(p,x)     while(p){x;}                           //<! while
#define Z          static
#define O          printf
#define R          return

#endif

/*
inspiration for above

kx k.h(partial) - typedefs and K object

typedef char*S,C;typedef unsigned char G;typedef short H;typedef int I;typedef long long J;typedef float E;typedef double F;typedef void V;

typedef struct k0 {
    signed char m,a,t;  // m,a are for internal use.
    C u;
    I r; 
    union {
        G  g; 
        H  h; 
        I  i; 
        J  j; 
        E  e; 
        F  f; 
        S  s;

        struct k0*k;
        struct {J  n;G  G0 [1];};
    }; 
} *K;

shakti k.h

#include<string.h>
typedef unsigned I;typedef long J;typedef double F;typedef unsigned char G,*S;typedef unsigned long U,K;
enum{KC=1,KS=2,KI=13,KJ,KT=21,KD=25,KF=43,XT=253,XD};

#define N(n,a...)   {int _=(n),i=0;while(i<_){a;++i;}}
#define S(i,c,a...) switch(i){c default:{a;}}
#define P(b,a...)   if(b)return({a;});
#define C(i,a...)   case i:{a;}break;
#define R           return

//atom: type value
#define tx (x>>56)
#define xi ((I)x)
#define xj *(J*)(0xffffffffffff&x)
#define xf *(F*)(0xffffffffffff&x)

//list: type count values
#define xt ((S)x)[-5]
#define yt ((S)y)[-5]
#define xn ((int*)x)[-1]
#define yn ((int*)y)[-1]
#define fn ((int*)f)[-1]
#define xI ((I*)x)
#define xJ ((J*)x)
#define xF ((F*)x)
#define xK ((K*)x)
#define yK ((K*)y)

K k(K,...);

K ki(I i){R k('i',i);}           K D(K x,K y){R k('a',x,y);}    
K kf(F f){R k('f',*(K*)&f);}     K T(K x,K y){R k('A',x,y);}
K kC(S s){I n=strlen((char*)s);R(K)memcpy((S)k('C',n),s,n);}
*/