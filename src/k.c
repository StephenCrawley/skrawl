/* 
 * basic repl for atomic+vector(1D) arithmetic(+-*%) and monadic ops(~-@). parens and assignment(names a-z) supported. easy to segfault.
 */

#include "a.h"
#include "o.h"
#include "p.h"

// global source string
C A[128];

// repl
I main(){S c=A;O(" ");W(fgets(c,128,stdin),Ti(c);rt();if(DBGT)pT();T *p=ts.b;K x=pr(p);pk(x);O(" "););DO(26,if(!(NULL==vt[i]))free(vt[i]));O("\n");}