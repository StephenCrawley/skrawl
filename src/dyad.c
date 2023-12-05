// functionality for dyad arithmetic and comparison ops

#include "dyad.h"
#include "object.h"

#define DYAD_ITER_TYPE(op,rt,xt,yt) if      (xn>yn) for (i64 i=0; i<rn; i++) rt(r)[i] = xt(x)[i] op yt(y)[0]; \
                                    else if (xn<yn) for (i64 i=0; i<rn; i++) rt(r)[i] = xt(x)[0] op yt(y)[i]; \
                                    else            for (i64 i=0; i<rn; i++) rt(r)[i] = xt(x)[i] op yt(y)[i]; 

#define DYAD_ITER(op) if      (xt==KI && yt==KI)  { DYAD_ITER_TYPE(op,INT,INT,INT) } \
                      else if (xt==KI && yt==KC)  { DYAD_ITER_TYPE(op,INT,INT,CHR) } \
                      else if (xt==KC && yt==KI)  { DYAD_ITER_TYPE(op,INT,CHR,INT) } \
                      else  /*(xt==KC && yt==KC)*/{ DYAD_ITER_TYPE(op,INT,CHR,CHR) } 

#define SWAP(a,b) {K t=b; b=a; a=t; op=(op=='<')?'>':(op=='>')?'<':op;}

K execDyad(char op, K x, K y){
    // ensure x is generic if one of the args is generic
    if (TYP(x) && !TYP(y)) SWAP(x,y);

    // metadata
    i8 xt=TYP(x), yt=TYP(y);
    i64 xn=KCOUNT(x), yn=KCOUNT(y);

    // bail if vectors don't conform
    if (!IS_ATOM(x) && !IS_ATOM(y) && (xn != yn)){
        return UNREF_XY(kerr("'length! dyad operands must conform"));
    }

    // if an arg is 0-count list, return
    if (!xn) return UNREF_Y(x);
    if (!yn) return UNREF_X(y);

    // if args are generic type, recurse
    if (!xt){
        K r=tn(KK,0),t;
        for (i64 i=0; i<xn; i++){
            t=execDyad(op, ref(OBJ(x)[i]), item(i,y));
            if (IS_ERROR(t)) return UNREF_XYR(t);
            r=jk(r,t);
        }
        return UNREF_XY(r);
    }

    // ensure the wider arg is assigned to x
    //if (xt < TYP(y)) SWAP(x,y);
    
    // initially just for comparison ops
    // so return type is always KI ("bool")
    // also just simple arrays
    i8  rt=( IS_ATOM(x) && IS_ATOM(y) ) ? -KI : KI ;
    i64 rn=MAX(xn,yn);
    K r=tn(rt,rn);

    switch(op){
    case '+': DYAD_ITER(+); break;
    case '<': DYAD_ITER(<); break;
    case '>': DYAD_ITER(>); break;
    default: r=UNREF_R(kerr("'nyi! dyad op"));
    }

    return UNREF_XY(r);
}

