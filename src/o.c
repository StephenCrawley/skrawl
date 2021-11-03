#include "a.h"
// create K object
ZK ma(J s,J n){K k=malloc(OZ(struct k,d)+s*n);k->n=n;k->r=1;R k;}                              //<! allocate object of atom size s
K kjn(J n){K k=ma(SZ(J),n);xT(k)=KJ;xR(k)=0;R k;} K kj(){K k=kjn(1);xT(k)=-KJ;R k;}            //<! return J object of length n. return J atom
K kfn(J n){K k=ma(SZ(F),n);xT(k)=KF;xR(k)=0;R k;} K kf(){K k=kfn(1);xT(k)=-KF;R k;}            //<! return F object of length n. return F atom
K kerr(S e){J n=strlen(e);K x=ma(SZ(C),n+1);DO(n,xC(x)[i]=e[i]);xC(x)[n+1]='\0';xt=-128;R x;}  //<! error object with error msg e
K k(I t,J n){R KJ==t?kjn(n):KF==t?kfn(n):E_NYI;} 
// str to K
K kjc(S s){K k=kj();*xJ(k)=strtoll(s,NULL,10);R k;} 
K kjcn(S s,I n){K k=kjn(n);S e;DO(n,xJ(k)[i]=strtoll(s,&e,10);s=e);R k;}
K kfc(S s){K k=kf();*xF(k)=strtod(s,NULL);R k;}
K kfcn(S s,I n){K k=kfn(n);S e;DO(n,xF(k)[i]=strtod(s,&e);s=e);R k;}
// refcount
V r0(K x){if(1>xr)free(x);}
V r1(K x){xR(x)+=1;}

K kfj(K x){K z=kfn(xn);DO(xn,xF(z)[i]=(F)xJ(x)[i]);r0(x);R z;} //<! cast. float from long

// print
V pk(K x){if(KJ==ABS(xt))DO(xn,O("%lld ",xJ(x)[i]))else if(KF==ABS(xt))DO(x->n,O("%f ",xF(x)[i]))else if(err(x))O("%s",xC(x));O("\n");}