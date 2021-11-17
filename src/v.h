#ifndef VERB
#define VERB

#include "a.h"
#include "p.h"

// adverb
K fld2(K (*f)(K,K),K x,K y);K each(K (*f)(K,K),K x,K y);
// monad
K neg(K x);K not(K x);K til(K x);K typ(K x);K enl(K x);K len(K x);K frs(K x);K whr(K x);K rev(K x);
//dyad
K sum(K x,K y);K prd(K x,K y);K dvd(K x,K y);K sub(K x,K y);K eq(K x,K y);K lt(K x,K y);K gt(K x,K y);K set(T *t,K x);K get(T *t);K cat(K x,K y);
K at(K x,K y);K bng(K x,K y);K and(K x,K y);K or(K x,K y);K mod(K x,K y);K take(K x,K y);

#define MONAD_INIT(maxt,rt) \
 P(xt>maxt,E_TYP)           \
 K z=k(rt,xn);

#define DYAD_INIT(f, maxt)            \
 P((xt>=0&&yt>=0)&&(xn!=yn),E_LEN)    \
 J zn=MAX(xn,yn);                     \
 /* if generic type */                \
 if(KK==xt||KK==yt){K z=k(KK,zn);     \
  if     (KK==xt&&KK==yt){DO(zn,r1(xK(x)[i]);r1(xK(y)[i]);xK(z)[i]= f(xK(x)[i],xK(y)[i]))} \
  else if(KK==xt&&0 < yt){r1(y);K b=kxp(y);DO(zn,r1(xK(x)[i]);r1(xK(b)[i]);xK(z)[i]= f(xK(x)[i],xK(b)[i]));r0(b);} \
  else if(0 < xt&&KK==yt){r1(x);K a=kxp(x);DO(zn,r1(xK(a)[i]);r1(xK(y)[i]);xK(z)[i]= f(xK(a)[i],xK(y)[i]));r0(a);} \
  else if(KK==xt&&0 > yt){DO(zn,r1(xK(x)[i]);r1(y);xK(z)[i]= f(xK(x)[i],y));} \
  else if(0 > xt&&KK==yt){DO(zn,r1(x);r1(xK(y)[i]);xK(z)[i]= f(x,xK(y)[i]));} \
  else   {r0(x);r0(y);R E_TYP;} \
  r0(x);r0(y);R z;}                   \
 /* else if simple type */            \
 C zt=MAX(ABS(xt),ABS(yt));           \
 zt=(0>xt && 0>yt)?-zt:zt;            \
 zt=MIN(maxt,zt);                     \
 K z=k(zt,zn);

#define DYAD_EXEC_OP(op, xa, ya, za) /* op, x/y/z accessor */ \
 if     (xn==yn){DO(zn,za(z)[i]= op( xa(x)[i] , ya(y)[i] ))} \
 else if (xn>yn){DO(zn,za(z)[i]= op( xa(x)[i] , ya(y)[0] ))} \
 else           {DO(zn,za(z)[i]= op( xa(x)[0] , ya(y)[i] ))} \
 r0(x);r0(y);

#define DYAD_EXEC_ZA(op, za)/*specify z accessor*/\
 if     (KJ==ABS(xt) && KJ==ABS(yt)){DYAD_EXEC_OP(op,xJ,xJ,za)} \
 else if(KJ==ABS(xt) && KF==ABS(yt)){DYAD_EXEC_OP(op,xJ,xF,za)} \
 else if(KF==ABS(xt) && KJ==ABS(yt)){DYAD_EXEC_OP(op,xF,xJ,za)} \
 else if(KF==ABS(xt) && KF==ABS(yt)){DYAD_EXEC_OP(op,xF,xF,za)} \
 else   {r0(x);r0(y);R E_NYI;}

#define DYAD_EXEC(f,maxt,op) DYAD_INIT(f,maxt);if(KJ==ABS(zt)){DYAD_EXEC_ZA(op,xJ)}else{DYAD_EXEC_ZA(op,xF)}

#endif