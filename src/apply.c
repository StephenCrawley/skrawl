#include "apply.h"
#include "object.h"
#include "verb.h"
#include "parse.h"
#include "compile.h"
#include "adverb.h"

// forward declarations
K index(K x, K y);
K amend4(K);

// f[;;][1] -> f[1;;]
K fillMV(K x, K y){
    x=reuse(x);
    i64 yn=CNT(y);
    // iterate y
    for (i64 i=0; i<yn; i++){
        // iterate the projection (start at 1st argument)
        for (i64 j=1,xn=CNT(x); j<xn; j++){
            // fill magic values
            if (IS_MAGIC_VAL(OBJ(x)[j])){
                OBJ(x)[j]=ref(OBJ(y)[i]);
                break;
            }
        }
    }
    HDR_RNK(x)-=yn;
    return UNREF_Y(x);
}

// f . x
// generic apply function
// f - some applicable value (primitive, lambda, list, etc)
// x - a list of arguments to f 
K apply(K x, K y){
    K r;
    i8  xt=TYP(x);
    i64 xn=CNT(x), yn=CNT(y);

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
    i8 rn=0;
    for (i64 i=0; i<yn; i++)
        rn+=IS_MAGIC_VAL(OBJ(y)[i]);
    if (rn){
        r=j2(k1(x),y);
        HDR_RNK(r)=rn;
        return tx(KP,r);
    }

    // enlist is special, can take any number of arguments
    if (IS_MONAD(x,TOK_COMMA))
        return squeeze(y);

    i8 rank=IS_OP(x,KV,TOK_AT) ? MIN(4,yn) : RANK(x);

    // too many args
    if (yn>rank){
        return UNREF_XY(kerr("'rank! too many args"));
    }

    // too few args
    if (yn<rank){
        // create projection
        if (xt!=KP) return tx(KP,j2(k1(x),y));
        // else fill in elided args of existing projection
        return fillMV(x,y);
    }

    // symbols
    if (xt==-KS){
        switch(*INT(x)){
        // `p@"x+y" -> return parse tree
        case 'p':
            y=first(y);
            if (TYP(y)!=KC)
                return UNREF_XY(kerr("'type! can only parse char vector"));
            y=j2(y,kc(0));
            return UNREF_XY(parse((const char*)y));
        // `x@"x+y" -> return bytecode
        case 'x':
            r=apply(ks('p'),y);
            return UNREF_X((IS_ERROR(r)||IS_NULL(r)) ? r : UNREF_R(first(compile(r))));

        default : return UNREF_XY(kerr("'type! symbol not an applicable value"));
        }
    }

    // projections
    if (xt==KP){
        x=fillMV(x,y);
        K f=ref(*OBJ(x));
        K a=sublist(x,1,xn-1);
        return apply(f,a);
    }

    if (IS_DERIVED_VERB(x)){
        return each2(UNREF_X(ref(*OBJ(x))),y);
    }

    if (rank==4){
        return amend4(y);
    }
    // apply dyad
    if (xt==KV){
        return UNREF_Y((*dyad_table[TAG_VAL(x)])(ref(OBJ(y)[0]),ref(OBJ(y)[1])));
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
    case KT: return kT(nulls(*OBJ(x)));
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

    // if y is a dict, index like (!y)!x[.y]
    if (yt==KD){
        r=index(ref(x),ref(VAL(y)));
        if (IS_ERROR(r))
            return UNREF_XY(r);
        return UNREF_XY(kD(ref(KEY(y)),r));
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

    // handle table with sym index. this is treated as a dict index
    if (xt==KT && ABS(yt)==KS)
        return UNREF_X(index(ref(*OBJ(x)),y));

    // from here we're dealing with indexing with ints
    // so if y not int, return error
    if (ABS(yt)!=KI)
        return UNREF_XY(kerr("'type! y is not valid index"));

    // handle tables. call index for each column
    if (xt==KT){
        // extract the underlying dict
        K dict=*OBJ(x);
        K key=KEY(dict);
        K val=VAL(dict);
        K r=tn(KK,0);

        // iterate and index each column 
        for (i64 i=0,n=CNT(val); i<n; i++){
            t=index(ref(OBJ(val)[i]),ref(y));
            r=jk(r,t);
        }

        // make dict with new indices
        r=kD(ref(key),squeeze(r));
        // if y is atom, return dict. else return table
        return UNREF_XY(yt<0?r:kT(r));
    }

    // if x is generic and y is atom
    if (!xt && yt<0){
        // ()[0] -> ()
        if (!xn) return UNREF_Y(x);
        // return ref(x[*y]) or nulls if out of bounds
        i64 j=*INT(y);
        return UNREF_XY((j>=0&&j<xn) ? ref(OBJ(x)[j]) : nulls(*OBJ(x)));
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
        r=squeeze(r);
        break;
    case KC: for (i64 i=0; i<yn; i++){ j=INT(y)[i], CHR(r)[i]=(j>=0&&j<xn)?CHR(x)[j]:CNULL; } break;
    case KI: for (i64 i=0; i<yn; i++){ j=INT(y)[i], INT(r)[i]=(j>=0&&j<xn)?INT(x)[j]:INULL; } break;
    case KF: for (i64 i=0; i<yn; i++){ j=INT(y)[i], FLT(r)[i]=(j>=0&&j<xn)?FLT(x)[j]:FNULL; } break;
    case KS: for (i64 i=0; i<yn; i++){ j=INT(y)[i], INT(r)[i]=(j>=0&&j<xn)?INT(x)[j]:SNULL; } break;
    }

    return UNREF_XY(r);
}

// x[i]:y
// checks type
K replaceList(K x, i64 i, K y){
    if (TYP(x) && TYP(x)!=-TYP(y))
        return UNREF_Y(kerr("'type! amend @[x;i;f;y] - can't amend with different type"));

    i64 s=ksize(x);
    if (!TYP(x)) unref(OBJ(x)[i]);
    memcpy(CHR(x)+(s*i),CHR(TYP(x) ? y : (K)&y),s);
    return TYP(x) ? UNREF_Y(x) : x;
}

// d[i]:y
// checks type
K replaceDict(K x, i64 i, K key, K y){
    if (TYP(VAL(x)) && TYP(VAL(x))!=ABS(TYP(y)))
        return UNREF_Y(kerr("'type! amend @[x;i;f;y] - can't amend with different type"));

    // replace existing
    if (CNT(KEY(x))>i){
        i64 s=ksize(x);
        if (!TYP(VAL(x))) unref(OBJ(VAL(x))[i]);
        memcpy(CHR(VAL(x))+(s*i),CHR(TYP(VAL(x)) ? y : (K)&y),s);
        return TYP(VAL(x)) ? UNREF_Y(x) : x;
    }
    // else append new key&val
    else {
        KEY(x)=j2(KEY(x),ref(key));
        VAL(x)=j2(VAL(x),TYP(VAL(x)) ? y : k1(y));
        return x;
    }
}

K amendList(K r, K ix, K f, K y){

    i64 rn=KCOUNT(r);

    // iterate ix
    for (i64 i=0,n=CNT(ix); i<n; i++){
        K indx=IS_ATOM(ix)?ref(ix):item(i,ix);
        if (*INT(indx)<0 || *INT(indx)>=rn){
            unref(indx);
            return kerr("'domain! amend @[x;i;f;y] - index i out of bounds");
        }

        // apply f[x@ix; y@ix]
        K xval=item(*INT(indx),r);
        K yval=item(i,y);
        K rval=apply(ref(f),k2(xval,yval));
        if (IS_ERROR(rval)){
            unref(indx);
            return rval;
        }

        // replace x[i] with the result value
        // x[i]:rval
        r=replaceList(r,*INT(indx),rval);
        if (IS_ERROR(r)){
            unref(indx);
            return r;
        }

        // cleanup
        unref(indx);
    }

    return ref(r);
}

K amendDict(K r, K ix, K f, K y){

    // iterate ix
    for (i64 i=0,n=CNT(ix); i<n; i++){
        // get index of key
        K key=ref(IS_ATOM(ix)?ix:item(i,ix));
        K indx=find(ref(KEY(r)),ref(key));
        if (IS_ERROR(indx)){
            return indx;
        }

        // apply f[x@ix; y@ix]
        K xval=index(ref(VAL(r)),ki(*INT(indx)));
        K yval=item(i,y);
        K rval=apply(ref(f),k2(xval,yval));
        if (IS_ERROR(rval)){
            unref(key);
            unref(indx);
            return rval;
        }

        // replace r[key] with the result value
        // r[key]:rval
        r=replaceDict(r,*INT(indx),key,rval);
        if (IS_ERROR(r)){
            unref(key);
            unref(indx);
            return r;
        }

        // cleanup
        unref(key);
        unref(indx);
    }

    return ref(r);
}

// @[r;i;f;y]
K amend4(K x){
    // reuse r if possible (append in place if refc==0)
    *OBJ(x)=reuse(*OBJ(x));

    // unpack arg
    K  r=OBJ(x)[0];
    K ix=OBJ(x)[1];
    K  f=OBJ(x)[2];
    K  y=OBJ(x)[3];

    i64 rn=KCOUNT(r);

    // @[;::;;] -> @[;!#x;;]
    if (IS_MONAD(ix,TOK_COLON)){
        ix=til(rn);
        OBJ(x)[1]=ix;
    }

    // @[;;;atom] -> @[;;;rn#atom]
    if (IS_ATOM(y)){
        y=take(ki(KCOUNT(ix)),y);
        OBJ(x)[3]=y;
    }

    // i and y must have same count
    if (KCOUNT(ix)!=KCOUNT(y))
        return UNREF_X(kerr("'length! amend @[x;i;f;y] - i and y must conform"));

    return UNREF_X((TYP(r)==KD ? amendDict : amendList)(r,ix,f,y));
}
