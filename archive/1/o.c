#include "a.h"
#include "o.h"
#include "p.h"
//TODO : refactor this file
// create K object
G KS[]={8,8,8,1}; // type size (K,J,F,C). used by mc
ZK ma(J s,J n){K k=malloc(OZ(struct k,d)+s*n);k->n=n;k->r=1;R k;}                             // allocate object of atom size s
K mc(K x,K y){DO(xn*KS[ABS(xt)],xD(y)[i]=xD(x)[i]);R y;}                                      // copy x data into y
K kn(J n){K k=ma(SZ(K),n);xT(k)=KK;R k;}                                                      // return generic object of length n
K knl(){K k=ma(1,0);xT(k)=KN;R k;}                                                            // return null object of length n 
K kqt(){K k=ma(1,0);xT(k)=KQ;R k;}
K kjn(J n){K k=ma(SZ(J),n);xT(k)=KJ;R k;} K kj(){K k=kjn(1);xT(k)=-KJ;R k;}                   // return J object of length n. return J atom
K kfn(J n){K k=ma(SZ(F),n);xT(k)=KF;R k;} K kf(){K k=kfn(1);xT(k)=-KF;R k;}                   // return F object of length n. return F atom
K kcn(J n){K k=ma(SZ(C),n);xT(k)=KC;R k;} K kc(){K k=kcn(1);xT(k)=-KC;R k;}                   // return C object of length n. return C atom
K kd(){K k=ma(SZ(K),2);xT(k)=KD;R k;}                                                         // return D object
K kjx(J x){K k=kj();*xJ(k)=x;R k;} K kfx(F x){K k=kf();*xF(k)=x;R k;} K kcx(C x){K k=kc();*xC(k)=x;R k;} // create atom with value x
K kerr(S e){J n=strlen(e);K x=ma(SZ(C),n+1);DO(n,xC(x)[i]=e[i]);xC(x)[n]='\0';xt=-128;R x;}   // error object with error msg e
K k(I t,J n){R KK==t?kn(n):KJ==t?kjn(n):-KJ==t?kj():KF==t?kfn(n):-KF==t?kf():KC==t?kcn(n):-KC==t?kc():KN==t?knl():KQ==t?kqt():E_NYI;}
K kxp(K x){K z=k(KK,xn);DO(xn,xK(z)[i]=KJ==ABS(xt)?kjx(xJ(x)[i]):KF==ABS(xt)?kfx(xF(x)[i]):kcx(xC(x)[i]));r0(x);R z;}   // expand.  eg 1 2 3->(1;2;3) N.B. assumes types KJ/KF
K sqz(K x){P(KK!=xt,x)P(0==xn,x)C t=xT(xK(x)[xn-1]);K z;P(0<=t&&1==xn,(z=xK(x)[0],r1(z),r0(x),z));P(0<=t,x);G b=1;DO(xn-1,b=(b&&t==xT(xK(x)[i]))); // squeeze. eg (1;2;3)->1 2 3 N.B. assumes child items are atoms
 P(!b,x);if(-KJ==t){z=k(KJ,xn);DO(xn,xJ(z)[i]=*xJ(xK(x)[i]));}else if(-KF==t){z=k(KF,xn);DO(xn,xF(z)[i]=*xF(xK(x)[i]));}
 else{z=k(KC,xn);DO(xn,xC(z)[i]=*xC(xK(x)[i]));}
 r0(x);R z;}
// str to K
K kjc(S s){K k=kj();*xJ(k)=strtoll(s,NULL,10);R k;} 
K kjcn(S s,I n){K k=kjn(n);S e;DO(n,xJ(k)[i]=strtoll(s,&e,10);s=e);R k;}
K kfc(S s){K k=kf();*xF(k)=strtod(s,NULL);R k;}
K kfcn(S s,I n){K k=kfn(n);S e;DO(n,xF(k)[i]=strtod(s,&e);s=e);R k;}
K kcc(S s,I n){K z=1==n?kc():kcn(n);DO(n,xC(z)[i]=s[i])R z;}
// refcount
V r0(K x){if(KK==xt||KD==xt){DO(xn,r0(xK(x)[i]))}xr--;if(1>xr)free(x);}
K r1(K x){if(KK==xt||KD==xt){DO(xn,r1(xK(x)[i]))}xr++;R x;}
// cast
K kfj(K x){K z=k(SGN(xt)*KF,xn);DO(xn,xF(z)[i]=(F)xJ(x)[i]);r0(x);R z;} // float from long
// print
ZV pc(K x){O("\"");DO(xn,putchar(xC(x)[i]));O("\"");}
ZV pj(K x){if(xn){DO(xn,O("%lld",xJ(x)[i]);if(i!=xn-1)O(" "))}else{O("\"j\"$()");};}
ZV pf(K x){if(xn){C s[9];DO(xn,I r=snprintf(s,9,"%f.6",xF(x)[i]);if(0>r){pk(kerr("'print"));R;};G p=8;W('0'==s[p-1],--p);O("%.*s",p,s);if(i!=xn-1)O(" "));}else{O("\"f\"$()");}} //print F without trailing 0s
ZV pk_(K x){if(0<xt&&1==xn)O(",");if(KK==xt){if(1==xn){O(",");}else{O("(");}DO(xn,pk_(xK(x)[i]);if(i!=xn-1)O(";"));if(1!=xn)O(")");}else if(KJ==ABS(xt)){pj(x);}else if(KF==ABS(xt)){pf(x);}else if(KC==ABS(xt)){pc(x);}
 else if(err(x)){O("%s",xC(x));}else{pk(E_TYP);};}
V pk(K x){if(KN==xt){r0(x);R;};if(0<xt&&1==xn)O(",");
 if(DBGP){O("typ:%d  cnt:%lld  ref:%d  dat:\n",xt,xn,xr);}; // print object meta+data
 if(KK==xt){if(1==xn){O(",");}DO(xn,pk_(xK(x)[i]);if(i!=xn-1)O("\n"))if(0==xn)O("()");}else if(KJ==ABS(xt)){pj(x);}else if(KF==ABS(xt)){pf(x);}else if(KC==ABS(xt)){pc(x);}
 else if(KD==xt){pk_(xK(x)[0]),O("!"),pk_(xK(x)[1]);}else if(err(x)){O("%s",xC(x));}else{pk(E_TYP);};O("\n");r0(x);}