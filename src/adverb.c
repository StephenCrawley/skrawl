#include "adverb.h"
#include "object.h"
#include "apply.h"

// x f\:y
// where f is a function pointer
K applyleft(K x, K*y, i64 n){
    K r,t;
    
    // can only map (int-indexable) lists
    if (IS_ATOM(x) || TYP(x)==KD){
        r=kerr("'rank!");
        goto cleanup;
    }

    // map over x
    r=tn(KK,0);
    for (i64 i=0,xn=KCOUNT(x); i<xn; i++){
        REF_N_OBJS(y,n);
        t=apply(item(i,x),y,n);
        if (IS_ERROR(t)){
            replace(&t,r);
            goto cleanup;
        }
        r=jk(r,t);
    }
    r=squeeze(r);

cleanup:
    UNREF_N_OBJS(y,n);
    return UNREF_X(r);
}

// x f'y
K each2(K f, K*a){
    K r,t;

    // extract x and y
    K x=a[0], y=a[1];

    if (IS_ATOM(x) && IS_ATOM(y))
        return apply(f,a,2);

    i64 xn=KCOUNT(x);
    i64 yn=KCOUNT(y);

    // 2 lists must have same length
    if (!IS_ATOM(x) && !IS_ATOM(y) && xn!=yn){
        r=kerr("'length");
        goto cleanup;
    }

    r=tn(KK,0);

    // iterate and apply f
    for (i64 i=0,n=MAX(xn,yn); i<n; i++){
        K args[]={item(i,x),item(i,y)};
        t=apply(ref(f),args,2);
        if (IS_ERROR(t)){
            replace(&t,r);
            goto cleanup;
        }
        r=jk(r,t);
    }
    r=squeeze(r);

cleanup:
    unref(f);
    UNREF_N_OBJS(a,2);
    return r;
}
