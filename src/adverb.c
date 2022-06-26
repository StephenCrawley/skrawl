#include "a.h"
#include "adverb.h"
#include "object.h"
#include "verb.h"

// adverbs table
V adverbs[] = {over, scan, each, eachLeft, eachRight, NULL};

K over(K f, K x){
    K r, t;

    // f/x or x f/y
    if (1 == xn || 2 == xn){
        uint64_t n;
        if (1 == xn){
            // extract arg
            x = ( KK == xt ) ? expand(first(x)) : expand(x);
            // if it's empty return empty
            if (0 == xn){
                return unref(f), x;
            }
            r = ref(xk[0]);
            n = 1;
        }
        else {
            // extract args
            x = expand(x);
            r = xk[0];
            t = expand(xk[1]);
            UNREFTOP(x);
            x = t;
            // if any empty, return empty
            if (0 == xn || 0 == rn){
                return unref(f), unref(x), unref(r), k(KK,0);
            }
            n = 0;
        }

        for (uint64_t i = n; i < xn; ++i){
            t = dotApply(ref(f), JOIN2(r, ref(xk[i])));
            if (KE == tt){            // if error was returned                   
                unref(f), unref(x);   // unref the args     
                return t;                                                                 
            }
            r = t;
        }
        unref(f);
        unref(x);
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
        int8_t type = xt;
        x = ( KK == xt ) ? expand(first(x)) : expand(x);
        if (0 == xn){
            return unref(f), x;
        }

        r = k(KK, xn);
        rk[0] = xk[0];
        for (uint64_t i = 1; i < rn; ++i){
            rk[i] = dotApply(ref(f), JOIN2(ref(rk[i-1]), xk[i]));
        }
        // if type is a simple list, the arg was an atom, so return an atom
        r = ( 0 < type ) ? first(r) : squeeze(r);
        unref(f); UNREFTOP(x);
        return r;
    }
    else if (2 == xn){
        x = expand(x);
        // if any empty, return empty
        if (0 == COUNT(xk[0]) || 0 == COUNT(xk[1])){
            return unref(f), unref(x), k(KK,0);
        }

        t = expand(xk[1]);
        r = k(KK, tn);
        rk[0] = xk[0];
        UNREFTOP(x);
        x = t;

        t = rk[0];
        tr--;
        for (uint64_t i = 0; i < xn; ++i){
            rk[i] = dotApply(ref(f), JOIN2(ref(t), xk[i]));
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

K each(K f, K x){
    if (2 < xn){
        unref(f), unref(x);
        return Kerr("error! adverb ' (each) only implemented for functions with rank < 3.");
    }

    K r;

    if (1 < xn){
        r = k(KK, xn);

        // check args conform when eaching multiple lists
        uint64_t n, m;
        bool compare = false;

        for (uint64_t i = 0; i < xn; ++i){
            n = K_COUNT(xk[i]);
            if (compare && n != m){
                unref(f), unref(x);
                return Kerr("length error! args to ' (each) don't conform.");
            }
            else {
                // only interested in comparing lists
                if (!IS_SCALAR(xk[i])){
                    m = n;
                    compare = true;
                }
            }
        }

        if (2 == xn){
            xk[0] = expand(xk[0]);
            xk[1] = expand(xk[1]);
            for (uint64_t i = 0; i < rn; ++i){
                rk[i] = dotApply(ref(f), JOIN2(ref(KOBJ(xk[0])[i]), ref(KOBJ(xk[1])[i])));
            }
        }
    }
    else {
        if (1 == xn) x = expand(first(x));
        r = k(KK, xn);
        for (uint64_t i = 0; i < rn; ++i){
            rk[i] = dotApply(ref(f), ref(xk[i]));
        }
    }

    unref(f), unref(x);
    return squeeze(r);
}

K eachLeft(K f, K x){
    if (2 != xn){
        unref(f), unref(x);
        return Kerr("error! not enough args supplied to \\: (each-left).");
    }
    x = expand(x);
    bool returnAtom = (0 > TYPE(xk[0]) && 0 > TYPE(xk[1]));
    K t = expand(xk[0]), y = xk[1];
    UNREFTOP(x);
    x = t;
    K r = k(KK, xn);
    for (uint64_t i = 0; i < rn; ++i){
        t = JOIN2(ref(xk[i]), ref(y));
        t = dotApply(ref(f), t);
        if (KE == tt){                     // if error was returned                   
            unref(f), unref(x), unref(y);  // unref the args     
            while (i--) unref(rk[i]);      // unref the child objects already assigned 
            free(r);                                                                  
            return t;                                                                 
        }
        rk[i] = t;
    }
    unref(f), unref(x), unref(y);
    return ( returnAtom ) ? first(r) : squeeze(r);
}

K eachRight(K f, K x){
    if (2 != xn){
        unref(f), unref(x);
        return Kerr("error! not enough args supplied to /: (each-right).");
    }
    x = expand(x);
    bool returnAtom = (0 > TYPE(xk[0]) && 0 > TYPE(xk[1]));
    K t = xk[0], y = expand(xk[1]);
    UNREFTOP(x);
    x = t;
    K r = k(KK, yn);
    for (uint64_t i = 0; i < rn; ++i){
        t = JOIN2(ref(x), ref(yk[i]));
        t = dotApply(ref(f), t);
        if (KE == tt){                     // if error was returned                   
            unref(f), unref(x), unref(y);  // unref the args     
            while (i--) unref(rk[i]);      // unref the child objects already assigned 
            free(r);                                                                  
            return t;                                                                 
        }
        rk[i] = t;
    }
    unref(f), unref(x), unref(y);
    return ( returnAtom ) ? first(r) : squeeze(r);
}
