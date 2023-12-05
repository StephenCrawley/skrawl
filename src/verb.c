#include "verb.h"
#include "object.h"
#include "apply.h"
#include "dyad.h"

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
    return IS_ATOM(x) ? x : index(TYP(x)==KD ?  value(x) : x, ki(0));
}

K ksqrt(K x){
    return UNREF_X( kerr("'nyi! monad %") );
}

// ,1 -> ,1
//K enlist(K x){
//    return squeeze(k1(x));
//}

K distinct(K x){
    return UNREF_X( kerr("'nyi! monad ?") );
}

K value(K x){
    return TYP(x)==KD ? UNREF_X(ref(VAL(x))) : UNREF_X( kerr("'nyi! monad .") );
}

K type(K x){
    return UNREF_X( kerr("'nyi! monad @") );
}

// !n. called in getKey. used internally 
K til(i64 n){
    K r=tn(KI,n);
    for (i64 i=0; i<n; i++) INT(r)[i]=i;
    return r;
}

// !x
K getKey(K x){
    i8 xt=TYP(x);

    // !3 -> 0 1 2
    if (xt==-KI){
        i64 n=*INT(x);
        unref(x);
        return n>=0 ? til(n) : kerr("'domain! !x (key) - x must be positive int");
    }

    // !`a`b!1 2 -> `a`b
    if (xt==KD)
        return UNREF_X(ref(KEY(x)));
    
    return UNREF_X( kerr("'type! !x (key) - invalid operand") );
}

K string(K x){
    return UNREF_X( kerr("'nyi! monad $") );
}

K count(K x){
    return UNREF_X( ki(KCOUNT(x)) );
}

K lower(K x){
    return UNREF_X( kerr("'nyi! monad _") );
}

K isNull(K x){
    return UNREF_X( kerr("'nyi! monad ^") );
}

K where(K x){
    // only valid with KI objects
    if (TYP(x)!=KI) return UNREF_X(kerr("'type! &x (where) - arg must be int list"));

    // get count of return vector
    i64 xn=HDR_CNT(x),rn=0;
    for (i64 i=0; i<xn; i++){
        i64 val=INT(x)[i];
        if (val<0) return UNREF_X(kerr("'value! &x (where) - values must be >=0"));
        rn+=val;
    }

    // create and fill the return vector
    K r=tn(KI,rn);
    for (i64 i=0,j=0; i<xn; i++){
        i64 val=INT(x)[i];
        if (val) for (i64 k=0; k<val; k++) INT(r)[j++]=i;
    }

    return UNREF_X(r);
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
    // return atom
    if (IS_ATOM(x)) return x;

    // if dict, reverse keys and vals
    if (TYP(x) == KD){
        K keys=reverse(ref(KEY(x)));
        K vals=reverse(ref(VAL(x)));
        return UNREF_X(kD(keys,vals));
    }

    // reverse int-indexable vectors
    i64 n=KCOUNT(x);
    K reverse_inds=tn(KI,n);
    for (i64 i=0; i<n; i++) INT(reverse_inds)[i] = n-i-1;

    return atApply(x,reverse_inds);
}


// DYAD definitions //


// x?y for KI and KS (syms encoded in i64)
K findSym(K x, K y){
    // init 
    i64 xn=CNT(x);
    i64 yn=CNT(y);
    K r=tn(TYP(y)<0?-KI:KI,yn);

    // for each y element
    for (i64 i=0; i<yn; i++){
        // search for a match in x
        i64 j=0;
        for (; j<xn; j++){
            if (INT(y)[i]==INT(x)[j])
                break;
        }
        INT(r)[i]=j;
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
    if (IS_ATOM(x)) x=enlist(x);
    if (IS_ATOM(y)) y=enlist(y);

    return kD(x,y);
}

// x:y
// return the right argument
K dex(K x, K y){
    return UNREF_X(y);
}

K add(K x, K y){
    return execDyad('+',x,y);
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

// x . y
K dotApply(K x, K y){
    if (IS_ATOM(y))
        return UNREF_XY(kerr("'rank! . (apply) - right arg must be list"));
    y=expand(TYP(y)==KD ? value(y) : y);
    i64 n=HDR_CNT(y);
    REF_N_OBJS(OBJ(y),n);
    return UNREF_Y(apply(x,OBJ(y),n));
}

K atApply(K x, K y){
    return apply(x,&y,1);
}

K cast(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad $") );
}

// 
K n_take(i64 n, K y){
    K r;

    if (TAG_TYP(y)) y=enlist(y);
    i8 yt=HDR_TYP(y);

    // x#d -> (x#!d)!x#.d
    if (yt==KD){
        K newkey=n_take(n,ref(KEY(y)));
        r=kD(newkey,n_take(n,ref(VAL(y))));
        return UNREF_Y(r);
    }

    // x#table -> iterate each column
    if (yt==KT){
        r=tn(KK,0);
        K dict=*OBJ(y);
        K cols=VAL(dict);
        for (i64 i=0,cn=CNT(cols); i<cn; i++)
            r=jk(r,n_take(n,ref(OBJ(cols)[i])));
        return UNREF_Y(kT(kD(ref(KEY(dict)),r)));
    }

    i64 rn=ABS(n);
    i8  rt=ABS(yt);

    // if 0#y, return empty list
    if (!rn) return UNREF_Y(tn(rt,rn));

    // if y has 0 count, then result is the same as index out of bounds
    i64 yn=CNT(y);
    if (!yn)
        return index(y,til(rn));

    r=tn(rt,rn);          //return object
    i64 sz=ksize(y);      //elemental size of y
    i64 rbytes=sz*rn;     //remaining number of bytes to copy into r
    u8 *ptr=CHR(r);       //pointer to where the next bytes get copied

    // if -x#y copy in the tail
    // -5#0 1 2 -> 1 2 0 1 2
    // copies the first 1 2
    if (n<0){
        i64 offset=rn%yn, m=sz*offset;
        ptr=m+(u8*)memcpy(ptr,CHR(y)+sz*(yn-offset),m);
        rbytes-=m;
    }

    // copy all of y, rn/yn times 
    i64 yb=sz*yn;
    while (rbytes >= yb){
        ptr=yb+(u8*)memcpy(ptr,CHR(y),yb);
        rbytes-=yb;
    }

    // copy the remainder
    memcpy(ptr,CHR(y),rbytes);

    // increment refcount if generic list
    if (!rt)
        for (i64 i=0; i<rn; i++) ref(OBJ(r)[i]);

    return UNREF_Y(squeeze(r));
}

// x#y
K take(K x, K y){
    if (TYP(x)!=-KI)
        return UNREF_XY(kerr("'type! x#y (take) - x must be int atom"));
    return UNREF_X(n_take(*INT(x),y));
}

// x_y
// 2_0 1 2 3 -> 2 3
// drop defined in terms of take
K drop(K x, K y){
    if (TYP(x)!=-KI)
        return UNREF_XY(kerr("'type! x_y (drop) - x must be int atom"));

    i64 n=*INT(x), m=KCOUNT(y);
    return UNREF_X(n_take(ABS(n)>=m ? 0 : n>0 ? n-m : n+m,y));
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
    return execDyad('<',x,y);
}

K greaterThan(K x, K y){
    return execDyad('>',x,y);
}

bool isMatch(K x, K y){
    // same object?
    if (x==y)
        return 1;

    // if both tag type, can't match as x==y would be true
    if (TAG_TYP(x) && TAG_TYP(y))
        return 0;

    // different types?
    if (TYP(x)!=TYP(y))
        return 0;

    // different counts?
    i64 xn=CNT(x);
    if (xn!=CNT(y))
        return 0;

    // recurse generic types
    if (IS_GENERIC(x)){
        for (i64 i=0; i<xn; i++){
            if (!match(OBJ(x)[i],OBJ(y)[i]))
                return 0;
        }
        return 1;
    }

    // simple types
    return !memcmp(CHR(x),CHR(y),xn*ksize(x));
}

// x~y
// do x and y match? same type, length, value
K match(K x, K y){
    return UNREF_XY(ki(isMatch(x,y)));
}

K max(K x, K y){
    return UNREF_XY( kerr("'nyi! dyad |") );
}

