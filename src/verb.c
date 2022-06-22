#include "a.h"
#include "verb.h"
#include "adverb.h"
#include "util/mergesort.h"
#include "parse.h"

// dyadic arithmetic macros
#define ADD(x, y)   ((x) + (y))
#define PRD(x, y)   ((x) * (y))
#define SUB(x, y)   ((x) - (y))
#define DIV(x, y)   ((x) / (y))
// dyadic boolean macros
#define LESS(x, y)  ((x) < (y))
#define MORE(x, y)  ((x) > (y))
#define EQUAL(x, y) ((x) ==(y))

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
    if ((0 > xt || 0 > yt) && ((KK == xt && 0 == xn) || (KK == yt && 0 == yn))){ \
        unref(x), unref(y);                                                      \
        return k(KK, 0);                                                         \
     }                                                                           \
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
    else if (KD == xt || KD == yt){                                              \
        if (KD == xt && KD == yt) {                                              \
            unref(x), unref(y);                                                  \
            return Kerr("not yet implemented: dict op dict");                    \
        }                                                                        \
                                                                                 \
        if (KD == xt){                                                           \
            t = f(ref(xk[1]) , ref(y));                                          \
            if (KE == tt){                                                       \
                unref(x), unref(y);                                              \
                return t;                                                        \
            }                                                                    \
            r = key(ref(xk[0]), t);                                              \
        }                                                                        \
        else {                                                                   \
            t = f(ref(x) , ref(yk[1]));                                          \
            if (KE == tt){                                                       \
                unref(x), unref(y);                                              \
                return t;                                                        \
            }                                                                    \
            r = key(ref(yk[0]), t);                                              \
        }                                                                        \
        unref(x), unref(y);                                                      \
        return r;                                                                \
    }                                                                            \
    else if (KT == xt || KT == yt){                                              \
        if (KT == xt && KT == yt) {                                              \
            unref(x), unref(y);                                                  \
            return Kerr("not yet implemented: table op table");                  \
        }                                                                        \
                                                                                 \
        if (KT == xt){                                                           \
            t = f(ref(xk[0]) , ref(y));                                          \
            if (KE == tt){                                                       \
                unref(x), unref(y);                                              \
                return t;                                                        \
            }                                                                    \
            r = flip(t);                                                         \
        }                                                                        \
        else {                                                                   \
            t = f(ref(x) , ref(yk[0]));                                          \
            if (KE == tt){                                                       \
                unref(x), unref(y);                                              \
                return t;                                                        \
            }                                                                    \
            r = flip(t);                                                         \
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

#define DYADIC_OP(op)                                                            \
    if (KI == ABS(rt)){                                                          \
        DYADIC_OP_ACCESSORS(op, ri);                                             \
    }                                                                            \
    else if (KF == ABS(rt)) {                                                    \
        DYADIC_OP_ACCESSORS(op, rf);                                             \
    }                                                                            \
    else {                                                                       \
        unref(x), unref(y), unref(r);                                            \
        return Kerr("type error. return type incompatible");                     \
    }  

#define DYADIC_OP_RETURN_I(op) DYADIC_OP_ACCESSORS(op, ri)

#define DYADIC_OP_RETURN_F(op) DYADIC_OP_ACCESSORS(op, rf)

// wrapper around DYADIC_OP_EXEC
// provides correct object accessor for the operation fepending on type
#define DYADIC_OP_ACCESSORS(op, ra)                                              \
    if      (KI == ABS(xt) && KI == ABS(yt)) { DYADIC_OP_EXEC(op, ra, xi, yi) }  \
    else if (KI == ABS(xt) && KF == ABS(yt)) { DYADIC_OP_EXEC(op, ra, xi, yf) }  \
    else if (KF == ABS(xt) && KI == ABS(yt)) { DYADIC_OP_EXEC(op, ra, xf, yi) }  \
    else if (KF == ABS(xt) && KF == ABS(yt)) { DYADIC_OP_EXEC(op, ra, xf, yf) }  \
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
//           +    *         -         %       .         !    |    &    <     >     =      ~      ?     ,    @        #     _
V dyads[] = {add, multiply, subtract, divide, dotApply, key, max, min, less, more, equal, match, find, cat, atApply, take, drop};

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
    DYADIC_OP_RETURN_F(DIV); 
    return r;
}

K max(K x, K y){
    DYADIC_INIT(max, KF); // declare return object r, type rtype, count rcount
    DYADIC_OP_RETURN_I(MAX); 
    return r;
}

K min(K x, K y){
    DYADIC_INIT(min, KF); // declare return object r, type rtype, count rcount
    DYADIC_OP_RETURN_I(MIN); 
    return r;
}

K less(K x, K y){
    DYADIC_INIT(less, KI); // declare return object r, type rtype, count rcount
    DYADIC_OP_RETURN_I(LESS); 
    return r;
}

K more(K x, K y){
    DYADIC_INIT(more, KI); // declare return object r, type rtype, count rcount
    DYADIC_OP_RETURN_I(MORE); 
    return r;
}

K equal(K x, K y){
    DYADIC_INIT(equal, KI); // declare return object r, type rtype, count rcount
    DYADIC_OP_RETURN_I(EQUAL); 
    return r;
}

K match(K x, K y){
    // types must match
    if (xt != yt){
        unref(x), unref(y);
        return Ki(false);
    }

    // lengths must match
    if (xn != yn){
        unref(x), unref(y);
        return Ki(false);
    }

    // call match recursively on general lists
    if (KK == xt || KD == xt || KT == xt){
        K r = Ki(true);
        K t;
        for (uint64_t i = 0; i < xn; ++i){
            t = match( ref(xk[i]), ref(yk[i]) );
            ri[0] = ri[0] && ti[0];
            unref(t);
        }
        unref(x), unref(y);
        return r;
    }

    // check elements are equal
    bool equal = true;
    // 8-byte types
    if (KI == ABS(xt) || KS == ABS(xt) || KF == ABS(xt)){ 
        for (uint64_t i = 0; i < xn; ++i){
            if (xi[i] != yi[i]){
                equal = false;
                break;
            }
        }
    }
    else if (KC == ABS(xt)){
        for (uint64_t i = 0; i < xn; ++i){
            if (xc[i] != yc[i]){
                equal = false;
                break;
            }
        }
    }
    
    unref(x), unref(y);
    return Ki(equal);
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

// x?y
// returns vector of indices, the 1st occurence of each y in x
// if y not in x, returns count of x
// 1 3 5 7 ? 1 2 5 -> 0 4 2
K find(K x, K y){
    if (0 > xt){
        unref(x), unref(y);
        return Kerr("type error! x arg must be list");
    }

    int8_t type = 0 > yt ? -KI : KI;
    K r, t;
    bool equal;

    x = expand(x);
    y = expand(y);
    r = k(type, yn);
    
    // iterate y
    for (uint64_t i = 0; i < yn; ++i){
        ri[i] = xn;
        // iterate x
        for (uint64_t j = 0; j < xn; ++j){
            t = match(ref(xk[j]), ref(yk[i]));
            equal = ti[0];
            unref(t);
            if (equal){
                ri[i] = j;
                break;
            }
        }
    }

    unref(x), unref(y);
    return r;
}

// TODO : create new script covering amend (@ and .) functionality
// if joining 2 dicts, update on matching keys and insert new keys
K upsertDicts(K x, K y){
    // can only join dicts with sym keys
    if (KS != TYPE(DKEYS(x)) || KS != TYPE(DKEYS(y))){
        unref(x), unref(y);
        return Kerr("type error! can only join dicts with sym keys.");
    }
    
    K xkeys = ref(DKEYS(x));
    K xvals = expand(ref(DVALS(x)));
    K yvals = expand(ref(DVALS(y)));

    // iterate y dict and upsert one by one
    for (uint64_t i = 0; i < K_COUNT(y); ++i){
        // get ykey to upsert, and index to upsert at
        K ykey = Ks(INT(DKEYS(y))[i]);
        K idx = find(ref(DKEYS(x)), ref(ykey));

        // if new key, insert
        if (K_COUNT(x) == INT(idx)[0]){
            xkeys = cat(xkeys, ykey);
            xvals = cat(xvals, enlist(ref(KOBJ(yvals)[i])));
        }
        // else update existing
        else {
            unref(KOBJ(xvals)[INT(idx)[0]]);
            KOBJ(xvals)[INT(idx)[0]] = ref(KOBJ(yvals)[i]);
            unref(ykey);
        }
        unref(idx);
    }

    unref(x), unref(y), unref(yvals);
    return key(xkeys, squeeze(xvals));
}

// x,y
// 1,2 -> 1 2
K cat(K x, K y){
    // if joining 2 dicts, special upsert logic
    if (KD == xt && KD == yt){
        return upsertDicts(x, y);
    }

    x = expand(x);
    y = expand(y);

    K r = k(KK, xn+yn);
    for (uint64_t i = 0; i < xn; ++i) rk[i] = ref( xk[i] );
    for (uint64_t i = 0; i < yn; ++i) rk[i + xn] = ref( yk[i] );

    unref(x), unref(y);
    return squeeze(r);
}

// called by atApply when indexing with ints or syms
K atApplyIndex(K x, K y){
    // printf("DEBUG: ");debugPrintK(ref(x));debugPrintK(ref(y));
    if ((KK > xt || (KU <= xt && KP >= xt)) ||
         ((KK <= xt && KS >= xt) && (KK != yt && KI != ABS(yt) && KN != yt)) || 
          (KD == xt && KS != ABS(yt)) ||
          (KT == xt && (KS != ABS(yt) && KK != yt && KI != ABS(yt)))){
        unref(x), unref(y);
        return Kerr("type error! left operand can't be indexed with right operand.");
    }

    K r, t;

    // call recursively if y arg is general list
    if (KK == yt){
        r = k(KK, yn);
        for (uint64_t i = 0; i < yn; ++i){
            t = atApplyIndex(ref(x), ref(yk[i]));
            HANDLE_IF_ERROR;
            rk[i] = t;
        }
    }
    // index with (::) -> identity
    else if (KN == yt){
        return unref(y), x;
    }
    // index with int
    else if (KI == ABS(yt)){
        x = expand(x);
        r = k(KK, yn);
        for (uint64_t i = 0; i < yn; ++i)
            rk[i] = ( yi[i] >= (I)xn ) ? KNUL : ref(xk[ yi[i] ]);
    }
    // index dict/table with sym
    else {
        t =         find(ref( (KD == xt) ? DKEYS(x) : TKEYS(x) ), ref(y));
        r = atApplyIndex(ref( (KD == xt) ? DVALS(x) : TVALS(x) ), t);
    }
    r = ( 0 > yt && 1 == rn ) ? first(r) : squeeze(r);
    unref(x), unref(y);
    return r;
}

// x@y
// also called in OP_APPLY instruction (compiled for x y, synatactic sugar for x@y)
K atApply(K x, K y){
    // if a cast, function or projection
    if (-KS == xt || (KU <= xt && KP >= xt)){
        // (f@x) ~ (f .,x)
        return dotApply(x, enlist(y));
    }
    // else an atom or vector
    else {
        return atApplyIndex(x, y);
    }
}

K dotApply(K x, K y){
    K r, t;

   // dot apply only takes list as right arg
    if (yt < 0){
        unref(x), unref(y);
        return Kerr("rank error! dyadic . (dot apply) must have list as right operand.");
    }

    // casting / parse
    if (-KS == xt){
        switch ((char) xi[0]){
            // `p "x+y" -> returns parse tree 
            case 'p' : {
                // can't parse char atom (type is +ve because dotApply args are lists)
                if (KC == yt){
                    unref(x), unref(y);
                    return Kerr("rank error! can't parse char");
                }

                y = first(y);

                if (KC != ABS(yt)){
                    unref(x), unref(y);
                    return Kerr ("type error! can only parse strings.");
                }

                char *s = malloc(yn + 1);
                for (uint64_t i = 0; i < yn; ++i) s[i] = yc[i];
                s[yn] = '\0';
                unref(x), unref(y);
                r = parse(s);
                free(s);
                return r;
            }
            default : {
                unref(x), unref(y);
                return Kerr("error! sym application nyi.");
            }
        }
    }

    // index
    if (KK <= xt && KT >= xt){
        return over(Kv(KV, '@'), K_JOIN2(x,y));
    }

    // create adverb-modified object
    if (KA == xt){
        r = Ka(xc[0], first(y));
        unref(x);
        return r;
    }

    // apply adverb-modified object
    if (KOVER <= xt && KEACHPRIOR >= xt){
        if (2 < yn){
            unref(x), unref(y);
            return Kerr("rank error! derived verbs of rank >2 not yet implemented.");
        }
        if (KOVER <= TYPE(xk[0]) && KEACHPRIOR >= TYPE(xk[0])){
            unref(x), unref(y);
            return Kerr("error! nested adverbs not yet implemented.");
        }

        // get the adverb func pointer and check it exists (i.e. is implemented)
        V f = adverbs[xt - KOVER];
        if (NULL == f){
            unref(x), unref(y);
            return Kerr("error! adverb NYI.");
        }

        // grab the applicable value
        x = first(x);

        // if juxtaposed, add the @ (x/y converted to x@/y)
        if (KK <= xt && KT >= xt){
            t = x;
            x = Kv(KV, '@');
            y = cat(enlist(t), y);
        }
        
        r = (*f)(x, y);
        return r;
    }

    // monad
    if (KU == xt){
        if (1 != yn){
            unref(x), unref(y);
            return Kerr("rank error! arg count not correct (expected 1)");
        }
        U f = monads[ (uint8_t)xc[0] ];
        r = (*f)( ref(yk[0]) );
    }
    // dyad
    else if (KV == xt){
        if (2 < yn){
            unref(x), unref(y);
            return Kerr("rank error! arg count not correct (expected 2)");
        }
        
        y = expand(y);
        
        if (1 == yn){ // project
            r = Kp(Ki(1), NULL);
            t = k(KK, 2);
            tk[0] = first(y);
            tk[1] = k(-KN, 0);
            rk[1] = cat(x, t);
            return r;
        }

        V f = dyads[ (uint8_t)xc[0] ];
        r = (*f)(ref(yk[0]), ref(yk[1]));
    }
    // projection
    else if (KP == xt){
        uint64_t rank = INT(xk[0])[0];
        if (yn > rank){
            unref(x), unref(y);
            return Kerr("rank error! arg count not correct for projection");
        }
        else {
            y = expand(y);
            // func
            K f = ref( KOBJ(xk[1])[0] );
            // args
            t = k(KK, COUNT(xk[1]) - 1);
            for (uint64_t i = 0, j = 0; i < tn; ++i){
                tk[i] = (j<yn && -KN == TYPE(KOBJ(xk[1])[i+1])) ? ref(yk[j++]) : ref(KOBJ(xk[1])[i+1]);
            }
            r = (rank == yn) ? dotApply(f, t) : Kp( Ki(rank-yn), cat(f,t) );
        }
    }
    // either wrong type or not yet implemented
    else {
        unref(x), unref(y);
        return Kerr("error! applicable value type not recognised");
    }

    unref(x), unref(y);
    return r;
}

// x#y
K take(K x, K y){
    if (-KI != xt){
        unref(x), unref(y);
        return Kerr("type error! #(take) left operand must be type `i");
    }

    K r;
    if (KD == yt){
        K keys = take(ref(x), DKEYS(y));
        K vals = take(x, DVALS(y));
        r = key(keys, vals);
        free(y);
    }
    else {
        y = expand(y);
        r = k(KK, ABS(xi[0]));

        // if x is postive, take arguments from left to right
        if (0 < xi[0]){
            for (uint64_t i = 0; i < rn; ++i)
                rk[i] = ref(yk[i % yn]);
        }
        // else take args from right to left
        else {
            for (uint64_t i = 0, last = yn-1; i < rn; ++i)
                rk[i] = ref(yk[last - (i % yn)]);
        }
        unref(x), unref(y);
    }
    return squeeze(r);
}

// x_y
K drop(K x, K y){
    if (-KI != xt){
        unref(x), unref(y);
        return Kerr("type error! _(drop) left operand must be type `i");
    }

    K r;
    if (KD == yt){
        K keys = drop(ref(x), DKEYS(y));
        K vals = drop(x, DVALS(y));
        r = key(keys, vals);
        free(y);
    }
    else {
        y = expand(y);
        r = k(KK, MAX(0, (I)yn - ABS(xi[0])));

        // if x is positive, drop items from left to right. else, drop items from right to left
        uint64_t j = ( 0 < xi[0] ) ? xi[0] : 0;
        for (uint64_t i = 0; i < rn; ++i, ++j)
            rk[i] = ref(yk[j]);

        unref(x), unref(y);
    }
    return squeeze(r);
}

// monadic verb table
//            +     *      -       %           .     !          |        &      <    >     =     ~    ?     ,     @     #
U monads[] = {flip, first, negate, squareRoot, NULL, bangMonad, reverse, where, asc, desc, NULL, not, NULL, NULL, type, count};

static K flipDictOrTab(K x){
    K r;

    if (KT == xt){
        r = ref(xk[0]);
    }
    else {
        // can't flip dict if key isn't symbol
        if (KS != TYPE( DKEYS(x) )){
            unref(x);
            return Kerr("type error! dict key must be symbol");
        }

        // dict values must be lists
        if (KK != TYPE( DVALS(x) )){
            unref(x);
            return Kerr("rank error! dict value must be general list");
        }

        for (uint64_t i = 0; i < xn; ++i){
            if (0 > TYPE( KOBJ(DVALS(x))[i] )){
                unref(x);
                return Kerr("rank error! dict values must not be atoms");
            }
        }

        // dict values must have same count
        uint64_t n = COUNT( KOBJ(DVALS(x))[0] );
        for (uint64_t i = 1; i < xn; ++i){
            if (n != COUNT( KOBJ(DVALS(x))[i] )){
                unref(x);
                return Kerr("length error! dict values must be equal length");
            }
        }

        // return value
        r = k(KT, 1);
        rk[0] = ref(x);
    }

    unref(x);
    return r;
}

// +x
// +(1 1;2 3) -> (1 2;1 3)
K flip(K x){
    // must be a general list
    if(KK != xt && KD != xt && KT != xt){
        unref(x);
        return Kerr("rank error!");
    }

    // separate function to flip dicts and tables
    if (KD == xt || KT == xt){
        return flipDictOrTab(x);
    }

    // must be rectangular
    uint64_t n = COUNT( xk[0] );
    for (uint64_t i = 1; i < xn; ++i){
        if (n != COUNT( xk[i]) ){
            unref(x);
            return Kerr("length error!");
        }
    }

    // we operate on a temp variable
    K t = k(KK, xn);
    for (uint64_t i = 0; i < xn; ++i){
        // if any of the list elements are simple vectors, expand
        if (0 < TYPE( xk[i] )) 
            tk[i] = expand( ref(xk[i]) );
        // else element unchanged
        else {
            tk[i] = ref( xk[i] );
        }
    }

    // return value
    K r = k(KK, n);
    for (uint64_t i = 0; i < n; ++i) rk[i] = k(KK, xn);

    // flip
    for (uint64_t i = 0; i < xn; ++i){
        for (uint64_t j = 0; j < n; ++j)
            KOBJ( rk[j] )[i] = ref( KOBJ( tk[i] )[j] );
    }

    unref(x), unref(t);

    for (uint64_t i = 0; i < rn; ++i)
        rk[i] = squeeze(rk[i]);

    return r;
}

// *x
// *1 2 3 -> 1
K first(K x){
    K r;

    // return atoms and special types unchanged 
    if (0 > xt || KU == xt || KV == xt || KP == xt || KN == xt){
        return x;
    }
    // dictionary
    else if (KD == xt){
        r = first( ref(xk[1]) );
    }
    // table
    else if (KT == xt){
        x = expand(x);
        r = ref( xk[0] );
    }
    // adverb-modified object (f/ f' ...)
    else if (KOVER <= xt && KEACHPRIOR >= xt){
        r = ref(xk[0]);
    }
    // simple vector
    else if (0 < xt){
        r = k(-xt, 1);
        if      (KI == xt) ri[0] = xi[0];
        else if (KF == xt) rf[0] = xf[0];
        else if (KC == xt) rc[0] = xc[0];
        else if (KS == xt) ri[0] = xi[0];
        else    {unref(r); r = Kerr("type error!");}
    }
    // general list
    else {
        if (0 == xn){
            r = ref(x);
        }
        else {
            r = ref( xk[0] );
        }
    }
    unref(x);
    return r;
}

// -x
// - 1 2 3 -> -1 -2 -3
K negate(K x){
    if (KS == ABS(xt)){
        unref(x);
        return Kerr("type error!");
    }

    K r, t;

    if (KD == xt){
        t = negate(ref(xk[1]));
        r = (KE == tt) ? t : key(ref(xk[0]), t);
        unref(x);
        return r;
    }

    r = k(xt, xn);
    if      (KI == ABS(xt)) for (uint64_t i = 0; i < xn; ++i) ri[i] = -xi[i];
    else if (KF == ABS(xt)) for (uint64_t i = 0; i < xn; ++i) rf[i] = -xf[i];
    else if (KC ==     xt)  for (uint64_t i = 0; i < xn; ++i) rc[i] = -xc[i];
    else if (KT ==     xt)  rk[0] = negate( ref(xk[0]) );
    else if (KK ==     xt){
        for (uint64_t i = 0; i < xn; ++i){
            t = negate( ref(xk[i]) );
            if (KE == tt){
                while(i--) unref(rk[i]);
                free(r), unref(x);
                return t;
            }
            rk[i] = t;
        }
    }

    unref(x);
    return r;
}

// %x
// %4 2 -> 2 1.4142
K squareRoot(K x){
    K r, t;

    if (KK == xt){
        r = k(KK, xn);
        for (uint64_t i = 0; i < xn; ++i){
            t = squareRoot( ref(xk[i]) );
            if (KE == tt){
                while (i--) unref(rk[i]);
                free(r), unref(x);
                return t;
            }
            rk[i] = t;
        }
    }
    else if (KD == xt){
        t = squareRoot(ref(xk[1]));
        r = (KE == tt) ? t : key(ref(xk[0]), t);
        unref(x);
        return r;
    }
    else if (KT == xt){
        r = squareRoot( ref(xk[0]) );
    }
    else if (KI == ABS(xt) || KF == ABS(xt)){
        r = k(0 > xt ? -KF : KF, xn);
        x = multiply(Kf(1), x);
        for (uint64_t i = 0; i < xn; ++i) rf[i] = sqrt(xf[i]);
    }
    else {
        r = Kerr("type error! arg must be numeric type");
    }

    unref(x);
    return r;
}

// !x
// !3 -> 0 1 2
K enumerate(K x){
    if (-KI != xt){
        unref(x);
        return Kerr("type error! arg must be int atom");
    }

    K r = k(KI, xi[0]);
    for (uint64_t i = 0; i < rn; ++i) ri[i] = i;
    unref(x);
    return r;
}

// !x
// !`a`b!1 2 -> `a`b
K getKey(K x){
    if (KD != xt && KT != xt){
        unref(x);
        return Kerr("type error! can only get key of dict and table.");
    }

    K r = ( KD == xt ) ? ref(DKEYS(x)) : ref(TKEYS(x));
    unref(x);
    return r;
}

K bangMonad(K x){
    if (-KI == xt){
        return enumerate(x);
    }
    else if (KD == xt || KT == xt){
        return getKey(x);
    }
    else {
        unref(x);
        return Kerr("type error! invalid opernd for monadic ! (key/enumerate).");
    }
}

// |x
// |1 2 3 -> 3 2 1
K reverse(K x){
    K r;

    if (KD == xt){
        r = key(reverse(ref(xk[0])), reverse(ref(xk[1])));
        unref(x);
        return r;
    }

    x = expand(x);
    r = k(KK, xn);
    for (uint64_t i = 0, n = xn/2; i < n; ++i){
        rk[i] = ref(xk[xn - (i+1)]);
        rk[xn - (i+1)] = ref(xk[i]);
    }
    if (0 != xn%2) rk[xn/2] = ref(xk[xn/2]);
    r = squeeze(r);
    unref(x);
    return r;
}

// ,x
// ,1 -> ,1
K enlist(K x){
    K r = k(KK, 1);
    rk[0] = x;
    return squeeze(r);
}

K where(K x){
    if (KI != xt){
        unref(x);
        return Kerr("type error! &(where) arg must be int list");
    }

    // get return count
    uint64_t n = 0;
    for (uint64_t i = 0; i < xn; ++i)
        if (0 < xi[i]) n += xi[i];

    K r = k(KI, n);
    uint64_t j = 0, h;
    for (uint64_t i = 0; i < xn; ++i){
        if (0 < xi[i]){
            h = j + xi[i];
            while (j < h){
                ri[j] = i;
                ++j;
            }
        }
    }

    unref(x);
    return r;
}

static K grade(K x, bool asc){
    if (KI != xt){
        unref(x);
        return Kerr("rank error! can only sort int vectors");
    }

    K r = enumerate(Ki(xn));
    K t = k(KI, xn);
    mergeSortIndex(x, r, t, 0, xn-1, asc);

    unref(x), unref(t);
    return r;
}

// <x (return sorted indices)
// <2 1 7 -> 1 0 2
K asc(K x){
    return grade(x, true);
}

// >x (return sorted indices)
// >2 1 7 -> 2 0 1
K desc(K x){
    return grade(x, false);
}

// =x not yet implemented

// ~x
// ~1 0 1 -> 0 1 0
K not(K x){
    K r, t;
    if (KK == xt){
        r = k(KK, xn);
        for (uint64_t i = 0; i < xn; ++i){
            t = not(ref(xk[i]));
            if (KE == tt){
                while (i--) unref(rk[i]);
                unref(x), free(r);
                return t;
            }
            rk[i] = t;
        }
    }
    else if (KD == xt){
        t = not( ref(DVALS(x)) );
        if (KE == xt){
            unref(x);
            return t;
        }
        r = key(DKEYS(x), t);
    }
    else if (KT == xt){
        t = not( ref(xk[0]) );
        if (KE == tt){
            unref(x);
            return t;
        }
        r = k(KT, 1);
        rk[0] = t;
    }
    else if (KI == ABS(xt) || KS == ABS(xt)){
        r = k(KI * SGN(xt), xn);
        for (uint64_t i = 0; i < xn; ++i) ri[i] = (0 == xi[i]);
    }
    else if (KC == ABS(xt)){
        r = k(KI * SGN(xt), xn);
        for (uint64_t i = 0; i < xn; ++i) ri[i] = (0 == xc[i]);
    }
    else if (KF == ABS(xt)){
        r = k(KI * SGN(xt), xn);
        for (uint64_t i = 0; i < xn; ++i) ri[i] = (0.0 == xf[i]);
    }
    else {
        r = Kerr("type error! type not handled by ~");
    }

    unref(x);
    return squeeze(r);
}

K type(K x){
    K r = Ks( (I) 0 > xt ? KATOMTYPES[ABS(xt)] : KLISTTYPES[xt] );
    unref(x);
    return r;
}

K count(K x){
    K r = Ki( K_COUNT(x) );
    unref(x);
    return r;
}
