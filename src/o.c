#include "a.h"
// create K object
//TODO : remove magic numbers (type codes)
//TODO : rename kJ->kjn (too close to the type macros)
ZK ma(J s,J n){K k=malloc(OZ(struct k,d)+s*n);k->n=n;k->r=1;k->t=(1-n)?1:-1;R k;}              //<! allocate object of atom size s, length n
K kJ(J n){K k=ma(SZ(J),n);xT(k)=n>1?KJ:-KJ;xR(k)=0;R k;} K kj(){K k=kJ(1);R k;}                //<! return J object of length n. return J atom
K kF(J n){K k=ma(SZ(F),n);xT(k)=n>1?KF:-KF;xR(k)=0;R k;} K kf(){K k=kF(1);R k;}                //<! return F object of length n. return F atom
K kerr(S e){J n=strlen(e);K x=ma(SZ(C),n+1);DO(n,xC(x)[i]=e[i]);xC(x)[n+1]='\0';xt=-128;R x;}  //<! error object with error msg e
K k(I t,J n){R KJ==t?kJ(n):KF==t?kF(n):E_NYI;} 
// str to K
K kjc(S s){K k=kj();*xJ(k)=strtoll(s,NULL,10);R k;} K kfc(S s){K k=kf();*xF(k)=strtod(s,NULL);R k;}
// refcount
V r0(K x){if(1>xR(x))free(x);}
V r1(K x){xR(x)+=1;}

// print
V pk(K x){if(KJ==ABS(xt))DO(x->n,O("%lld ",xJ(x)[i]))else if(KF==ABS(xt))DO(x->n,O("%f ",xF(x)[i]))else if(err(x))O("%s",xC(x));r0(x);O("\n");}