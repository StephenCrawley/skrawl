#include "a.h"
#include "p.h"

Z J xp(J x,J n){J r=1;DO(n,r*=x);R r;} //<! exponentiation
Z J sl(S a){J i=0;W('\0'-a[i],++i);R i;} //<! strlen
Z J ja(S a){J n=sl(a),r=0;DO2(n,r+=(a[i]-'0')*xp(10,j));R r;} //<! J from string

//<! ca:char is alpha?                            cn:char is number?
Z I ca(C c){R('a'<=c&&'z'>=c)||('A'<=c&&'Z'>=c);} Z I cn(C c){R'0'<=c&&'9'>=c;}
//<! init new string to tokenize  next char            skip whitespace
V Ti(S a){ts.s=a;ts.c=a;}         Z C nc(){R *ts.c++;} Z V ws(){W(' '==*ts.c||'\n'==*ts.c,++ts.c);}
//<! make token
Z T mt(TT t){T z;z.s=ts.s;z.l=(I)(ts.c-ts.s);z.t=t;R z;}
//<! num token                                         id token
Z T num(){W(cn(*ts.c)||'.'==*ts.c,++ts.c);R mt(NUM);}  Z T id(){W(ca(*ts.c),++ts.c);R mt(ID);}
//<! str token
Z T str(){W('"'-*ts.c,++ts.c);++ts.c;R mt(STR);}
//<! return next token
T nt(){ws();ts.s=ts.c;C c=nc();if(cn(c))R num();if(ca(c))R id();if('"'==c)R str();
 switch(c){
 CS('*', R mt(ST))  CS('+', R mt(PL))  CS('%', R mt(DV))  CS('!', R mt(BA))  
 CS('-', R mt(HI))  CS('@', R mt(AT))  CS('~', R mt(TL))  CS('#', R mt(HS))
 CS(',', R mt(CM))  CS('.', R mt(DT))  CS('\'',R mt(AP))  CS('/', R mt(FS))
 CS('\\',R mt(BS))  CS('(', R mt(LP))  CS(')', R mt(RP))  CS('{', R mt(LB))  
 CS('}', R mt(RB))  CS('[', R mt(LS))  CS(']', R mt(RS))  CS('<', R mt(LA))  
 CS('>', R mt(RA))  CS(';', R mt(SC))  CS(':', R mt(CL))  CS('?', R mt(QM))
 CS('|', R mt(PI))  CS('\0',R mt(END)) default:R mt(NR);};
 }

//<! debug, print token meta
V pt(T t){O("token type:%2d| len:%d| lexeme:'%.*s'\n", t.t,t.l,t.l,t.s);}
