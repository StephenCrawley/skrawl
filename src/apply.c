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
    if (TYP(x)<KINDEXABLE_END){
        // x[y]
        if (CNT(y)==1)
            return index(x,first(y)); 

        // x[y;...]
        t=tn(KK,1); //reusable box
        for (i64 i=0,n=CNT(y); i<n; i++){
            // box y[i]
            *OBJ(t)=OBJ(y)[i];
            // apply x to y[i]
            x=apply(x,ref(t));
            // handle error
            if (IS_ERROR(x))
                break;
        }
        *OBJ(t)=knul();
        unref(t);
        return UNREF_Y(x);
    }

    return UNREF_XY(kerr(kC0("'nyi! apply")));
}

// get object with same shape, replacing all values with nulls
static K nulls(K x){
    if (!CNT(x)) return ref(x);

    K   r;
    i8  xt=TYP(x);
    i64 xn=CNT(x);

    // call recursively on generic objects
    if (!xt){
        r=tn(xt,xn);
        for (i64 i=0; i<xn; i++)
            OBJ(r)[i]=nulls(OBJ(x)[i]);
        return r;
    }

    // else create nulls
    switch(ABS(xt)){
    case KC: r=tn(xt,xn); for (i64 i=0; i<xn; i++) CHR(r)[i]=CNULL; return r;
    case KI: r=tn(xt,xn); for (i64 i=0; i<xn; i++) INT(r)[i]=INULL; return r;
    case KF: r=tn(xt,xn); for (i64 i=0; i<xn; i++) FLT(r)[i]=FNULL; return r;
    case KS: r=tn(xt,xn); for (i64 i=0; i<xn; i++) INT(r)[i]=SNULL; return r;
    case KD: { K k=ref(*OBJ(x)); return kD(k,nulls(OBJ(x)[1])); }
    default: return ku(0);
    }
}

// index x at y
K index(K x, K y){
    // init
    K r,t;
    i8  xt=TYP(x), yt=TYP(y);
    i64 xn=CNT(x), yn=CNT(y);

    // return type error if x is atom
    if (xt<0)
        return UNREF_XY(kerr(kC0("'type! can't index an atom")));
    
    // handle dicts. indexed using find()
    if (xt==KD){
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
        for (i64 i=0; i<yn; i++){
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
    if (ABS(yt)!=KI)
        return UNREF_XY(kerr(kC0("'type! y is not valid index")));

    // if x is generic and y is atom
    if (!xt && yt<0){
        // ()[0] -> ()
        if (!xn) return UNREF_Y(x);
        // return ref(x[*y]) or nulls if out of bounds
        i64 j=*INT(y);
        return UNREF_XY((j>=0&&j<xn) ? ref(OBJ(x)[*INT(y)]) : nulls(*OBJ(x)));
    }
    
    // return object
    r=tn(yt>=0?xt:-xt,yn);

    i64 j;
    K nl=0;
    switch (xt){
    case KK: 
        for (i64 i=0; i<yn; i++){
            j=INT(y)[i];
            OBJ(r)[i]= (j>=0&&j<xn) ? ref(OBJ(x)[j]) : !nl?nl=nulls(xn?*OBJ(x):x):ref(nl);
        }
        break;
    case KC: for (i64 i=0; i<yn; i++){ j=INT(y)[i], CHR(r)[i]=(j>=0&&j<xn)?CHR(x)[j]:CNULL; } break;
    case KI: for (i64 i=0; i<yn; i++){ j=INT(y)[i], INT(r)[i]=(j>=0&&j<xn)?INT(x)[j]:INULL; } break;
    case KF: for (i64 i=0; i<yn; i++){ j=INT(y)[i], FLT(r)[i]=(j>=0&&j<xn)?FLT(x)[j]:FNULL; } break;
    case KS: for (i64 i=0; i<yn; i++){ j=INT(y)[i], INT(r)[i]=(j>=0&&j<xn)?INT(x)[j]:SNULL; } break;
    }

    return UNREF_XY(squeeze(r));
}
