/* main file
 * currently runs tests on latest interpreter modules
 * below tests scanner and parser in p.c
 * basic repl for atomic arithmetic(+-*)! parens and assignment(names a-z) supported
 */

#include "a.h"
#include "p.h"

// global source string
//S a="123  +4.*xyz- 5.67;\"abc\",\"def\"; +/1 2 3 ";
S a="5-(4-1)*1+2";C A[128];

// tests
//I main(){O("source: %s\ntokens:\n",a);Ti(a);rt();T t;I i=0;W(END-(t=ts.b[i++]).t,pt(t));pt(t);R 0;}
//I main(){O("source: %s\nsteps:\n",a);Ti(a);rt();if(DBGT)pT();T *p=ts.b;O("answer: %d\n",pr(p));}
I main(){S c=A;O(" ");W(fgets(c,128,stdin),Ti(c);rt();if(DBGT)pT();O("steps:\n");T *p=ts.b;O("answer: %d\n",pr(p));O(" "););}