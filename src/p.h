#ifndef PARSE
#define PARSE

#include "a.h"

//            (  )  {  }  [  ]  ;  :  <  >  |  ?  +  -  *  %  !  "  @  ~  #  ,  .  '  /     1   "a"    \0  not recognised
typedef enum {LP,RP,LB,RB,LS,RS,SC,CL,LA,RA,PI,QM,PL,HI,ST,DV,BA,QT,AT,TL,HS,CM,DT,AP,FS,BS,NUM,STR,ID,END,NR}TT; 
// start,length,token type
typedef struct {S s;I l;TT t;}T;
// scanner. start,current
typedef struct {S s,c;}TS;
TS ts;

// forward declarations
Z T mt(TT t);
// new,  print,     init       
T  nt(V); V pt(T t); V Ti(S a);

#endif