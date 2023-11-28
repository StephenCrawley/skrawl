#include "adverb.h"
#include "object.h"
#include "apply.h"

// x f\:y
// where f is a function pointer
K applyleft(K x, K*y, i64 n){
    // can only map (int-indexable) lists
    if (IS_ATOM(x) || TYP(x)==KD){
        UNREF_N_OBJS(y,n);
        return UNREF_X(kerr("'rank!"));
    }

    // map over x
    K r=tn(KK,0),t;
    for (i64 i=0,xn=KCOUNT(x); i<xn; i++){
        REF_N_OBJS(y,n);
        t=apply(item(i,x),y,n);
        if (IS_ERROR(t)){
            UNREF_N_OBJS(y,n);
            return UNREF_XR(t);
        }
        r=jk(r,t);
    }
    return UNREF_X(squeeze(r));
}

// x f'y
K each2(K f, K*a){
    // extract x and y
    K x=a[0], y=a[1];
    
    // can't map atoms
    bool xatom=IS_ATOM(x);
    bool yatom=IS_ATOM(y);
    if (xatom && yatom){
        unref(f);
        UNREF_N_OBJS(a,2);
        return kerr("'rank!");
    }

    i64 xn=KCOUNT(x), yn=KCOUNT(y);
    K r=tn(KK,0),t;
    if (yatom){
        for (i64 i=0; i<xn; i++){
            K args[]={item(i,x),ref(y)};
            t=apply(ref(f),args,2);
            if (IS_ERROR(t)){
                UNREF_N_OBJS(a,2);
                unref(f),unref(r);
                return t;
            }
            r=jk(r,t);
        }
    }
    else if (xatom){
        for (i64 i=0; i<yn; i++){
            K args[2]={ref(x),item(i,y)};
            t=apply(ref(f),args,2);
            if (IS_ERROR(t)){
                UNREF_N_OBJS(a,2);
                unref(f),unref(r);
                return t;
            }
            r=jk(r,t);
        }
    }
    else {
        // lists must be same length
        if (xn!=yn){
            unref(r),unref(f);
            return kerr("'length");
        }

        for (i64 i=0; i<xn; i++){
            K args[]={item(i,x),item(i,y)};
            t=apply(ref(f),args,2);
            if (IS_ERROR(t)){
                UNREF_N_OBJS(a,2);
                unref(f),unref(r);
                return t;
            }
            r=jk(r,t);
        }
    }
    unref(f);
    UNREF_N_OBJS(a,2);
    return squeeze(r);
}
