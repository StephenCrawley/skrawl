#include "a.h"
#include "token.h"
#include "object.h"

K ref(K x){
    // if generic K type, call 'ref' on all child K objects
    if (KK == xt){
        for (uint64_t i = 0; i < xn; ++i) ref(xk[i]);
    }

    return xr++, x;
}

void unref(K x){
    // if generic K type, call 'unref' on all child K objects
    if (KK == xt){
        for (uint64_t i = 0; i < xn; ++i) unref(xk[i]);
    }

    // decrement refcount. if ref reaches 0, free the object
    xr--;
    if (1 > xr) free(x);
}

// malloc K object
static K ma(size_t size, uint64_t count){
    K r = malloc((size*count) + offsetof(struct k, d));
    rn = count;
    rr = 1;
    return r;
}

// malloc and return an uninitialized K object 
K k(int8_t type, uint64_t count){
    // KWIDTHS is a #define in a.h, a list of numbers corresponding to the width of each type
    // the K object types are used to index into this list
    uint8_t widths[] = {KWIDTHS};
    uint8_t size = widths[ ABS(type) ];
    K r = ma(size, count);
    rt = type;
    return r;
}

K Ki(int64_t x){
    K r = k(-KI, 1);
    ri[0] = x;
    return r;
}

K Kf(double x){
    K r = k(-KF, 1);
    rf[0] = x;
    return r;
}

K Kc(char x){
    K r = k(-KC, 1);
    rc[0] = x;
    return r;
}

K Kerr(const char *error){
    uint64_t n = strlen(error);
    K r = ma(sizeof(C), n);
    rt = KE;
    for (uint64_t i = 0; i < n; ++i) rc[i] = error[i];
    return r;
}

// squeeze a general K list into a more compact simple list if possible
// eg (1;2;3) -> 1 2 3 or ("a";"b";"c") -> "abc" 
K squeeze(K x){
    if (KK != xt) return x; // can only squeeze general lists

    int8_t type = TYPE( xk[0] );
    if (0 <= type) return x; // can only squeeze general list of atoms

    for (uint64_t i = 1; i < xn; ++i){
        if (type != TYPE( xk[i] )) return x; // can't squeeze if types differ
    }

    K r = k(ABS(type), xn);
    if (KI == rt)
        for (uint64_t i = 0; i < xn; ++i){
            ri[i] = INT( xk[i] )[0];
        }
    else if (KF == rt)
        for (uint64_t i = 0; i < xn; ++i){
            rf[i] = FLOAT( xk[i] )[0];
        }
    else if (KC == rt)
        for (uint64_t i = 0; i < xn; ++i){
            rc[i] = CHAR( xk[i] )[0];
        }
    else {printf("squeeze type NYI. exit...\n"); exit(1);}

    unref(x);
    return r;
}

// expand a simple K list to a general list 
// eg 1 2 3 -> (1;2;3) or "abc" -> ("a";"b";"c")
K expand(K x){
    K r = k(KK, xn);
    if      (KI == xt) for (uint64_t i = 0; i < rn; ++i) rk[i] = Ki( xi[i] );
    else if (KF == xt) for (uint64_t i = 0; i < rn; ++i) rk[i] = Kf( xf[i] );
    else if (KC == xt) for (uint64_t i = 0; i < rn; ++i) rk[i] = Kc( xc[i] );
    else {unref(x), unref(r); return Kerr("type error! can't expand");}
    return r;
}

// print functionality
// pX are private helpers to print various types
// printK is public print function

// print int object
static void pI(K x){
    if (xn){
        if (0 < xt && 1 == xn) putchar(',');
        for (uint64_t i = 0, last = xn-1; i < xn; ++i){
            printf("%ld", xi[i]);
            if (i != last) putchar(' ');
        }
    }
    else {
        printf("0#0");
    }
}

// print float object
// truncates trailing 0s
static void pF(K x){
    if (xn){ // if not empty 0-count object
        if (0 < xt && 1 == xn) putchar(',');
        char buff[9]; // TODO : fix. major bug
        for (uint64_t i = 0, last = xn-1; i < xn; ++i) {
            int r = snprintf(buff, 9, "%f.6", xf[i]);
            if (0 > r){ // if snprintf failed
                printf("'print float");
                return;
            }
            // remove trailing 0s
            uint8_t p = 8;
            while ('0' == buff[p-1]) --p;
            // print
            printf("%.*s", p, buff);
            if (i != last) putchar(' ');
        }
    }
    else {
        printf("0#0.");
    }
}

// print char object
static void pC(K x){
    if (0 < xt && 1 == xn) putchar(',');
    putchar('"');
    for (uint64_t i = 0; i < xn; ++i) putchar( xc[i] );
    putchar('"');
}

// print op (monad / dyad / adverb)
// these objects contain a single char which indexes the KOPS string
// this index is also used to access the function pointer for the op
static void pO(K x){
    static const char ops[] = KOPS;
    char op = ops[ (int8_t) *xc ];
    putchar(op);
    // if monad or digraph (/: \: ':) put : to stdout
    if (KM == xt || TOKEN_EACHL == xc[0] || TOKEN_EACHR == xc[0] || TOKEN_EACHPRIOR == xc[0]) 
        putchar(':');
}

// print error
static void pE(K x){
    putchar('\'');
    for (uint64_t i = 0; i < xn; ++i) putchar( xc[i] );
}

// prints a K object on one line
// used to print child K objects in a general K list
void printOneLineK(K x){
    if (KK == xt){
        putchar('(');
        for (uint64_t i = 0, last = xn-1; i < xn; ++i){
            printOneLineK(xk[i]);
            if (i != last) putchar(';');
        }
        putchar(')');
    }
    else if (KN ==     xt)  printf("::");
    else if (KC == ABS(xt)) pC(x);
    else if (KI == ABS(xt)) pI(x);
    else if (KF == ABS(xt)) pF(x);
    else if (KM ==     xt)  pO(x);
    else if (KD ==     xt)  pO(x);
    else if (KA ==     xt)  pO(x);
    else if (KE ==     xt)  pE(x);
    else {printf("can't print type: %d",xt);}
}

void printK(K x){
    // if general K object, print each element on its own line
    if (KK == xt){
        for (uint64_t i = 0, last = xn-1; i < xn; ++i){
            printOneLineK(xk[i]);
            if (i != last) putchar('\n');
        }
    }
    else if (KN ==     xt) return; 
    else if (KC == ABS(xt)) pC(x);
    else if (KI == ABS(xt)) pI(x);
    else if (KF == ABS(xt)) pF(x);
    else if (KM ==     xt)  pO(x);
    else if (KD ==     xt)  pO(x);
    else if (KA ==     xt)  pO(x);
    else if (KE ==     xt)  pE(x);
    else {printf("can't print type: %d",xt);}
    putchar('\n');
    unref(x);
}

void debugPrintK(K x){
    printf("len: %ld, type: %d\n", xn, xt);
    printK(x);
    putchar('\n');
}