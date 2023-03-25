#include "adverb.h"
#include "object.h"
#include "apply.h"

// x f\:y
// where f is a function pointer
K mapleft(DYAD f, K x, K y){
    // can only map (int-indexable) lists
    if (IS_ATOM(x) || TYP(x)==KD)
        return UNREF_XY(kerr("'rank!"));

    // map over x
    K r=tn(KK,0),t;
    for (i64 i=0,n=KCOUNT(x); i<n; i++){
        t=f(item(i,x),ref(y));
        if (IS_ERROR(t))
            return UNREF_XYR(t);
        r=jk(r,t);
    }
    return UNREF_XY(squeeze(r));
}

// x f'y
K each2(K f, K a){
    // extract x and y
    K x=OBJ(a)[0], y=OBJ(a)[1];
    
    // can't map atoms
    bool xatom=IS_ATOM(x);
    bool yatom=IS_ATOM(y);
    if (xatom && yatom){
        unref(f),unref(a);
        return kerr("'rank!");
    }

    // lists must have same count
    i64 xn=KCOUNT(x), yn=KCOUNT(y);
    if (!xatom && !yatom && xn!=yn){
        unref(f),unref(a);
        return kerr("'length");
    }

    K r=tn(KK,0),t;
    if (xn==yn){
        for (i64 i=0; i<xn; i++){
            t=k2(item(i,x),item(i,y));
            t=apply(ref(f),t);
            if (IS_ERROR(t)){
                unref(f),unref(a),unref(r);
                return t;
            }
            r=jk(r,t);
        }
    }
    else if (xn>yn){
        for (i64 i=0; i<xn; i++){
            t=k2(item(i,x),ref(y));
            t=apply(ref(f),t);
            if (IS_ERROR(t)){
                unref(f),unref(a),unref(r);
                return t;
            }
            r=jk(r,t);
        }
    }
    else {
        for (i64 i=0; i<yn; i++){
            t=k2(ref(x),item(i,y));
            t=apply(ref(f),t);
            if (IS_ERROR(t)){
                unref(f),unref(a),unref(r);
                return t;
            }
            r=jk(r,t);
        }
    }
    unref(f),unref(a);
    return squeeze(r);
}
