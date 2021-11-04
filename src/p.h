#ifndef PARSE
#define PARSE
#include "a.h"
//TODO : replace this debugging with ifdef
#define DBGT 0  // debug token
#define DBGP 0  // debug parse

/*            (  )  {  }  [  ]  ;  :  =  <  >  |  ?  +  -  *  %  !  "  @  ~  #  ,  .  '  /  \  1   2.3 "a" foo \0  not recognised */
typedef enum {LP,RP,LB,RB,LS,RS,SC,CL,EQ,LA,RA,PI,QM,PL,HY,ST,DV,BA,QT,AT,TL,HS,CM,DT,AP,FS,BS,INT,FLT,STR,ID, END,NR}TT;
typedef struct {S s;I l;TT t;}T; // token. start,length,token type
typedef struct {S s,c;T b[128];T *bp;}TS; // token scanner. start,current,buffer,buffer pointer
TS ts;

//TODO : full variable name support
K vt[26]; // value table

#define ZT static T

// forward declarations
T nt();V Ti(S a);V pt(T t);V pT();V rt();K pr(T *tk);

#endif