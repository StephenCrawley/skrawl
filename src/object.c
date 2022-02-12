#include "a.h"
#include "token.h"
#include "object.h"
#include "verb.h"

K ref(K x){
    // if generic K type, call 'ref' on all child K objects
    if (KK == xt || KD ==xt){
        for (uint64_t i = 0; i < xn; ++i) ref(xk[i]);
    }

    return xr++, x;
}

void unref(K x){
    // if generic K type, call 'unref' on all child K objects
    if (KK == xt || KD == xt || KT == xt){
        for (uint64_t i = 0; i < xn; ++i) unref(xk[i]);
    }

    // decrement refcount. if ref reaches 0, free the object
    xr--;
    if (1 > xr) free(x);
}

// returns the count seen by the user with monadic #
// when x is a table, returns the count of rows 
uint64_t count(K x){
    return (KT == xt) ? COUNT( KOBJ( KOBJ( xk[0] )[1])[0] ) : xn;
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

K Ks(int64_t x){
    K r = k(-KS, 1);
    ri[0] = x;
    return r;
}

K Kerr(const char *error){
    uint64_t n = strlen(error);
    K r = ma(sizeof(C), n);
    rt = KE;
    for (uint64_t i = 0; i < n; ++i) rc[i] = error[i];
    return r;
}

// function to squeeze a general list of dictionaries into a table
static K squeezeDicts(K x){
    // first check all dict keys are symbols
    for (uint64_t i = 0; i < xn; ++i)
        if(KS != TYPE( KOBJ(xk[i])[0] )) return x;

    // check all dict keys are the same
    K key = KOBJ( xk[0] )[0];
    K tmp;
    for (uint64_t i = 1; i < xn; ++i){
        tmp = KOBJ( xk[i] )[0];

        // check keys have same count. if not, return x
        if (COUNT(key) != COUNT(tmp)) return x;

        // check keys are equal. if not, return x
        for (uint8_t j = 0; j < COUNT(key); ++j)
            if (INT(key)[j] != INT(tmp)[j]) return x;
    }

    // create column data
    K t = k(KK, xn);
    for (uint64_t i = 0; i < xn; ++i) tk[i] = ref( KOBJ(xk[i])[1] );
    K cols = flip(t);

    // create table
    // a table object contains a single element: a dict object
    K dict = k(KD, 2);
    KOBJ(dict)[0] = ref(key);
    KOBJ(dict)[1] = cols;
    K r = k(KT, 1);
    rk[0] = dict;

    unref(x);
    return r;
}

// squeeze a general K list into a more compact simple list if possible
// eg (1;2;3) -> 1 2 3 or ("a";"b";"c") -> "abc" 
K squeeze(K x){
    // can only squeeze general lists
    if (KK != xt) return x;

    int8_t type = TYPE( xk[0] );

    // if type is dictionary, call squeezeDicts and return result
    if (KD == type) return squeezeDicts(x);

    // can only squeeze general list of atoms
    if (0 <= type) return x; 

    // can't squeeze if types differ
    for (uint64_t i = 1; i < xn; ++i) 
        if (type != TYPE( xk[i] )) return x; 

    // squeeze
    K r = k(ABS(type), xn);
    if      (KI == rt) for (uint64_t i = 0; i < xn; ++i) ri[i] = INT  ( xk[i] )[0];
    else if (KF == rt) for (uint64_t i = 0; i < xn; ++i) rf[i] = FLOAT( xk[i] )[0];
    else if (KC == rt) for (uint64_t i = 0; i < xn; ++i) rc[i] = CHAR ( xk[i] )[0];
    else if (KS == rt) for (uint64_t i = 0; i < xn; ++i) ri[i] = INT  ( xk[i] )[0];
    else {printf("squeeze type NYI. exit...\n"); exit(1);}
    unref(x);
    return r;
}

// expand a simple K list to a general list 
// eg 1 2 3 -> (1;2;3) or "abc" -> ("a";"b";"c")
K expand(K x){
    if (KK == xt) return x;
    uint64_t n = (KD == xt) ? 1 : count(x);
    K r = k(KK, n);
    if      (KI == ABS(xt)) for (uint64_t i = 0; i < rn; ++i) rk[i] = Ki( xi[i] );
    else if (KF == ABS(xt)) for (uint64_t i = 0; i < rn; ++i) rk[i] = Kf( xf[i] );
    else if (KC == ABS(xt)) for (uint64_t i = 0; i < rn; ++i) rk[i] = Kc( xc[i] );
    else if (KS == ABS(xt)) for (uint64_t i = 0; i < rn; ++i) rk[i] = Ks( xi[i] );
    else if (KD == ABS(xt)) rk[0] = ref(x);
    else if (KT == ABS(xt)) {
        K keys = KOBJ(xk[0])[0];
        K vals = KOBJ(xk[0])[1];
        vals = flip(ref(vals));
        for (uint64_t i = 0; i < COUNT(vals); ++i) rk[i] = key(ref(keys) , ref(KOBJ(vals)[i]));
        unref(vals);
    }
    else {
        unref(x), unref(r);
        return Kerr("type error! can't expand");
    }
    unref(x);
    return r;
}

// print functionality
// pX are private helpers to print various types
// printK is public print function

// print int object
static void printI(K x){
    if (0 == xn){
        printf("0#0");
        return;
    }

    if (0 < xt && 1 == xn) putchar(',');
    for (uint64_t i = 0, last = xn-1; i < xn; ++i){
        printf("%ld", xi[i]);
        if (i != last) putchar(' ');
    }
}

// print float object
// truncates trailing 0s
static void printF(K x){
    if (0 == xn){
        printf("0#0.");
        return;
    }

    if (0 < xt && 1 == xn) putchar(',');
    char buf[9]; // TODO : fix. major bug
    for (uint64_t i = 0, last = xn-1; i < xn; ++i) {
        int r = snprintf(buf, 9, "%f.6", xf[i]);
        if (0 > r){ // if snprintf failed
            printf("'print float");
            return;
        }
        // remove trailing 0s
        uint8_t p = 8;
        while ('0' == buf[p-1]) --p;
        // print
        printf("%.*s", p, buf);
        if (i != last) putchar(' ');
    }
}

// print char object
static void printC(K x){
    if (0 == xn){
        printf("\"\"");
        return;
    }

    if (0 < xt && 1 == xn) putchar(',');
    putchar('"');
    for (uint64_t i = 0; i < xn; ++i) putchar( xc[i] );
    putchar('"');
}

// print symbol object
static void printS(K x){
    if (0 == xn){
        printf("0#`");
        return;
    }

    if (0 < xt && 1 == xn) putchar(',');
    for (uint64_t i = 0; i < xn; ++i){
        putchar('`');

        // symbols are contained in 8-byte slots
        // we access each symbol with xi (int64_t pointer) 
        // then cast this to char pointer so we can access and print each byte
        char *ptr = (char *)(xi + i);

        // symbols less than 8 chars are '\0' padded.
        // print while not null char AND within the 8 bytes
        uint8_t j = 0;
        while (ptr[j] && j != 8){
            putchar(ptr[j++]);
        }
    }
}

// print dictionary
static void printD(K x){
    printOneLineK(xk[0]);
    putchar('!');
    printOneLineK(xk[1]);
}

// print table
static void printT(K x){
    putchar('+');
    printD(xk[0]);
}

// print op (monad / dyad / adverb)
// these objects contain a single char which indexes the KOPS string
// this index is also used to access the function pointer for the op
static void printO(K x){
    static const char ops[] = KOPS;
    char op = ops[ (int8_t) *xc ];
    putchar(op);
    // if monad or digraph (/: \: ':) put : to stdout
    if (KU == xt || TOKEN_EACHL == xc[0] || TOKEN_EACHR == xc[0] || TOKEN_EACHPRIOR == xc[0]) 
        putchar(':');
}

// print error
static void printE(K x){
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
    else if (KC == ABS(xt)) printC(x);
    else if (KI == ABS(xt)) printI(x);
    else if (KF == ABS(xt)) printF(x);
    else if (KS == ABS(xt)) printS(x);
    else if (KD ==     xt)  printD(x);
    else if (KT ==     xt)  printT(x);
    else if (KU ==     xt)  printO(x);
    else if (KV ==     xt)  printO(x);
    else if (KA ==     xt)  printO(x);
    else if (KE ==     xt)  printE(x);
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
    else if (KN ==     xt)  {unref(x);return;}
    else if (KC == ABS(xt)) printC(x);
    else if (KI == ABS(xt)) printI(x);
    else if (KF == ABS(xt)) printF(x);
    else if (KS == ABS(xt)) printS(x);
    else if (KD ==     xt)  printD(x);
    else if (KT ==     xt)  printT(x);
    else if (KU ==     xt)  printO(x);
    else if (KV ==     xt)  printO(x);
    else if (KA ==     xt)  printO(x);
    else if (KE ==     xt)  printE(x);
    else {printf("can't print type: %d",xt);}
    putchar('\n');
    unref(x);
}

void debugPrintK(K x){
    printf("len: %ld, type: %d\n", xn, xt);
    printK(x);
    putchar('\n');
}
