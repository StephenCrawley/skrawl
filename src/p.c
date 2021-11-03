// token+parse
#include "a.h"
#include "p.h"
#include "o.h"
#include "v.h"
K vt[26]={NULL};
TT ot[]={CL,EQ,LA,RA,PI,QM,PL,HY,ST,DV,BA,AT,TL,HS,CM,DT};               //<! operator table
ZI io(TT t){DO(SZ(ot)/SZ(ot[0]),if(ot[i]==t)R i+1);R 0;}                 //<! is operator / index(+1) of operator
// tokenizer utilities
ZT mt(TT t){T z;z.s=ts.s;z.l=(I)(ts.c-ts.s);z.t=t;R z;}                  //<! make token
ZC nc(){R *ts.c++;}                                                      //<! consume next char
ZV ws(){W(' '==*ts.c||'\n'==*ts.c,++ts.c);}                              //<! skip whitespace
ZI ca(C c){R('a'<=c&&'z'>=c)||('A'<=c&&'Z'>=c);}                         //<! char is alpha?
ZI cn(C c){R'0'<=c&&'9'>=c;}                                             //<! char is num?
ZT id(){W(ca(*ts.c)||cn(*ts.c),++ts.c);R mt(ID);}                        //<! id token
ZT str(){W('"'-*ts.c,++ts.c);++ts.c;R mt(STR);}                          //<! str token
ZT num(){G f=0,g;W(cn(*ts.c)||(g='.'==*ts.c),++ts.c;f=f||g);R f?mt(FLT):mt(INT);} //<! num token
ZT hy(){TT t=ts.bp[-1].t;R((INT==t||FLT==t||RP==t)&&(' '-ts.s[-1]||' '==*ts.c))?mt(HY):cn(*ts.c)?num():mt(HY);} //<! handle minus/unary neg/negative num
ZT dt(){R cn(ts.c[1])?num():mt(DT);}                                     //<! num starting with . or just . ? TODO: fix this
ZT com(){W('\n'-*ts.c++,);R nt();}                                       //<! skip comment and return next token
V Ti(S a){ts.s=a;ts.c=a;ts.bp=ts.b;}                                     //<! init new string to tokenize (may replace with non-global solution)
T nt(){ws();ts.s=ts.c;C c=nc();                                          //<! return next token
 if(cn(c))R num();if(ca(c))R id();if('"'==c)R str();switch(c){
 CS('/',if(ts.b==ts.bp)R com();if(' '==ts.s[-1])R com();R mt(FS))        //<! skip comments and return next token
 CS('*',R mt(ST)) CS('+',R mt(PL)) CS('%',R mt(DV)) CS('!',R mt(BA)) CS('-',R hy())   CS('.', R dt())   CS('\'',R mt(AP)) 
 CS('@',R mt(AT)) CS('~',R mt(TL)) CS('#',R mt(HS)) CS(',',R mt(CM)) CS('(',R mt(LP)) CS(')', R mt(RP)) CS('\\',R mt(BS)) 
 CS('{',R mt(LB)) CS('}',R mt(RB)) CS('[',R mt(LS)) CS(']',R mt(RS)) CS('<',R mt(LA)) CS('>', R mt(RA)) CS('\0',R mt(END)) 
 CS(';',R mt(SC)) CS(':',R mt(CL)) CS('?',R mt(QM)) CS('|',R mt(PI)) CS('=',R mt(EQ)) default:R mt(NR);}}
V rt(){T t;W(END-(t=nt()).t,*ts.bp++=t);*ts.bp=t;};                      //<! read all tokens from source into buffer ts.b
V pt(T t){O("typ:%2d len:%d lexeme:'%.*s'\n",t.t,t.l,t.l,t.s);}          //<! debug,print token
V pT(){O("tokens:\n");T t;I i=0;W(END-(t=ts.b[i++]).t,pt(t));pt(t);}     //<! print all tokens using pt
// parser utilities
ZK prsn(T *tk){I n=1,f=(FLT==tk->t),g=0;W(INT==tk[n].t||(g=(FLT==tk[n].t)),++n;f=MAX(f,g));R 1==n?(f?kfc(tk->s):kjc(tk->s)):(f?kfcn(tk->s,n):kjcn(tk->s,n));} //<! parse num
ZK fact(T *tk){TT t=tk->t;R (INT==t||FLT==t)?prsn(tk):LP==t?pr(tk+1):ID==t?get(tk):E_NYI;} //<! parse factor (NUM/FLT/parens/var)
K pr(T *tk){K x,y;TT t=tk->t;//<! parse+exec
 if(END==t)R kerr("'end");if(END==tk[1].t||RP==tk[1].t)R fact(tk); //<! if next token is END or )->eval+return current token
 if(io(t)){if(AT==t||HY==t||TL==t||BA){K x=pr(tk+1);R err(x)?x:AT==t?typ(x):HY==t?neg(x):BA==t?til(x):not(x);}else{R E_NYI;}} //<! monad operators
 if(CL==tk[1].t){y=pr(tk+2);if(err(y))R y;else{R set(tk,y);}} //<! assign x:y
 I i=0;if(LP==t){G n=1;W(n,++i;TT t=tk[i].t;n+=LP==t?1:RP==t?-1:0);if(END==tk[i+1].t||RP==tk[i+1].t)R fact(tk);else y=pr(tk+i+2);} //<! handle ( )
 //TODO : handle float vectors
 else if(INT==t||FLT==t){W(INT==tk[i+1].t||FLT==tk[i+1].t,++i);if(END==tk[i+1].t||RP==tk[i+1].t){R fact(tk);}else{y=pr(tk+i+2);}} //<! parse num literal
 else{y=pr(tk+2);}if(err(y))R y;
 x=fact(tk);if(err(x))R x;if(DBGP){O("x: ");pk(x);O("xt: %d\n",xt);O("y: ");pk(y);O("yt: %d\n",yt);O("op: %.*s\n",tk[i+1].l,tk[i+1].s);}; //<! get x (left operand). debug prints
 switch(tk[i+1].t){CS(PL,R sum(x,y))CS(ST,R prd(x,y))CS(DV,R dvd(x,y))CS(HY,R sub(x,y))CS(EQ,R eq(x,y))CS(RA,R gt(x,y))CS(CM,R cat(x,y))}//<! case +*-:~,
 R E_NYI;}