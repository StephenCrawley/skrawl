#include "verb.h"
#include "object.h"
#include "apply.h"

DYAD dyad_table[]={
    set,      add,     subtract, multiply,    divide, join, find,
    dotApply, atApply, key,      cast,        take,   drop, fill,
    min,      equal,   lessThan, greaterThan, match,  max
};

// x?y for KI and KS (syms encoded in i64)
K findSym(K x, K y){
    // init 
    i64 xn=CNT(x);
    i64 yn=CNT(y);
    K r=tn(0>TYP(y)?-KI:KI,yn);

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

// x?y
// for each y, return index of 1st occurrence in x, or CNT(x) if no occurrence
K find(K x, K y){
    // init
    i8  xt=TYP(x), axt=ABS(xt);
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

// x!y
// create a dictionary with x keys and y values
K key(K x, K y){
    if (CNT(x)!=CNT(y))
        return UNREF_XY( kerr(kC0("'length! x!y operand length mismatch")) );
    return kD(TYP(x)>=0?x:va(x), TYP(y)>=0?y:va(y));
}

K set(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad :")) );
}

K add(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad +")) );
}

K subtract(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad -")) );
}

K multiply(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad *")) );
}

K divide(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad %")) );
}

K join(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad ,")) );
}

K dotApply(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad .")) );
}

K atApply(K x, K y){
    return apply(x,k1(y));
}

K cast(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad $")) );
}

K take(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad #")) );
}

K drop(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad _")) );
}

K fill(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad ^")) );
}

K min(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad &")) );
}

K equal(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad =")) );
}

K lessThan(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad <")) );
}

K greaterThan(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad >")) );
}

K match(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad ~")) );
}

K max(K x, K y){
    return UNREF_XY( kerr(kC0("'nyi! dyad |")) );
}

