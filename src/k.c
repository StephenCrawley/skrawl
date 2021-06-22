/* main file
 * currently runs tests on latest interpreter modules.
 * below tests scanner in p.c (parser to be added) 
 */

#include "a.h"
#include "p.h"

//<! global source string
S a="123  +4.*xyz- 5.67;\"abc\",\"def\";  ";

//<!tests
I main(){Ti(a);T t;W(END-(t=nt(a)).t,pt(t));pt(t);R 0;}