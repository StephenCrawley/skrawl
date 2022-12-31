#include "parse.h"

#define AT_EXPR_END(c) sc(";)\n\0", (c))
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
    return (c > 126) ? 0 : " +\"++++'()+++++'0000000000+ +++++                           ' ++                            + +"[c-32];
}

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

static K classSwitch(Parser *p, char a, char c){
    K x;

    switch (c){
    case '0': --p->current, x=parseNum(p); break;
    case '"': x=parseStr(p); break;
    case '+': c=peek(p),x=AT_EXPR_END(c)?kv(a):'\''!=class(c)?ku(a):'\''==c?ku(a):kv(a); break;
    case '(': x=')'==peek(p)?tn(0,0):Exprs(',', p); if (')'==(a=next(p))){ break; } --p->current; unref(x); /*FALLTHROUGH*/ 
    default : return '\n'==a ? HANDLE_ERROR("'parse! unexpected EOL\n") : HANDLE_ERROR("'parse! unexpected token: %c\n", a);
    }

    return parseAdverb(p, x);
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
        return y=expr(p), p->compose ? COMPOSE(x, y) : k2(x, y);
    
    a = next(p);
    c = class(a);
    if ('+' == c){ //infix verb
        y = parseAdverb(p, kv(a));
    }
    else { //infix noun
        char *t = p->current; //temp to reset if infix noun is not followed by adverb

        y = classSwitch(p, a, c);
        if (p->error) return y;

        if ( !IS_ADVERB(y) ){
            if ( AT_EXPR_END(peek(p)) ){
                return k2(x, y);
            }
            else {
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
    
    return 1==CNT(r) ? (t=ref(*OBJ(r)), unref(r), t) : j2(k1(ku(c)), r);
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
