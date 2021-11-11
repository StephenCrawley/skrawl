#include "a.h"
#include "o.h"
#include "v.h"
#include "p.h"

// monad
K neg(K x){MONAD_INIT(KF,xt);DO(xn,if(KJ==ABS(xt)){xJ(z)[i]=-(xJ(x)[i]);}else{xF(z)[i]=-(xF(x)[i]);});r0(x);R z;}
K not(K x){K z=k(xt,xn);DO(xn,if(KJ==ABS(xt)){xJ(z)[i]=0==xJ(x)[i];}else{xJ(z)[i]=(0.0==xF(x)[i]);});r0(x);R z;;}
K typ(K x){K z=k(-KJ,1);xJ(z)[0]=xt;r0(x);R z;}
K til(K x){P(0<xt||KF==ABS(xt),E_TYP);K z=k(KJ,xJ(x)[0]);DO(z->n,xJ(z)[i]=i);r0(x);R z;}
K len(K x){K z=k(-KJ,0);xJ(z)[0]=xn;r0(x);R z;}
K enl(K x){K z;if(0<=xt){z=k(KK,1);xK(z)[0]=x;r1(x);}else if(-KJ==xt){z=k(KJ,1);xJ(z)[0]=xJ(x)[0];}else if(-KF==xt){z=k(KF,1);xF(z)[0]=xF(x)[0];}else{z=E_NYI;}r0(x);R z;}
K frs(K x){K z=KK==xt?r1(xK(x)[0]),xK(x)[0]:KJ==xt?kjx(xJ(x)[0]):KF==xt?kfx(xF(x)[0]):(-KJ==xt||-KF==xt)?r1(x),x:E_TYP;r0(x);R z;}
// dyad
K sum(K x,K y){DYAD_INIT(sum,KF);DYAD_EXEC(+);R z;}
K sub(K x,K y){DYAD_INIT(sub,KF);DYAD_EXEC(-);R z;}
K prd(K x,K y){DYAD_INIT(prd,KF);DYAD_EXEC(*);R z;}
K eq(K x,K y){DYAD_INIT(eq,KJ);DYAD_EXEC_ZA(==,xJ);R z;}
K lt(K x,K y){DYAD_INIT(lt,KJ);DYAD_EXEC_ZA(<, xJ);R z;}
K gt(K x,K y){DYAD_INIT(gt,KJ);DYAD_EXEC_ZA(>, xJ);R z;}
K cat(K x,K y){K z=ABS(xt)==ABS(yt)?k(ABS(xt),xn+yn):k(KK,xn+yn);J j=0;
 if     (KJ==ABS(xT(z))){DO(xn,xJ(z)[j]=xJ(x)[i];++j);DO(yn,xJ(z)[j]=xJ(y)[i];++j)}
 else if(KF==ABS(xT(z))){DO(xn,xF(z)[j]=xF(x)[i];++j);DO(yn,xF(z)[j]=xF(y)[i];++j)}
 else                   {DO(xn,xK(z)[j]=KK==ABS(xt)?(r1(xK(x)[i]),xK(x)[i]):KJ==ABS(xt)?kjx(xJ(x)[i]):kfx(xF(x)[i]);++j);
                         DO(yn,xK(z)[j]=KK==ABS(yt)?(r1(xK(y)[i]),xK(y)[i]):KJ==ABS(yt)?kjx(xJ(y)[i]):kfx(xF(y)[i]);++j)} //inefficient
 r0(x);r0(y);R z;}
K at_(K x,K y){P(KJ!=ABS(yt),(r0(x),r0(y),E_TYP));K z=k(KK,yn);DO(yn,xK(z)[i]=KK==xt?R1(xK(x)[xJ(y)[i]]):ABS(KJ)==xt?kjx(xJ(x)[xJ(y)[i]]):kfx(xF(x)[xJ(y)[i]]));r0(x);r0(y);R sqz(z);} //inefficient
// % is special. always returns float so one arg must be float
K dvd(K x,K y){if(KJ==ABS(xt)&&KJ==ABS(yt)){y=kfj(y);};DYAD_INIT(dvd,KF);DYAD_EXEC_ZA(/,xF);R z;}
// load+store
K get(T *t){P(1<t->l,E_NYI);C c=*t->s;P(!('a'<=c&&'z'>=c),E_NYI);K x=vt[c-'a'];C var[10];var[0]='\'';snprintf(var+1,9,"%.*s",t->l,t->s);P(NULL==x,kerr(var));r1(x);R x;}
K set(T *t,K x){P(1<t->l,E_NYI);C c=t->s[0];P(!('a'<=c&&'z'>=c),E_NYI);if(NULL!=vt[c-'a'])r0(vt[c-'a']);vt[c-'a']=x;r1(x);R x;}
// internal functions (for debugging)
ZK ref(K x){K z=k(-KJ,1);xJ(z)[0]=xr-1;r0(x);R z;} // return refcount (-1 to ignore the reference of the ref function itself)
K bng(K x,K y){P(-KJ!=xt,E_TYP);r1(y);K z= -1==xJ(x)[0]?typ(y):-2==xJ(x)[0]?ref(y):E_NYI;r0(x);r0(y);R z;}

//TODO :
//K ind(K x,K y){DYAD_INIT(ind,KJ);P(KJ-ABS(yt),E_TYP);if(0<yt){}else{K z=(KJ==ABS(xt)?);};R z;} // rewrite o.c to improve K creation