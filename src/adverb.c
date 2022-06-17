#include "a.h"
#include "adverb.h"
#include "verb.h"

// adverbs table
W adverbs[] = {over, scan, NULL, NULL, NULL, NULL};

// x f/ y
K over(K f, K x, K y){
    y = expand(y);
    for (uint64_t i = 0; i < yn; ++i){
        x = cat(enlist(x), enlist(yk[i]));
        x = dotApply(ref(f), x);
    }
    unref(f), free(y);
    return x;
}

K scan(K f, K x, K y){
    y = expand(y);
    K r = k(KK, yn);
    for (uint64_t i = 0; i < rn; ++i){
        x = cat(enlist(x), enlist(yk[i]));
        rk[i] = ref(dotApply(ref(f), x));
        x = rk[i];
    }
    unref(f), unref(rk[rn-1]), free(y);
    return squeeze(r);
}

// adverb helper function
// some adverb-modified dyads can be applied prefix and infix 
// eg "+/ 1 2 3" and "10 +/ 1 2 3"
// in the prefix case, a suitable identity is needed for the seed (left arg)
// eg for dyadic + (plus) the prototype is 0 with type `i
K getIdentity(K x){
    // drill down to function
    while (KOVER <= xt && KEACHPRIOR >= xt)
        x = xk[0];

    // return prototype
    int8_t type = xt;
    
    if (KV == xt){
        uint8_t type = xc[0];
        return 
            0  == type ? Ki(0)   : // +
            1  == type ? Kf(1)   : // *
            2  == type ? Ki(0)   : // -
            13 == type ? k(KK,0) : // ,
            Kerr("error! prototype NYI for verb");
    }
    else if (KK <= type && KT >= type){
        return KNUL;
    }
    else {
        return Kerr("error! prototype NYI");
    }
}
