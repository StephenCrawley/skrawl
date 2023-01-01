#include "parse.h"

#define AT_EXPR_END(c) sc(";)]\n\0", (c))
#define HANDLE_ERROR(...) __extension__({if (!p->error){p->error=true; printf(__VA_ARGS__);}; ku(':');})
#define COMPOSE(x,y) __extension__({K _x=(x),_y=(y); k3(kw(0),_x,_y) ;})

// foward declarations
static K Exprs(char c, Parser *p);

// consume whitepace
static inline void ws(Parser *p){ while(' '==*p->current) ++p->current; }

// next char. consume whitespace and consume & return next char
static inline char next(Parser *p){ ws(p); return *p->current++; }

// peek char. consume whitespace and peek next char (return but don't consume it)
static inline char peek(Parser *p){ ws(p); return *p->current; }

// return char class 
static char class(char c){
    //                       ! "#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
    return (c > 126) ? 0 : " +\"++++'()+++++'0000000000+ +++++aaaaaaaaaaaaaaaaaaaaaaaaaa ' ++`aaaaaaaaaaaaaaaaaaaaaaaaaa + +"[c-32];
}

// parse x'  
static K parseAdverb(Parser *p, K x){
    char c, t; //class, type
    char *s = ADVERB_STR;

    while ( '\'' == class(peek(p)) ){
        c = *p->current++;
        t = K_ADVERB_START + sc(s, c) - s;
        if (':'==peek(p)){ ++p->current; t+=3; }
        x = kwx(t, x);
    }

    return x;
}

// parse x[y;z]
static K parseMExpr(Parser *p, K x){
    K r;
    char c;
    while ('[' == peek(p)){
        ++p->current;
        r = ']'==peek(p) ? k1(ku(':')) : Exprs(0, p);
        if (']' != (c=next(p))){
            unref(x), unref(r);
            return HANDLE_ERROR("'parse! unexpected token: %c", c);
        }

        // parse +[1] as (+:;1) and +[1;2] as (+;1;2)
        if (IS_VERB(x) && CNT(r)>1) TYP(x)=KV; 

        x = j2(k1(x), r);
    }
    return x;
}

// parse adverbs and m-expressions
static K parsePostfix(Parser *p, K x){
    x = parseMExpr(p, x);
    x = parseAdverb(p, x);
    if ('['==peek(p)) x=parsePostfix(p, x);
    return x;
}

static K parseStr(Parser *p){
    K r;
    char a, *s = p->current; //current char, start of string after first "
    u64 n = 0; //count of chars in string
    while ('"' != (a=next(p))){
        ++n; 
        if (!a){ //if EOL
            --p->current;
            return HANDLE_ERROR("'parse! unclosed string\n");
        }
    }
    r = tn(1==n ? -KC : KC, n);
    memcpy(CHR(r), s, n);
    return r;
}

static i64 parseInt(char *s, i32 len){
    i64 n = 0;
    if ('-' == *s) return -parseInt(++s, len-1);
    for (i64 i=0; i<len; i++) n = (n*10) + (s[i]-'0'); 
    return n;
}

static double parseFlt(char *s, i32 len, i32 d){
    double f;
    i64 l, r; //left of dot, right of dot
    if ('-' == *s) return -parseFlt(++s, len-1, d-1);
    l = d<len   ? parseInt(s, d) : 0;
    r = d<len-1 ? parseInt(s+d+1, len-d-1) : 0;
    f = (double) r;
    for (i32 i=0, n=len-d-1; i<n; i++) f/=10.0;
    return (double)l + f;
}

// parse number(s). "1","1 23"
static K parseNum(Parser *p){
    K r = tn(KI, 0);
    char c, *s, f, *d; //c:current char, s:start of num, f:count of dots seen, d:location of dot
    i8 t;  //return type
    i64 n; //int placeholder

    // parse number or numeric list 
    do {
        f = 0; 
        // consume the number. s is where it starts 
        s = p->current, d = s;
        c = *s;
        if ('-'==c) c = *++p->current;
        while ('0'==class(c) || c=='.'){
            if (c=='.') { f += 1; d = p->current; }
            c = *++p->current;
        }

        // if more than one dot, error
        if (f > 1)
            return unref(r), HANDLE_ERROR("'parse! invalid number\n");

        // create a number and join it 
        t = ABS(TYP(r));
        if (f){
            // if float parsed after ints, cast the return object to float
            if (KI==t && CNT(r)){ 
                for (i32 i=0; i<CNT(r); i++) FLT(r)[i] = (double) INT(r)[i];
                TYP(r) = KF;
            }
            r = j2(r, kf(parseFlt(s, p->current-s, d-s)));
        }
        else {
            n = parseInt(s, p->current-s);
            r = j2(r, KI==t ? ki(n) : kf( (double)n ));
        }

        // if not whitespace, then we're done parsing num literals
        if (' '!=*p->current) break;

        c = peek(p);
    } while ('0'==class(c) || ('-'==c && '0'==class(p->current[1])));

    return r;
}

static i64 encodeSym(Parser *p){
    i8 i = 0;
    i64 n = 0;
    char a = *p->current;
    char c = class(a);

    while ('a'==c || '0'==c){
        // encode char in i64. max at chars per symbol
        if (i<8){
            CHR(&n)[i++] = a;
        }
        a = *++p->current;
        c = class(a);
    }

    return n;
}

static K parseSym(Parser *p){
    K r = tn(KS, 0);
    char c;

    do {
        ++p->current;
        r = j2(r, ks(encodeSym(p)));
        c = peek(p);
    } while('`'==c);

    return r;
}

static K classSwitch(Parser *p, char a, char c){
    K x;

    switch (c){
    case '0': --p->current, x=parseNum(p); break;
    case '`': --p->current, x=parseSym(p); break;
    case '"': x=parseStr(p); break;
    case '+': c=peek(p),x=AT_EXPR_END(c)?kv(a):'\''!=class(c)?ku(a):'\''==c?ku(a):kv(a); break;
    case '(': x=')'==peek(p)?tn(0,0):Exprs(',',p); if (')'==(a=next(p))){ break; } --p->current; unref(x); /*FALLTHROUGH*/ 
    default : return '\n'==a ? HANDLE_ERROR("'parse! unexpected EOL\n") : HANDLE_ERROR("'parse! unexpected token: %c\n", a);
    }

    return parsePostfix(p, x);
}

// parse single expression
static K expr(Parser *p){
    K x, y, z; //prefix, infix, right expression
    char a, c; //current char, char class
    
    a = next(p);
    c = class(a);

    // if '-' followed by digit, we're parsing a number, so reclassify
    if ('-'==a && '0'==class(*p->current)) c = '0';

    // parse based on class of current character
    x = classSwitch(p, a, c);
    if (p->error) return x;

    bool va = IS_VERB(x) || IS_ADVERB(x);

    // return x and set composition flag
    if ( AT_EXPR_END(peek(p)) ) 
        return p->compose |= va, x;

    // parse +x
    if (va) 
        return y = expr(p), p->compose ? COMPOSE(x, y) : k2(x, y);
    
    a = next(p);
    c = class(a);
    if ('+'==c && '['!=peek(p)){ //infix verb
        y = parsePostfix(p, kv(a));
    }
    else { //infix noun
        // need to handle 2 cases. x y z->(x;(y;z)) and x y/z->((/;y);x;z)
        char *t = p->current-1; //temp to reset if infix noun is not followed by adverb

        y = classSwitch(p, a, c);
        if (p->error) return unref(x), y;

        if ( !IS_ADVERB(y) ){
            if ( AT_EXPR_END(peek(p)) ){
                return k2(x, y);
            }
            else {
                unref(y);
                p->current = t;
                return y = expr(p), p->compose ? COMPOSE(x, y) : k2(x, y);
            }
        }
    }

    // parse x+
    if ( AT_EXPR_END(peek(p)) ) 
        return p->compose = true, k2(y, x);
    
    // parse x+y
    z = expr(p);
    return p->compose ? COMPOSE(k2(y, x), z) : k3(y, x, z);
}

// parse ;-delimited Expressions
static K Exprs(char c, Parser *p){
    K r=tn(0,0), t;

    do r=jk(r, expr(p)), p->compose=false; while(';'==next(p));
    --p->current;
    
    return !c ? r : 1==CNT(r) ? (t=ref(*OBJ(r)), unref(r), t) : j2(k1(ku(c)), r);
}

K parse(char *src){
    K r;

    // init parser struct
    Parser p;
    p.error = false;
    p.compose = false;
    p.src = src;
    p.current = src;

    // return if only whitespace in input
    if (sc("\n\0", peek(&p))){
        return (K)0;
    }

    // parse Expressions
    r = Exprs(';', &p);
    return p.error ? unref(r),(K)0 : r;
}
