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

// return the count of iterations to perform
// -1 means all args are atomic
// -2 means args don't conform (ie different-count vectors)
// eg f'[1;2 3 4 5] ->  4
//    f'[1 2;3 4 5] -> -2
//    f'[1]         -> -1
static i64 iterCount(K*x, i64 i){
    i64 cnt=-1;
    while (i--){
        if (!IS_ATOM(x[i])){
            i64 n=KCOUNT(x[i]);
            // init cnt if we haven't read a vector yet
            if (cnt == -1){
                cnt = n;
                continue;
            }
            // else check vector counts are equal
            if (cnt != n) return -2;
        }
    }
    return cnt;
}

static void fillArgs(K*d, K*s, i64 n, i64 i){
    switch (n){ 
    case 8: d[7]=item(i,s[7]);  //fall through
    case 7: d[6]=item(i,s[6]);  //fall through
    case 6: d[5]=item(i,s[5]);  //fall through
    case 5: d[4]=item(i,s[4]);  //fall through
    case 4: d[3]=item(i,s[3]);  //fall through
    case 3: d[2]=item(i,s[2]);  //fall through
    case 2: d[1]=item(i,s[1]);  //fall through
    case 1: d[0]=item(i,s[0]);  //fall through
    }
}

// f'x, f'[x;y], ...
K each(K f, K*a, i64 n){
    K r;
    i64 cnt=iterCount(a,n);

    // vectors must conform
    // iterCount returns -2 if they don't
    if (cnt == -2){
        r=kerr("'length!");
        goto cleanup;
    }

    // iterCount returns -1 if all args are atomic
    if (cnt == -1)
        return apply(f,a,n);

    // else iterCount returns the number of iterations to perform
    // iterate and apply f
    K args[8];
    r=tn(KK,0);
    for (i64 i=0; i<cnt; i++){
        fillArgs(args,a,n,i);
        K t = apply(ref(f),args,n);
        if (IS_ERROR(t)){
            replace(&r,t);
            goto cleanup;
        }
        r=jk(r,t);
    }
    r=squeeze(r);

cleanup:
    unref(f);
    UNREF_N_OBJS(a,n);
    return r;
}
