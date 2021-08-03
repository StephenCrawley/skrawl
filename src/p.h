#ifndef PARSE
#define PARSE
#include "a.h"
#define DBGT 0 //<! debug token
#define DBGP 0 //<! debug parse

I vt[26]; //<! value table (TODO: move to vm module)

/*            (  )  {  }  [  ]  ;  :  =  <  >  |  ?  +  -  *  %  !  "  @  ~  #  ,  .  '  /  \  1   "a" foo \0  not recognised */
typedef enum {LP,RP,LB,RB,LS,RS,SC,CL,EQ,LA,RA,PI,QM,PL,HY,ST,DV,BA,QT,AT,TL,HS,CM,DT,AP,FS,BS,NUM,STR,ID, END,NR}TT; 
typedef struct {S s;I l;TT t;}T; //<! token. start,length,token type
typedef struct {S s,c;T b[128];T *bp;}TS; //<! token scanner. start,current,buffer,buffer pointer
TS ts;

typedef enum{OK,ERR}PR; //<! parse result

#define ZT static T

// forward declarations
T nt();V Ti(S a);V pt(T t);V pT();V rt();I pr(T *tk);

#endif