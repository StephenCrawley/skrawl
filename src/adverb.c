#include "a.h"
#include "adverb.h"
#include "verb.h"

// adverbs table
V adverbs[] = {over, scan, NULL, eachLeft, eachRight, NULL};

K over(K f, K x){
    K r, t;

    // f/x
    if (1 == xn){
        // extract arg. if it's empty return empty.
        x = ( KK == xt ) ? expand(first(x)) : expand(x);
        if (0 == xn){
            return unref(f), x;
        }

        r = xk[0];
        for (uint64_t i = 1; i < xn; ++i){
            r = dotApply(ref(f), K_JOIN2(r, xk[i]));
        }
        unref(f), free(x);
        return r;
    }
    // f/[x;y]
    else if (2 == xn){
        r = xk[0];
        t = expand(xk[1]);
        free(x);
        x = t;

        // if any empty, return empty
        if (0 == xn || 0 == rn){
            return unref(f), unref(x), unref(r), k(KK,0);
        }

        for (uint64_t i = 0; i < xn; ++i){
            r = dotApply(ref(f), K_JOIN2(r, xk[i]));
        }
        unref(f), free(x);
        return r;
    }
    // f/[x;y;...]
    else {
        unref(f), unref(x);
        return Kerr("error! over for f with rank >2 nyi");
    }
}

K scan(K f, K x){
    K r, t;

    // f\x
    if (1 == xn){
        // extract arg. if it's empty return empty.
        x = ( KK == xt ) ? expand(first(x)) : expand(x);
        if (0 == xn){
            return unref(f), x;
        }

        r = k(KK, xn);
        rk[0] = xk[0];
        for (uint64_t i = 1; i < rn; ++i){
            rk[i] = dotApply(ref(f), K_JOIN2(ref(rk[i-1]), xk[i]));
        }
        r = ( 1 == xn ) ? first(r) : squeeze(r);
        unref(f), free(x);
        return r;
    }
    else if (2 == xn){
        // if any empty, return empty
        if (0 == COUNT(xk[0]) || 0 == COUNT(xk[1])){
            return unref(f), unref(x), k(KK,0);
        }

        t = expand(xk[1]);
        r = k(KK, tn);
        rk[0] = xk[0];
        free(x), x = t;

        t = rk[0];
        tr--;
        for (uint64_t i = 0; i < xn; ++i){
            rk[i] = dotApply(ref(f), K_JOIN2(ref(t), xk[i]));
            t = rk[i];
        }
        unref(f), free(x);
        return squeeze(r);
    }
    else {
        unref(f), unref(x);
        return Kerr("error! scan for f with rank >2 nyi");
    }
}

K eachLeft(K f, K x){
    if (2 != xn){
        unref(f), unref(x);
        return Kerr("error! not enough args supplied to \\: (each-left).");
    }
    x = expand(x);
    K t = expand(xk[0]), y = xk[1];
    free(x);
    x = t;
    K r = k(KK, xn);
    for (uint64_t i = 0; i < rn; ++i){
        t = K_JOIN2(xk[i], ref(y));
        rk[i] = dotApply(ref(f), t);
    }
    unref(f), free(x), unref(y);
    return r;
}

K eachRight(K f, K x){
    if (2 != xn){
        unref(f), unref(x);
        return Kerr("error! not enough args supplied to /: (each-right).");
    }
    x = expand(x);
    K t = xk[0], y = expand(xk[1]);
    free(x);
    x = t;
    K r = k(KK, yn);
    for (uint64_t i = 0; i < rn; ++i){
        t = K_JOIN2(ref(x), yk[i]);
        rk[i] = dotApply(ref(f), t);
    }
    unref(f), unref(x), free(y);
    return r;
}
