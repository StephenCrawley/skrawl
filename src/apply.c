#include "apply.h"
#include "object.h"
#include "verb.h"
#include "parse.h"
#include "adverb.h"

// forward declarations
K index(K x, K y);

// f . x
// generic apply function
// f - some applicable value (primitive, lambda, list, etc)
// x - a list of arguments to f 
K apply(K x, K y){
    i8  xt=TYP(x);
    i64 yn=CNT(y);

    // simple atom is not an applicable value
    // (some symbols are special, they have special functions)
    if (xt<0 && xt!=-KS)
        return UNREF_XY(kerr("rank! atom not an applicable value"));

    // index
    if (xt>=0 && xt<K_INDEXABLE_END){
        // x[y]
        if (yn==1)
            return index(x,first(y)); 

        // x[y;...]
        for (i64 i=0; i<yn; i++){
            // handle elided index. x[;i] -> x@\:i
            if (IS_MAGIC_VAL(OBJ(y)[i]))
                return (yn==++i) ? UNREF_Y(x) : mapleft(apply,x,sublist(y,i,yn-i));
            // apply to next index
            x=apply(x,k1(ref(OBJ(y)[i])));
            // handle error
            if (IS_ERROR(x))
                break;
        }
        return UNREF_Y(x);
    }

    // if any magic values, return projection
    for (i64 i=0; i<yn; i++)
        if (IS_MAGIC_VAL(OBJ(y)[i]))
            return tx(KP,j2(k1(x),y));

    // enlist is special, can take any number of arguments
    if (IS_MONAD(x,TOK_COMMA))
        return squeeze(y);

    // symbols
    if (xt==-KS){
        // syms can only be applied monadic
        if (yn!=1)
            return UNREF_XY(kerr("'rank! sym can only be monadic"));

        y=first(y);
        switch(*INT(x)){
        // `p@x -> return parse tree
        case 'p':
            if (TYP(y)!=KC)
                return UNREF_XY(kerr("'type! can only parse char vector"));
            y=j2(y,kc(0));
            return UNREF_XY(parse((const char*)y));

        default : return UNREF_XY(kerr("'type! symbol not an applicable value"));
        }
    }

    return UNREF_XY(kerr("'nyi! apply"));
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
    // x[::] -> return x
    if (IS_MONAD(y,TOK_COLON) || IS_MAGIC_VAL(y))
        return x;
        
    // init
    K r,t;
    i8  xt=TYP(x), yt=TYP(y);
    i64 xn=CNT(x), yn=CNT(y);

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

    // handle dicts. indexed using find()
    if (xt==KD){
        // get the keys to search on
        K k=KEY(x);
        // find the indexes of the keys
        K ix=find(ref(k),y);
        // return if error finding keys
        if (IS_ERROR(ix))
            return UNREF_X(ix);
        // index the dict values
        return UNREF_X(index(ref(OBJ(x)[1]),ix));
    }

    // handle tables. call index for each column
    if (xt==KT){
        // extract the underlying dict
        K dict=*OBJ(x);

        // if y is sym we're indexing by column, extract columns as if dict
        if (ABS(yt)==KS)
            return UNREF_X(index(ref(dict),y));

        // else we're indexing by row 
        // extract keys and vals and create new column object
        K key=KEY(dict);
        K val=VAL(dict);
        K r=tn(KK,0);

        // iterate and index each column 
        for (i64 i=0,n=CNT(val); i<n; i++){
            t=index(ref(OBJ(val)[i]),ref(y));
            // return if error
            if (IS_ERROR(t))
                return UNREF_XYR(t);
            // else append
            r=jk(r,squeeze(t));
        }

        // make dict with new indices
        r=kD(ref(key),squeeze(r));
        // if y is atom, return dict. else return table
        return UNREF_XY(yt<0?r:kT(r));
    }

    // from here we're dealing with indexing with ints
    // so if y not int, return error
    if (ABS(yt)!=KI)
        return UNREF_XY(kerr("'type! y is not valid index"));

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
