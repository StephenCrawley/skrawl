#ifndef VERB
#define VERB

#include "a.h"
#include "p.h"

// adverb
K fld(K(*f)(K,K),K x,G s);K fld2(K (*f)(K,K),K x,K y);K each(K (*f)(K),K x);K each2(K (*f)(K,K),K x,K y);K scan(K (*f)(K,K),K x,K y);
K eachr(K (*f)(K,K),K x,K y);K eachl(K (*f)(K,K),K x,K y);K eachp(K (*f)(K,K),K x,K y);K eachp_(K(*f)(K,K),K x);
// monad
K neg(K x);K not(K x);K til(K x);K typ(K x);K enl(K x);K len(K x);K frs(K x);K whr(K x);K rev(K x);K str(K x);
//dyad
K sum(K x,K y);K prd(K x,K y);K dvd(K x,K y);K sub(K x,K y);K eq(K x,K y);K lt(K x,K y);K gt(K x,K y);K set(T *t,K x);K get(T *t);K cat(K x,K y);
K at(K x,K y);K bng(K x,K y);K and(K x,K y);K or(K x,K y);K mod(K x,K y);K take(K x,K y);K drop(K x,K y);K mtc(K x,K y);

#define MONAD_INIT(maxt,rt) P(xt>maxt,E_TYP); K z=k(rt,xn);

#define DYAD_INIT(f, maxt)            \
 P((0>xt||0>yt)&&((KK==xt&&0==xn)||(KK==yt&&0==yn)),(r0(x),r0(y),k(KK,0))) \
 P((xt>=0&&yt>=0)&&(xn!=yn),E_LEN)    \
 J zn=MAX(xn,yn);                     \
 /* if generic type */                \
 if(KK==xt||KK==yt){K z=k(KK,zn);     \
  if     (KK==xt&&KK==yt){DO(zn,xK(z)[i]= f(r1(xK(x)[i]),r1(xK(y)[i])))} \
  else if(KK==xt&&0 < yt){K b=kxp(r1(y));DO(zn,xK(z)[i]= f(r1(xK(x)[i]),r1(xK(b)[i])));r0(b);} \
  else if(0 < xt&&KK==yt){K a=kxp(r1(x));DO(zn,xK(z)[i]= f(r1(xK(a)[i]),r1(xK(y)[i])));r0(a);} \
  else if(KK==xt&&0 > yt){DO(zn,xK(z)[i]= f(r1(xK(x)[i]),r1(y)));} \
  else if(0 > xt&&KK==yt){DO(zn,xK(z)[i]= f(r1(x),r1(xK(y)[i])));} \
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
 else if(KJ==ABS(xt) && KC==ABS(yt)){DYAD_EXEC_OP(op,xJ,xC,za)} \
 else if(KC==ABS(xt) && KJ==ABS(yt)){DYAD_EXEC_OP(op,xC,xJ,za)} \
 else if(KC==ABS(xt) && KC==ABS(yt)){DYAD_EXEC_OP(op,xC,xC,za)} \
 else   {r0(x);r0(y);R E_NYI;}

#define DYAD_EXEC(f,maxt,op) DYAD_INIT(f,maxt);if(KJ==ABS(zt)){DYAD_EXEC_ZA(op,xJ)}else if(KC==ABS(zt)){DYAD_EXEC_ZA(op,xC)}else{DYAD_EXEC_ZA(op,xF)}

#endif