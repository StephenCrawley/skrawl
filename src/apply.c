
#include "apply.h"
#include "object.h"
#include "verb.h"

// forward declarations
K index(K x, K y);

// f . x
// generic apply function
// f - some applicable value (primitive, lambda, list, etc)
// x - a list of arguments to f 
K apply(K x, K y){
    K t;

    // enlist is special, can take any number of arguments
    if (IS_OP(x,KU,TOK_COMMA))
        return UNREF_X(squeeze(y));

    // index
    if (KL>TYP(x)){
        // x[y]
        if (1==CNT(y))
            return t=ref(*OBJ(y)),unref(y),index(x,t); //TODO: replace with first()

        // x[y;...]
        else {
            t=tn(KK,1); //reusable box
            for (i64 i=0,n=CNT(y); i<n; i++){
                // box y[i]
                *OBJ(t)=OBJ(y)[i];
                // apply x to y[i]
                x=apply(x,ref(t));
                // handle error
                if (IS_ERROR(x))
                    return unref(t),UNREF_Y(x);
            }
            return unref(t),UNREF_Y(x);
        }
    }

    return UNREF_XY(kerr(kC0("'nyi! apply")));
}

// index x at y
K index(K x, K y){
    // init
    K r,t;
    i8 xt=TYP(x), yt=TYP(y);
    i64 n=CNT(y);

    // return type error if x is atom
    if (0>xt)
        return UNREF_XY(kerr(kC0("'type! can't index an atom")));
    
    // handle dicts. indexed using find()
    if (KD==xt){
        // get the keys to search on
        K k=*OBJ(x);
        // find the indexes of the keys
        K ix=find(ref(k),y);
        // return if error finding keys
        if (IS_ERROR(ix))
            return UNREF_X(ix);
        // index the dict values
        return UNREF_X(index(ref(OBJ(x)[1]),ix));
    }

    // if y is general list, index for each item
    if (!yt){
        r=tn(KK,0);
        for (i64 i=0; i<n; i++){
            // call on y[i]
            t=index(ref(x),ref(OBJ(y)[i]));
            // handle if error
            if (IS_ERROR(t))
                return UNREF_XYR(t);
            // else append to return object
            r=jk(r,t);
        }
        return UNREF_XY(squeeze(r));
    }

    // from here we're dealing with indexing with ints
    // so if y not int, return error
    if (KI!=ABS(yt))
        return UNREF_XY(kerr(kC0("'type! y is not valid index")));

    // if x is generic and y is atom, return ref(x[*y])
    if (!xt && 0>yt)
        return UNREF_XY( ref(OBJ(x)[*INT(y)]) );
    
    // return object
    r=tn(yt>0?xt:-xt,n);

    i64 j, xn=CNT(x);
    switch (xt){
    case KK: for (i64 i=0; i<n; i++) j=INT(y)[i], OBJ(r)[i]=(j<0||j>=xn)?ku(0):OBJ(x)[j]; break;
    case KC: for (i64 i=0; i<n; i++) j=INT(y)[i], CHR(r)[i]=(j<0||j>=xn)?   32:CHR(x)[j]; break;
    case KI: for (i64 i=0; i<n; i++) j=INT(y)[i], INT(r)[i]=(j<0||j>=xn)?    0:INT(x)[j]; break;
    case KF: for (i64 i=0; i<n; i++) j=INT(y)[i], FLT(r)[i]=(j<0||j>=xn)?    0:FLT(x)[j]; break;
    }

    // if x was generic list, ref each element of the output
    if (!xt)
        for (i64 i=0; i<n; i++) ref(OBJ(r)[i]);

    return UNREF_XY(squeeze(r));
}
