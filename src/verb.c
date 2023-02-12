#include "verb.h"
#include "object.h"

//                 : + - * % , ?    . @ ! $ # _ ^ & ? = < > ~ |
DYAD dyad_table[]={0,0,0,0,0,0,find,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// x?y for KI and KS (syms encoded in i64)
K findSym(K x, K y){
    // init 
    i64 xn=CNT(x);
    i64 yn=CNT(y);
    K r=tn(KI,yn);

    // pointers to use in the loop
    i64 *xptr=INT(x), *rptr=INT(r);

    // for each y element
    for (i64 i=0; i<yn; i++){
        i64 j=0, yi=INT(y)[i];

        // search for a match in x
        for (; j<xn; j++){
            if (yi==xptr[j])
                break;
        }
        rptr[i]=j;
    }

    return UNREF_XY(r);
}

K find(K x, K y){
    // init
    i8  xt=TYP(x);
    i8 axt=ABS(xt);
    i8  yt=TYP(y);

    // for now, just find for same-type operands
    if (axt!=ABS(yt)){
        unref(x), unref(y);
        return kerr(kC0("'nyi! x?y for differing types"));
    }

    switch (axt){
        case KI: /* KS is i64 so same logic applies */
        case KS: return findSym(x,y);
        default: return UNREF_XY( kerr(kC0("'nyi! x?y for given types")) );
    }
}

