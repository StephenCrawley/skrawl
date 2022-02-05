#include "a.h"
#include "verb.h"

// dyadic arithmetic macros
#define ADD(x, y)  ((x) + (y))
#define PRD(x, y)  ((x) * (y))
#define SUB(x, y)  ((x) - (y))
#define DIV(x, y)  ((x) / (y))
// dyadic boolean macros
#define LESS(x, y) ((x) < (y))
#define MORE(x, y) ((x) > (y))

// macro to handle errors
// cleans up objects and returns the error
// assumes a temp object t is present
#define HANDLE_IF_ERROR                                                           \
    if (KE == tt){                /* if error was returned */                     \
        unref(x), unref(y);       /* unref the args */                            \
        while (i--) unref(rk[i]); /* unref the child objects already assigned */  \
        free(r);                                                                  \
        return t;                                                                 \
    }

// dyadic arithmetic operation
// k arithmetic ops are "atomic"
// that is, for "atom op vector" the atom recursively drills down to each element of the vector
// eg 1 + (1 2;3 4) is (2 3;4 5)
// furthermore, vectors with conforming lengths can be given as operands to dyadic ops
// eg 1 3 + (4 5 6;7 8 9) is (5 6 7;10 11 12)
// if the vectors don't conform at each level then a length error is thrown

// DYADIC_INIT handles 2 cases
// it calls function f recursively if any of the arguments are general lists
// or it initializes the return value if none of the args are general lists
#define DYADIC_INIT(f, maxtype)                                                  \
    /* if operands are vectors of different lengths, return length error */      \
    if ((KK <= xt && KK <= yt) && xn != yn){                                     \
        unref(x), unref(y);                                                      \
        return Kerr("length error! operands don't conform");                     \
    }                                                                            \
    K r, t; /* declare return and temp value */                                  \
                                                                                 \
    /* 2 cases: */                                                               \
    /*   a) 1 or both of the operands are general lists -> recursively call on each list element and return result */  \
    /*   b) both operands are atomic or simple lists -> set up the return type but don't return anything */ \
                                                                                 \
    /* case a: */                                                                \
    /* TODO : error handling. need to check return val from recursive calls */   \
    if(KK == xt || KK == yt){                                                    \
        r = k(KK, MAX(xn, yn));                                                  \
        if(KK == xt && KK == yt){                                                \
            for(uint64_t i = 0; i < rn; ++i){                                    \
                t = f(ref(xk[i]), ref(yk[i]));                                   \
                HANDLE_IF_ERROR;                                                 \
                rk[i] = t;                                                       \
            }                                                                    \
        }                                                                        \
        else if(KK == xt && 0 > yt){                                             \
            for(uint64_t i = 0; i < rn; ++i){                                    \
                t = f(ref(xk[i]), ref(y));                                       \
                HANDLE_IF_ERROR;                                                 \
                rk[i] = t;                                                       \
            }                                                                    \
        }                                                                        \
        else if(0 > xt && KK == yt){                                             \
            for(uint64_t i = 0; i < rn; ++i){                                    \
                t = f(ref(x),     ref(yk[i]));                                   \
                HANDLE_IF_ERROR;                                                 \
                rk[i] = t;                                                       \
            }                                                                    \
        }                                                                        \
        else if(KK == xt && 0 < yt){                                             \
            y = expand(y);                                                       \
            for(uint64_t i = 0; i < rn; ++i){                                    \
                t = f(ref(xk[i]), ref(yk[i]));                                   \
                HANDLE_IF_ERROR;                                                 \
                rk[i] = t;                                                       \
            }                                                                    \
        }                                                                        \
        else if(0 < xt && KK == yt){                                             \
            x = expand(x);                                                       \
            for(uint64_t i = 0; i < rn; ++i){                                    \
                t = f(ref(xk[i]), ref(yk[i]));                                   \
                HANDLE_IF_ERROR;                                                 \
                rk[i] = t;                                                       \
            }                                                                    \
        }                                                                        \
        else {                                                                   \
            unref(r);                                                            \
            r = Kerr("type error! dyad operands have incorrect type");           \
        }                                                                        \
        unref(x), unref(y);                                                      \
        return r;                                                                \
    }                                                                            \
                                                                                 \
    /* case b: */                                                                \
    /* return type is the wider of the 2 operands */                             \
    int8_t rtype =  MAX(ABS(xt), ABS(yt));                                       \
    rtype = MIN(maxtype, rtype);                                                 \
    rtype = (0 < xt || 0 < yt) ? rtype : -rtype;                                 \
    uint64_t rcount = MAX(xn, yn);                                               \
    r = k(rtype, rcount);

// wrapper around DYADIC_OP_EXEC
// provides correct object accessor for the operation fepending on type
#define DYADIC_OP(op)                                                            \
    if      (KI == ABS(xt) && KI == ABS(yt)) { DYADIC_OP_EXEC(op, ri, xi, yi) }  \
    else if (KI == ABS(xt) && KF == ABS(yt)) { DYADIC_OP_EXEC(op, rf, xi, yf) }  \
    else if (KI == ABS(xt) && KC == ABS(yt)) { DYADIC_OP_EXEC(op, ri, xi, yc) }  \
    else if (KF == ABS(xt) && KI == ABS(yt)) { DYADIC_OP_EXEC(op, rf, xf, yi) }  \
    else if (KF == ABS(xt) && KF == ABS(yt)) { DYADIC_OP_EXEC(op, rf, xf, yf) }  \
    else if (KF == ABS(xt) && KC == ABS(yt)) { DYADIC_OP_EXEC(op, rf, xf, yc) }  \
    else if (KC == ABS(xt) && KI == ABS(yt)) { DYADIC_OP_EXEC(op, ri, xc, yi) }  \
    else if (KC == ABS(xt) && KF == ABS(yt)) { DYADIC_OP_EXEC(op, rf, xc, yf) }  \
    else if (KC == ABS(xt) && KC == ABS(yt)) { DYADIC_OP_EXEC(op, ri, xi, yc) }  \
    else { /* incompatible type. return error */                                 \
        unref(x),unref(y),unref(r);                                              \
        return Kerr("type error! dyad operand has incompatible type");           \
    }                                                                            \
    unref(x), unref(y);

// executes the dyadic operation
#define DYADIC_OP_EXEC(op, ra, xa, ya) /* dyadic op, z accessor, x accessor, y accessor */ \
    if      (xn == yn) for (uint64_t i = 0; i < xn; ++i) ra[i] = op( xa[i] , ya[i] ); \
    else if (xn  > yn) for (uint64_t i = 0; i < xn; ++i) ra[i] = op( xa[i] , ya[0] ); \
    else             { for (uint64_t i = 0; i < yn; ++i) ra[i] = op( xa[0] , ya[i] ); }


// dyadic verb table
// used for verb dispatch in the VM
//           +    *         -         %       .     !    |    &    <     >
D dyads[] = {add, multiply, subtract, divide, NULL, key, max, min, less, more};

K add(K x, K y){
    DYADIC_INIT(add, KF); // declare return object r, type rtype, count rcount
    DYADIC_OP(ADD); 
    return r;
}

K multiply(K x, K y){
    DYADIC_INIT(multiply, KF); // declare return object r, type rtype, count rcount
    DYADIC_OP(PRD); 
    return r;
}

K subtract(K x, K y){
    DYADIC_INIT(subtract, KF); // declare return object r, type rtype, count rcount
    DYADIC_OP(SUB); 
    return r;
}

K divide(K x, K y){
    // divide always returns F
    // we multiply one of the args by 1.0 if neither are F
    // we multiply x for simplicity
    // it could be a little smarter than this. eg cast, and cast the shorter/simpler arg
    if (KF != xt && KF != yt) x = multiply(Kf(1), x);

    DYADIC_INIT(divide, KF); // declare return object r, type rtype, count rcount
    DYADIC_OP(DIV); 
    return r;
}

K max(K x, K y){
    DYADIC_INIT(max, KF); // declare return object r, type rtype, count rcount
    DYADIC_OP(MAX); 
    return r;
}

K min(K x, K y){
    DYADIC_INIT(min, KF); // declare return object r, type rtype, count rcount
    DYADIC_OP(MIN); 
    return r;
}

K less(K x, K y){
    DYADIC_INIT(less, KI); // declare return object r, type rtype, count rcount
    DYADIC_OP(LESS); 
    return r;
}

K more(K x, K y){
    DYADIC_INIT(more, KI); // declare return object r, type rtype, count rcount
    DYADIC_OP(MORE); 
    return r;
}

K key(K x, K y){
    // `k!`v -> error
    if (0 > xt || 0 > yt){
        unref(x), unref(y);
        return Kerr("type error! can't key non-list args");
    }
    // `k`k1!1 2 3 -> error
    if (xn != yn){
        unref(x), unref(y);
        return Kerr("length error! can't key args of different lengths");
    }

    K r = k(KD, 2);
    rk[0] = x;
    rk[1] = y;

    return r;
}
