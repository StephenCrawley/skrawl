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
K whr(K x){P(KJ!=xt,(r0(x),E_TYP));R1(x);R fld2(cat,take(kjx(0),kjx(0)),each(take,x,til(len(x))));} //{,/x#'!#x}. TODO: should be: {,/(0|x)#'!#x}
K rev(K x){R R1(R1(x)),at(x, sub(len(x),sum(kjx(1),til(len(x)))));} //{x@(#x)-1+!#x}
// dyad
#define SUM(x,y)  ((x) + (y))
#define SUB(x,y)  ((x) - (y))
#define PRD(x,y)  ((x) * (y))
#define DIV(x,y)  ((x) / (y))
#define  EQ(x,y)  ((x) ==(y))
#define  LT(x,y)  ((x) < (y))
#define  GT(x,y)  ((x) > (y))
K sum(K x,K y){DYAD_EXEC(sum,KF,SUM);R z;}
K sub(K x,K y){DYAD_EXEC(sub,KF,SUB);R z;}
K prd(K x,K y){DYAD_EXEC(prd,KF,PRD);R z;}
K  eq(K x,K y){DYAD_EXEC( eq,KJ, EQ);R z;}
K  lt(K x,K y){DYAD_EXEC( lt,KJ, LT);R z;}
K  gt(K x,K y){DYAD_EXEC( gt,KJ, GT);R z;}
K  or(K x,K y){DYAD_EXEC( or,KF,MAX);R z;}
K and(K x,K y){DYAD_EXEC(and,KF,MIN);R z;}
K cat(K x,K y){K z=ABS(xt)==ABS(yt)?k(ABS(xt),xn+yn):k(KK,xn+yn);J j=0;
 if     (KJ==ABS(xT(z))){DO(xn,xJ(z)[j]=xJ(x)[i];++j);DO(yn,xJ(z)[j]=xJ(y)[i];++j)}
 else if(KF==ABS(xT(z))){DO(xn,xF(z)[j]=xF(x)[i];++j);DO(yn,xF(z)[j]=xF(y)[i];++j)}
 else                   {DO(xn,xK(z)[j]=KK==ABS(xt)?(r1(xK(x)[i]),xK(x)[i]):KJ==ABS(xt)?kjx(xJ(x)[i]):kfx(xF(x)[i]);++j);
                         DO(yn,xK(z)[j]=KK==ABS(yt)?(r1(xK(y)[i]),xK(y)[i]):KJ==ABS(yt)?kjx(xJ(y)[i]):kfx(xF(y)[i]);++j)} //inefficient
 r0(x);r0(y);R z;}
ZK at_(K x,K y){P(KJ!=ABS(yt),(r0(x),r0(y),E_TYP));K z=k(KK,yn);DO(yn,xK(z)[i]=KK==xt?R1(xK(x)[xJ(y)[i]]):KJ==ABS(xt)?kjx(xJ(x)[xJ(y)[i]]):kfx(xF(x)[xJ(y)[i]]));r0(x);r0(y);R sqz(z,yt);} //inefficient
K at(K x,K y){K z;if(KJ==ABS(yt)){z=at_(R1(x),R1(y));}else{z=k(KK,yn);DO(yn,xK(z)[i]=at(R1(x),R1(xK(y)[i])));}r0(x),r0(y);R z;}
K take(K x,K y){P(KJ!=ABS(xt),(r0(x),r0(y),E_TYP));P(KK==yt,(r0(x),r0(y),E_NYI));J n=xJ(x)[0];K z=k(ABS(yt),n);
 if(KJ==ABS(yt)){DO(n,xJ(z)[i]=xJ(y)[i%yn])}else if(KF==ABS(yt)){DO(n,xF(z)[i]=xF(y)[i%yn])}else{DO(n,xK(z)[i]=xK(y)[i%yn])};r0(x);r0(y);R sqz(z,1);}
// % is special. always returns float so one arg must be float
K dvd(K x,K y){if(KJ==ABS(xt)&&KJ==ABS(yt)){y=kfj(y);};DYAD_INIT(dvd,KF);DYAD_EXEC_ZA(DIV,xF);R z;}
K mod(K x,K y){P(KJ!=ABS(xt)||-KJ!=yt,(r0(x),r0(y),E_NYI));DYAD_INIT(mod,KJ);DO(xn,xJ(z)[i]=xJ(x)[i]%*xJ(y));R z;}
// load+store
K get(T *t){P(1<t->l,E_NYI);C c=*t->s;P(!('a'<=c&&'z'>=c),E_NYI);K x=vt[c-'a'];C var[10];var[0]='\'';snprintf(var+1,9,"%.*s",t->l,t->s);P(NULL==x,kerr(var));r1(x);R x;}
K set(T *t,K x){P(1<t->l,E_NYI);C c=t->s[0];P(!('a'<=c&&'z'>=c),E_NYI);if(NULL!=vt[c-'a'])r0(vt[c-'a']);vt[c-'a']=x;r1(x);R x;}
// internal functions (for debugging)
ZK ref(K x){K z=k(-KJ,1);xJ(z)[0]=xr-1;r0(x);R z;} // return refcount (-1 to ignore the reference of the ref function itself)
K bng(K x,K y){P(-KJ!=xt,E_TYP);r1(y);K z= -1==xJ(x)[0]?typ(y):-2==xJ(x)[0]?ref(y):E_NYI;r0(x);r0(y);R z;}
// adverb
K each(K (*f)(K,K),K x,K y){K z=k(KK,xn);DO(xn,xK(z)[i]=(*f)(KK==xt?R1(xK(x)[i]):kjx(xJ(x)[i]),KK==yt?R1(xK(y)[i]):kjx(xJ(y)[i])));r0(x),r0(y);R z;}
K fld2(K (*f)(K,K),K x,K y){P(0>yt,E_RNK);K z=R1(x);DO(yn,z=(*f)(z,KK==yt?R1(xK(y)[i]):kjx(xJ(y)[i])));r0(x),r0(y);R z;}