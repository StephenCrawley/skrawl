#include "a.h"
#include "p.h"

// tokenizer utilities
Z T mt(TT t){T z;z.s=ts.s;z.l=(I)(ts.c-ts.s);z.t=t;R z;}      //<! make token
Z C nc(){R *ts.c++;}                                          //<! consume next char
Z V ws(){W(' '==*ts.c||'\n'==*ts.c,++ts.c);}                  //<! skip whitespace
Z I ca(C c){R('a'<=c&&'z'>=c)||('A'<=c&&'Z'>=c);}             //<! char is alpha?
Z I cn(C c){R'0'<=c&&'9'>=c;}                                 //<! char is num?
Z T id(){W(ca(*ts.c),++ts.c);R mt(ID);}                       //<! id token
Z T str(){W('"'-*ts.c,++ts.c);++ts.c;R mt(STR);}              //<! str token
Z T num(){W(cn(*ts.c)||'.'==*ts.c,++ts.c);R mt(NUM);}         //<! num token
V Ti(S a){ts.s=a;ts.c=a;}                                     //<! init new string to tokenize (may replace with non-global solution)

// return next token
T nt(V){ws();ts.s=ts.c;C c=nc();if(cn(c))R num();if(ca(c))R id();if('"'==c)R str();
 switch(c){
 CS('*', R mt(ST))  CS('+', R mt(PL))  CS('%', R mt(DV))  CS('!', R mt(BA))
 CS('-', R mt(HI))  CS('@', R mt(AT))  CS('~', R mt(TL))  CS('#', R mt(HS))
 CS(',', R mt(CM))  CS('.', R mt(DT))  CS('\'',R mt(AP))  CS('/', R mt(FS))
 CS('\\',R mt(BS))  CS('(', R mt(LP))  CS(')', R mt(RP))  CS('{', R mt(LB))
 CS('}', R mt(RB))  CS('[', R mt(LS))  CS(']', R mt(RS))  CS('<', R mt(LA))
 CS('>', R mt(RA))  CS(';', R mt(SC))  CS(':', R mt(CL))  CS('?', R mt(QM))
 CS('|', R mt(PI))  CS('\0',R mt(END)) default:R mt(NR);};}

// debug, print token meta
V pt(T t){O("type:%2d len:%d lexeme:'%.*s'\n", t.t,t.l,t.l,t.s);}