#include "parse.h"

#define AT_EXPR_END(c) sc(";)\n\0", (c))
#define HANDLE_ERROR(...) __extension__({if (!p->error){p->error=true; printf(__VA_ARGS__);}; ku(':');})

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
    //                       !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
    return (c > 126) ? 0 : " + ++++'()+++++'0000000000+ +++++                           ' ++                            + +"[c-32];
}

static K parseAdverb(Parser *p, K x){
    char c, t; //class, type
    char *s = ADVERB_STR;

    while ( '\'' == class(peek(p)) ){
        c = *p->current++;
        t = K_ADVERB_START + sc(s, c) - s;
        if (':'==peek(p)){ ++p->current; t+=3; }
        x = kw(t, x);
    }

    return x;
}

static i64 parseInt(char *s, i32 len){
    i64 n = 0;
    if ('-' == *s) return -parseInt(++s, len-1);
    for (i64 i=0; i<len; i++) n = (n*10) + (s[i]-'0'); 
    return n;
}

// parse number(s). "1","1 23"
static K parseNum(Parser *p){
    K r = tn(KI, 0);
    i64 n;
    char c, *s;

    // parse number or numeric list 
    do {
        // consume the number. s is where it starts 
        s = p->current;
        if ('-'==*p->current) ++p->current;
        while ('0'==class(*p->current)) ++p->current;

        // create a number and join it 
        n = parseInt(s, (i32)(p->current - s));
        r = j2(r, ki(n));

        // if not whitespace, then we're done parsing num literals
        if (' '!=*p->current) break;

        c = peek(p);
    } while ('0'==class(c) || ('-'==c && '0'==class(p->current[1])));

    return r;
}

// parse single expression
static K expr(Parser *p){
    K x, y;
    char a, c; //current char, char class
    
    a = next(p);
    c = class(a);

    // if '-' followed by digit, we're parsing a number, so reclassify
    if ('-'==a && '0'==class(peek(p))) c = '0';

    // parse classes
    switch (c){
    case '0': --p->current, x=parseNum(p); break;
    case '+': x=parseAdverb(p,kv(a)); return ( AT_EXPR_END(peek(p)) ) ? x : k2(x, expr(p));
    case '(': x=')'==peek(p)?tn(0,0):Exprs(',', p); if (')'==(a=next(p))){ break; } --p->current; unref(x); /*FALLTHROUGH*/ 
    default : return '\n'==a ? HANDLE_ERROR("'parse! unexpected EOL\n") : HANDLE_ERROR("'parse! unexpected token: %c\n", a);
    }

    if ( AT_EXPR_END(peek(p)) ) return x;

    if ( '\''==class(peek(p)) ){
        x = parseAdverb(p, x);
        return ( AT_EXPR_END(peek(p)) ) ? x : k2(x, expr(p));
    }
    
    a = next(p);
    if ('+' != class(a)){
        unref(x);
        return HANDLE_ERROR("'parse! expected dyadic op\n");
    }

    y = parseAdverb(p, kv(a));
    return k3(y, x, expr(p));
}

// parse ;-delimited Expressions
static K Exprs(char c, Parser *p){
    K r=tn(0,0), t;

    do r=jk(r, expr(p)); while(';'==next(p));
    --p->current;
    
    return 1==CNT(r) ? (t=ref(*OBJ(r)), unref(r), t) : j2(k1(ku(c)), r);
}

K parse(char *src){
    K r;

    // init parser struct
    Parser p;
    p.error = false;
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
