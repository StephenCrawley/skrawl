#include <stdio.h>
typedef char C,*S; typedef int I; typedef long long J; typedef double F; typedef void V;
typedef struct k{I t,r;J n,v[];}*K; //<! type,refcount,length,data
#define DO(n,x)  {J i=0,_n=(n);for(;i<_n;++i){x;}} //<! for
#define DO2(n,x) {J i=0,j=(n)-1;for(;j>=0;++i,--j){x;}} //<! forward+backward for
#define W(p,x)   while(p){x;} //<! while
#define O        printf
#define R        return

J xp(J x,J n){J r=1;DO(n,r*=x);R r;} //<! exponentiation
J sl(S a){J i=0;W('\0'-a[i],++i);R i;} //<! strlen
J ja(S a){J n=sl(a),r=0;DO2(n,r+=(a[i]-'0')*xp(10,j));R r;} //<! J from string

//<! ca:char is alpha?                          cn:char is number?
I ca(C c){R(c>='a'&&c<='z')||(c>='A'&&c<='Z');} I cn(C c){R c>='0'&&c<='9';}
//<! verbs   cv:char is verb? returns verb index+1 if true
S vt="+-*%"; I cv(C c){I n=sl(vt);DO(n,if(vt[i]==c)R i+1);R 0;}

//<!tests
I main(){
 S a="aBc-5Z0*&";J n=sl(a);
 DO(n,O("%c is alpha? %d. num? %d. verb? %d\n",a[i],ca(a[i]),cn(a[i]),cv(a[i])));
 R 0;}
