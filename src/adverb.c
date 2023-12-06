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

// are all objects atomic?
static bool allAtomic(K*x, i64 n){
    for (i64 i=0; i<n; i++)
        if(!IS_ATOM(x[i]))return false;
    return true;
}

// return the length to iterate over
// do all args conform? (ie same length if vectors?)
// -1 means args don't conform
static i64 argsCount(K*x, i64 n){
    i64 i=0,cnt;
    while (IS_ATOM(x[i])) i++;
    cnt=KCOUNT(x[i]);
    for (i64 j=i+1;j<n;j++)
        if (!IS_ATOM(x[j]) && KCOUNT(x[j])!=cnt) return -1;
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

// x f'y
K each(K f, K*a, i64 n){
    K r;

    if (allAtomic(a,n))
        return apply(f,a,n);

    // vectors must conform
    // argsCount returns -1 if they don't
    // otherwise returns the cnt to iterate
    i64 cnt=argsCount(a,n);
    if (cnt == -1){
        r=kerr("'length!");
        goto cleanup;
    }

    // iterate and apply f
    K args[8];
    r=tn(KK,0);
    for (i64 i=0; i<cnt; i++){
        fillArgs(args,a,n,i);
        K t=apply(ref(f),args,n);
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
