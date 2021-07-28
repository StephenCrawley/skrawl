#include "a.h"
#include "p.h"
// tokenizer utilities
Z T mt(TT t){T z;z.s=ts.s;z.l=(I)(ts.c-ts.s);z.t=t;R z;}  //<! make token
Z C nc(){R *ts.c++;}                                      //<! consume next char
Z V ws(){W(' '==*ts.c||'\n'==*ts.c,++ts.c);}              //<! skip whitespace
Z I ca(C c){R('a'<=c&&'z'>=c)||('A'<=c&&'Z'>=c);}         //<! char is alpha?
Z I cn(C c){R'0'<=c&&'9'>=c;}                             //<! char is num?
Z T id(){W(ca(*ts.c),++ts.c);R mt(ID);}                   //<! id token
Z T str(){W('"'-*ts.c,++ts.c);++ts.c;R mt(STR);}          //<! str token
Z T num(){W(cn(*ts.c)||'.'==*ts.c,++ts.c);R mt(NUM);}     //<! num token
V Ti(S a){ts.s=a;ts.c=a;}                                 //<! init new string to tokenize (may replace with non-global solution)
T nt(){ws();ts.s=ts.c;C c=nc();                           //<! return next token
 if(cn(c))R num();if(ca(c))R id();if('"'==c)R str();switch(c){
 CS('*', R mt(ST))  CS('+', R mt(PL))  CS('%', R mt(DV))  CS('!', R mt(BA))  CS('-', R mt(HI))  CS('@', R mt(AT))  
 CS('~', R mt(TL))  CS('#', R mt(HS))  CS(',', R mt(CM))  CS('.', R mt(DT))  CS('\'',R mt(AP))  CS('/', R mt(FS))  
 CS('\\',R mt(BS))  CS('(', R mt(LP))  CS(')', R mt(RP))  CS('{', R mt(LB))  CS('}', R mt(RB))  CS('[', R mt(LS))  
 CS(']', R mt(RS))  CS('<', R mt(LA))  CS('>', R mt(RA))  CS(';', R mt(SC))  CS(':', R mt(CL))  CS('?', R mt(QM))  
 CS('|', R mt(PI))  CS('\0',R mt(END)) default:R mt(NR);}}
V rt(){T t;I i=0;W(END-(t=nt()).t,ts.b[i++]=t);ts.b[i]=t;}; //<! read all tokens from source into buffer ts.b
V pt(T t){O("typ:%2d len:%d lexeme:'%.*s'\n", t.t,t.l,t.l,t.s);} //<! debug,print token
V pT(){O("tokens:\n");T t;I i=0;W(END-(t=ts.b[i++]).t,pt(t));pt(t);} //<! print all tokens using pt
// parser utilities
I fact(T *tk){TT t=tk->t;R NUM==t?strtod(tk->s,NULL):LP==t?pr(tk+1):ID==t?(vt[*tk->s-'a']):-128;} //<! parse factor (NUM/parens)
I pr(T *tk){  //<! parse
 if(END==tk->t)R 0;if(END==tk[1].t||RP==tk[1].t)R fact(tk);I y,i=0; //<! if next token is END or ), evaluate+return current token
 if(LP==tk->t){I n=1;W(n,++i;TT t=tk[i].t;n+=(LP==t)?1:(RP==t)?-1:0);if(END==tk[i+1].t)R fact(tk);else y=pr(tk+i+2);}else{y=pr(tk+2);}; //<! skip ( expr )
 I x=fact(tk);if(DBGP)O("x=%d, y=%d\n",x,y);switch(tk[i+1].t){CS(PL,R x+y) CS(ST,R x*y) CS(HI,R x-y) CS(CL,R vt[*tk->s-'a']=y)}; //<! case +*-:
 R -128;}