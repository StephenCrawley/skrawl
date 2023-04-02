#include "verb.h"
#include "object.h"
#include "apply.h"

MONAD monad_table[]={
    identity, flip,  neg,     first,     ksqrt, enlist,  distinct,
    value,    type,  getKey,  string,    count, lower,   isNull,
    where,    group, gradeUp, gradeDown, not,   reverse
};

DYAD dyad_table[]={
    dex,      add,     subtract, multiply,    divide, join, find,
    dotApply, atApply, makeKey,  cast,        take,   drop, fill,
    min,      equal,   lessThan, greaterThan, match,  max
};


// MONAD definitions //


K identity(K x){
    return x;
}

K flip(K x){
    // dict from table
    if (TYP(x)==KT){
        return UNREF_X(ref(*OBJ(x)));
    }
    // table from dict
    else if (TYP(x)==KD){
        // check that dict is valid 

        // +1 2!(,1;,2) -> error
        if (TYP(KEY(x))!=KS)
            return UNREF_X(kerr("'type! +x (flip) - key must be sym"));

        // +`a`b!1 2 -> error
        K val=VAL(x);
        if (TYP(val)!=KK)
            return UNREF_X(kerr("'rank! +x (flip) - dict value must be list of lists"));

        // +`a`b!(1 2;3 4 5) -> error
        i64 n=CNT(x), m=CNT(OBJ(val)[0]);
        for (i64 i=0; i<n; i++){
            if (CNT(OBJ(val)[i]) != m)
                return UNREF_X(kerr("'length! +x (flip) - dict values must have equal count"));
            
            if (IS_ATOM(OBJ(val)[i]))
                return UNREF_X(kerr("'nyi! +x (flip) - dict values must not be atomic"));
        }

        return kT(x);
    }
    
    // matrix transpose not yet implemented
    return UNREF_X(kerr("'nyi! +:"));
}

K neg(K x){
    return UNREF_X( kerr("'nyi! monad -") );
}

// *x
// *1 2 3 -> 1
K first(K x){
    // init
    K r;
    i8  xt=TYP(x);
    i64 xn=CNT(x);

    // atoms
    if (xt<0 || xt>=K_INDEXABLE_END)
        return x;

    // generic objects
    if (!xt)
        return !xn ? x : UNREF_X(ref(*OBJ(x)));

    // return first item of dict values
    if (xt==KD)
        return UNREF_X(first(ref(OBJ(x)[1])));

    // *[table] returns a dict
    if (xt==KT)
        return index(x,ki(0));

    switch(xt){
    case KC: r=kc(xn ? *CHR(x) : CNULL); break;
    case KI: r=ki(xn ? *INT(x) : INULL); break;
    case KF: r=kf(xn ? *FLT(x) : FNULL); break;
    case KS: r=ks(xn ? *INT(x) : SNULL); break;
    }
    return UNREF_X(r);
}

K ksqrt(K x){
    return UNREF_X( kerr("'nyi! monad %") );
}

K enlist(K x){
    return UNREF_X( kerr("'nyi! monad ,") );
}

K distinct(K x){
    return UNREF_X( kerr("'nyi! monad ?") );
}

K value(K x){
    return UNREF_X( kerr("'nyi! monad .") );
}

K type(K x){
    return UNREF_X( kerr("'nyi! monad @") );
}

K getKey(K x){
    return UNREF_X( kerr("'nyi! monad !") );
}

K string(K x){
    return UNREF_X( kerr("'nyi! monad $") );
}

K count(K x){
    return UNREF_X( kerr("'nyi! monad #") );
}

K lower(K x){
    return UNREF_X( kerr("'nyi! monad _") );
}

K isNull(K x){
    return UNREF_X( kerr("'nyi! monad ^") );
}

K where(K x){
    return UNREF_X( kerr("'nyi! monad &") );
}

K group(K x){
    return UNREF_X( kerr("'nyi! monad =") );
}

K gradeUp(K x){
    return UNREF_X( kerr("'nyi! monad <") );
}

K gradeDown(K x){
    return UNREF_X( kerr("'nyi! monad >") );
}

K not(K x){
    return UNREF_X( kerr("'nyi! monad ~") );
}

K reverse(K x){
    return UNREF_X( kerr("'nyi! monad |") );
}


// DYAD definitions //


// x?y for KI and KS (syms encoded in i64)
K findSym(K x, K y){
    // init 
    i64 xn=CNT(x);
    i64 yn=CNT(y);
    K r=tn(TYP(y)<0?-KI:KI,yn);

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

    return r;
}

// x?y
// for each y, return index of 1st occurrence in x, or CNT(x) if no occurrence
K find(K x, K y){
    // init
    i8  xt=TYP(x), axt=ABS(xt);
    i8  yt=TYP(y);

    // for now, just find for same-type operands
    if (axt!=ABS(yt)){
        return UNREF_XY(kerr("'nyi! x?y (find) - different xy types"));
    }

    switch (axt){
    case KI: /* KS is i64 so same logic applies */
    case KS: return UNREF_XY(findSym(x,y));
    default: return UNREF_XY(kerr("'nyi! x?y (find) - type"));
    }
}

// x!y
// create a dictionary with x keys and y values
K makeKey(K x, K y){
    if (TYP(x)==KD || TYP(y)==KD)
        return UNREF_XY(kerr("'type! x!y (make dict) - operands must be lists"));

    if (KCOUNT(x)!=KCOUNT(y))
        return UNREF_XY(kerr("'length! x!y (make dict) - operand length mismatch"));

    // enlist x and y if needed
    x=IS_ATOM(x) ? va(x) : x;
    y=IS_ATOM(y) ? va(y) : y;

    return kD(x,y);
}

// x:y
// return the right argument
K dex(K x, K y){
    return UNREF_X(y);
}

K add(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad +") );
}

K subtract(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad -") );
}

K multiply(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad *") );
}

K divide(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad %") );
}

K join(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad ,") );
}

K dotApply(K x, K y){
    if (IS_ATOM(y) || TYP(y)==KD)
        return UNREF_XY(kerr("'rank! . (apply) - right arg must be list"));
    return apply(x,expand(y));
}

K atApply(K x, K y){
    return apply(x,k1(y));
}

K cast(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad $") );
}

K take(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad #") );
}

K drop(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad _") );
}

K fill(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad ^") );
}

K min(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad &") );
}

K equal(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad =") );
}

K lessThan(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad <") );
}

K greaterThan(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad >") );
}

K match(K x, K y){
    // same object?
    if (x==y)
        return UNREF_XY(ki(1));

    // different types?
    if (TYP(x)!=TYP(y))
        return UNREF_XY(ki(0));

    // different counts?
    i64 xn=CNT(x);
    if (xn!=CNT(y))
        return UNREF_XY(ki(0));

    // recurse generic types
    if(IS_GENERIC(x)){
        for (i64 i=0; i<xn; i++){
            K t=match(ref(OBJ(x)[i]),ref(OBJ(y)[i]));
            if (!*INT(t))
                return UNREF_XY(t);
            unref(t);
        }
        return UNREF_XY(ki(1));
    }

    // simple types
    K r=ki(!memcmp(CHR(x),CHR(y),xn*ksize(x)));
    return UNREF_XY(r);
}

K max(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad |") );
}

