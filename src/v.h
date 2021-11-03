#ifndef VERB
#define VERB

#include "a.h"
#include "p.h"
K neg(K x);K not(K x);K til(K x);K typ(K x);
K sum(K x,K y);K prd(K x,K y);K dvd(K x,K y);K sub(K x,K y);K eq(K x,K y);K gt(K x,K y);K set(T *t,K x);K get(T *t);K cat(K x,K y);

#define MONAD_INIT(maxt,rt) \
 P(xt>maxt,E_TYP)           \
 K z=k(rt,xn);

#define DYAD_INIT(maxt)            \
 P((xt>=0&&yt>=0)&&(xn!=yn),E_LEN) \
 J zn=MAX(xn,yn);                  \
 C zt=MAX(ABS(xt),ABS(yt));        \
 zt=MAX(maxt,zt);                  \
 K z=k(zt,zn);

#define DYAD_EXEC(op)  \
 /* TODO : refactor below */\
 /*a op a*/ \
 if     (-KJ==xt && -KJ==yt){xJ(z)[0]=xJ(x)[0] op xJ(y)[0];} \
 else if(-KJ==xt && -KF==yt){xF(z)[0]=xJ(x)[0] op xF(y)[0];} \
 else if(-KF==xt && -KJ==yt){xF(z)[0]=xF(x)[0] op xJ(y)[0];} \
 else if(-KF==xt && -KF==yt){xF(z)[0]=xF(x)[0] op xF(y)[0];} \
 /*a op v*/ \
 else if(-KJ==xt && KJ==yt){DO(zn,xJ(z)[i]=xJ(x)[0] op xJ(y)[i]);} \
 else if(-KJ==xt && KF==yt){DO(zn,xF(z)[i]=xJ(x)[0] op xF(y)[i]);} \
 else if(-KF==xt && KJ==yt){DO(zn,xF(z)[i]=xF(x)[0] op xJ(y)[i]);} \
 else if(-KF==xt && KF==yt){DO(zn,xF(z)[i]=xF(x)[0] op xF(y)[i]);} \
 /*v op a*/ \
 else if(KJ==xt && -KJ==yt){DO(zn,xJ(z)[i]=xJ(x)[i] op xJ(y)[0]);} \
 else if(KJ==xt && -KF==yt){DO(zn,xF(z)[i]=xJ(x)[i] op xF(y)[0]);} \
 else if(KF==xt && -KJ==yt){DO(zn,xF(z)[i]=xF(x)[i] op xJ(y)[0]);} \
 else if(KF==xt && -KF==yt){DO(zn,xF(z)[i]=xF(x)[i] op xF(y)[0]);} \
 /*v op v*/ \
 else if(KJ==xt && KJ==yt){DO(zn,xJ(z)[i]=xJ(x)[i] op xJ(y)[i]);} \
 else if(KJ==xt && KF==yt){DO(zn,xF(z)[i]=xJ(x)[i] op xF(y)[i]);} \
 else if(KF==xt && KJ==yt){DO(zn,xF(z)[i]=xF(x)[i] op xJ(y)[i]);} \
 else if(KF==xt && KF==yt){DO(zn,xF(z)[i]=xF(x)[i] op xF(y)[i]);} \
 else   {R E_NYI;}                                            \
 r0(x);r0(y);

#endif