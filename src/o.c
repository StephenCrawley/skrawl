#include "a.h"
#include "o.h"
#include "p.h"
//TODO : refactor this file
// create K object
ZK ma(J s,J n){K k=malloc(OZ(struct k,d)+s*n);k->n=n;k->r=1;R k;}                             // allocate object of atom size s
K kn(J n){K k=ma(SZ(K),n);xT(k)=KK;R k;}                                                      // return generic object of length n
K knl(){K k=ma(1,0);xT(k)=KN;R k;}                                                            // return null object of length n 
K kqt(){K k=ma(1,0);xT(k)=KQ;R k;}
K kjn(J n){K k=ma(SZ(J),n);xT(k)=KJ;R k;} K kj(){K k=kjn(1);xT(k)=-KJ;R k;}                   // return J object of length n. return J atom
K kfn(J n){K k=ma(SZ(F),n);xT(k)=KF;R k;} K kf(){K k=kfn(1);xT(k)=-KF;R k;}                   // return F object of length n. return F atom
K kjx(J x){K k=kj();xJ(k)[0]=x;R k;} K kfx(F x){K k=kf();xF(k)[0]=x;R k;}                     // create atom with value x
K kerr(S e){J n=strlen(e);K x=ma(SZ(C),n+1);DO(n,xC(x)[i]=e[i]);xC(x)[n+1]='\0';xt=-128;R x;} // error object with error msg e
K k(I t,J n){R KK==t?kn(n):KJ==t?kjn(n):-KJ==t?kj():KF==t?kfn(n):-KF==t?kf():KN==t?knl():KQ==t?kqt():E_NYI;}
K kxp(K x){K z=k(KK,xn);DO(xn,xK(z)[i]=KJ==ABS(xt)?kjx(xJ(x)[i]):kfx(xF(x)[i]));r0(x);R z;}   //expand. eg 1 2 3->(1;2;3) N.B. assumes types KJ/KF
// str to K
K kjc(S s){K k=kj();*xJ(k)=strtoll(s,NULL,10);R k;} 
K kjcn(S s,I n){K k=kjn(n);S e;DO(n,xJ(k)[i]=strtoll(s,&e,10);s=e);R k;}
K kfc(S s){K k=kf();*xF(k)=strtod(s,NULL);R k;}
K kfcn(S s,I n){K k=kfn(n);S e;DO(n,xF(k)[i]=strtod(s,&e);s=e);R k;}
// refcount
V r0(K x){if(KK==xt){DO(xn,r0(xK(x)[i]))}xr--;if(1>xr)free(x);}
V r1(K x){if(KK==xt){DO(xn,r1(xK(x)[i]))}xr++;}
K R1(K x){r1(x);R x;}

K kfj(K x){K z=kfn(xn);DO(xn,xF(z)[i]=(F)xJ(x)[i]);r0(x);R z;} // cast. float from long

// print
ZV pj(K x){DO(xn,O("%lld ",xJ(x)[i]));}
ZV pf(K x){C s[9];DO(xn,I r=snprintf(s,9,"%f.6",xF(x)[i]);if(0>r){pk(kerr("'print"));R;};G i=8;W('0'==s[i-1],--i);O("%.*s",i,s);O(" "))} //print F without trailing 0s
ZV pk_(K x){if(0<xt&&1==xn)O(",");if(KK==xt){O("(");DO(xn,pk_(xK(x)[i]);if(i!=xn-1)O(";"));O(")");}else if(KJ==ABS(xt)){pj(x);}else if(KF==ABS(xt)){pf(x);}
 else if(err(x)){O("%s",xC(x));}else{pk(E_TYP);};}
V pk(K x){if(KN==xt){r0(x);R;};if(0<xt&&1==xn)O(",");
 if(DBGP){O("typ: %d. cnt: %lld. ref:%d\ndat: ",xt,xn,xr);}; // print object meta+data
 if(KK==xt){DO(xn,pk_(xK(x)[i]);if(i!=xn-1)O("\n"))}else if(KJ==ABS(xt)){pj(x);}else if(KF==ABS(xt)){pf(x);}else if(err(x)){O("%s",xC(x));}else{pk(E_TYP);};O("\n");r0(x);}