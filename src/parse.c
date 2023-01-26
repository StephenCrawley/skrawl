#include "parse.h"

#define AT_EXPR_END(c) sc(";)]}\n\0", (c))
#define COMPOSE(x,y) __extension__({K _x=(x),_y=(y); k3(kw(0),_x,_y) ;})
#define HEAD_IS_ADVERB(x) ( KK==TYP(x) && KW==TYP(*OBJ(x)) )
#define IS_K_SET(x) (TAG_TYP(x) ? 0==TAG_VAL(x) : 0)
#define PRINT_ERROR(...) (puts(p->src),printf("%*s^\n",(int)(p->current-p->src),""),printf("'parse! "), printf(__VA_ARGS__))
#define HANDLE_ERROR(...) __extension__({if (!p->error){p->error=true; PRINT_ERROR(__VA_ARGS__);} ku(':');})

// foward declarations
static K Exprs(char c, Parser *p);

// increment current char pointer
static inline Parser *inc(Parser *p){ return ++p->current, p; }
// decrement current char pointer
static inline Parser *dec(Parser *p){ return --p->current, p; }

// consume whitepace
static inline void ws(Parser *p){ while(' '==*p->current) ++p->current; }

// peek char. consume whitespace and peek next char (return but don't consume it)
static inline char peek(Parser *p){ ws(p); char a=*p->current; return '/'==a&&' '==p->current[-1] ? 0 : a; }

// next char. consume whitespace and consume & return next char
static inline char next(Parser *p){ char a=peek(p); return ++p->current, a; }

// return char class 
static char class(char c){
    //                           ! "#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
    return (32>c||c>126) ? 0 : " +\"++++/()+++++/0000000000+ +++++aaaaaaaaaaaaaaaaaaaaaaaaaa / ++`aaaaaaaaaaaaaaaaaaaaaaaaaa{+}+"[c-32];
}

// parse x'  
static K parseAdverb(Parser *p, K x){
    char t; //type
    while ( '/' == class(peek(p)) ){
        t = ic(ADVERB_STR, *p->current++);
        if (':'==*p->current){ ++p->current; t+=3; }
        // special case: if x==0 we're parsing bare adverbs (eg "'/")
        x = x ? k2(kw(t), x) : kw(t);
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
            return HANDLE_ERROR("unexpected token: %c", c);
        }

        // parse +[1] as (+:;1) and +[1;2] as (+;1;2)
        if (KV==TYP(x) && 1==CNT(r)) x = tx(KU,x); 

        x = j2(k1(x), r);
    }
    return x;
}

// parse adverbs and m-expressions
static K parsePostfix(Parser *p, K x){
    // parse m-expressions
    x = parseMExpr(p, x);
    if(p->error) return x;

    // parse adverbs
    x = parseAdverb(p, x);

    // if followed by [, recurse
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
            return HANDLE_ERROR("unclosed string\n");
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
    l = d       ? parseInt(s, d) : 0;
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
        while ('0'==class(c) || '.'==c){
            if ('.'==c) { f += 1; d = p->current; }
            c = *++p->current;
        }

        // if more than one dot, error
        if (f > 1)
            return unref(r), HANDLE_ERROR("invalid number\n");

        // create a number and join it 
        t = ABS(TYP(r));
        if (f){
            // if float parsed after ints, cast the return object to float
            if (KI==t && CNT(r)){ 
                for (i32 i=0; i<CNT(r); i++) FLT(r)[i] = (double) INT(r)[i];
                r=tx(KF,r);
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
    } while ('0'==class(c) || '0'==class('-'==c?p->current[1+('.'==p->current[1])]:'.'==c?p->current[1]:0));

    return r;
}

static i64 encodeSym(Parser *p){
    i64 n = 0;
    char c, *s=p->current;
    while ('a'==(c=class(*p->current)) || '0'==c)
        ++p->current;
    // encode char in i64. max 8 chars per symbol
    memcpy(&n, s, (p->current-s)>8 ? 8 : p->current-s);
    return n;
}

// parse `a`b`c
static K parseSym(Parser *p){
    K r = ks(encodeSym(inc(p)));
    while('`'==peek(p)) r = j2(r, ks(encodeSym(inc(p))));
    // enlist, as sym literals are enlisted in K parse tree
    return KS==TYP(r) ? k1(r) : va(r); 
}

static K parseVar(Parser *p){
    return ks(encodeSym(p));
}

// parse ({[ Expressions... ]})
static K parseFenced(Parser *p, char b){
    char a=peek(p);
    // parse Expressions
    K r = ')'==a?tn(0,0):'}'==a?ku(':'):']'==a?k1(ks(0)):Exprs(')'==b?',':'}'==b?';':0, p);

    // if not properly closed, return error
    if (b != (a=peek(p)))
        return unref(r), !a ? HANDLE_ERROR("unexpected EOL\n") : HANDLE_ERROR("unexpected token: %c\n", a);

    return inc(p), r;
}

// parse func args {[...] }. must be sym list
static K parseArgs(Parser *p){
    K r = squeeze(parseFenced(p,']'));
    return KS==TYP(r) ? r : (unref(r),HANDLE_ERROR("invalid function args\n"));
}

static K classSwitch(Parser *p){
    char a, c, *s; //current char, char class, start of object
    K x, y; //x:parsed object, y:lambda params
    bool f=0; //is lambda function?

    a = next(p);
    // if "-" or "-." followed by digit, we're parsing a number, so classify appropriately
    c = '0'==class('-'==a?p->current['.'==*p->current]:'.'==a?*p->current:a) ? '0' : class(a);

    switch (c){
    case '0': x=parseNum(dec(p)); break;
    case '`': x=parseSym(dec(p)); break;
    case 'a': x=parseVar(dec(p)); break;
    case '"': x=parseStr(p); break;
    case '+': x=kv(a); a=peek(p); if(':'==a?inc(p),1:!AT_EXPR_END(a)&&'['!=a&&('/'!=class(a)||'\''==a)) x=tx(KU,x); break;
    case '/': x=parseAdverb(dec(p),0); break;
    case '{': f=1,s=p->current-1,y='['==peek(p)?parseArgs(inc(p)):k1(ks(0)); if (p->error) return y; //else FALLTHROUGH
    case '(': x=parseFenced(p,")}"[f]); if (p->error){ if(f)unref(y); return x; } break;
    default : return !a ? HANDLE_ERROR("unexpected EOL\n") : HANDLE_ERROR("unexpected token: %c\n", a);
    }

    // create lambda object
    if (f) x=k2(y,x), y=tn(KC,p->current-s), memcpy(CHR(y),s,p->current-s), x=tx(KL,j2(k1(y),x));

    return parsePostfix(p, x);
}

// parse single expression
static K expr(Parser *p){
    K x, y, z; //prefix, infix, right expression
    char a, c; //current char, char class

    // parse prefix
    x = classSwitch(p);
    if (p->error) return x;

    bool va = IS_VERB(x) || HEAD_IS_ADVERB(x);

    // set composition flag and return x
    if ( AT_EXPR_END(peek(p)) ) 
        return p->compose |= va, x;

    // parse +x
    if (va) 
        return y = expr(p), p->compose ? COMPOSE(x, y) : k2(x, y);

    // parse next term. some hairiness here
    // need to determine whether y is infix or start of another expression
    // if y is a verb ('*','+',"f/",etc) it is infix -> (y;x;expr())
    // else (y is a noun) it is the start of the next expr, so we rewind and parse (x;expr())
    // also, a '-' token can be dyadic minus or start of negative number
    // x-1 -> (-;`x;1) but x -1 -> (`x;-1)
    char *temp = p->current; // so we can rewind if y is a noun
    a = *p->current++;
    // switch based on whether a space precedes the term
    c = ' '==p->current[-2] ? ' ' : '+'==class(a) ? "+?"['.'==a&&'0'==class(*p->current)] : '?';
    switch (c){
    case ' ': if('+'!=class(a) || '0'==class('-'==a?p->current['.'==*p->current]:'.'==a?*p->current:0)){ y=classSwitch(dec(p)); break; } //else FALLTHROUGH
    case '+': y=':'==*p->current?inc(p),ku(a):kv(a); y=parsePostfix(p, y); break;
    default : y=classSwitch(dec(p));
    }

    // if y is a noun 
    if ( !( IS_VERB(y) || HEAD_IS_ADVERB(y) ) ){
        // return (x;y)
        if (AT_EXPR_END(peek(p))){
            return k2(x, y);
        }
        // else rewind and return (x;expr()) 
        unref(y);
        p->current = temp;
        return y = expr(p), p->compose ? COMPOSE(x, y) : k2(x, y);
    }

    // parse x+
    if ( AT_EXPR_END(peek(p)) ) 
        return p->compose = true, k2(y, x);
    
    // parse x+y
    z = expr(p);
    return (p->compose && !IS_K_SET(y)) ? COMPOSE(k2(y,x), z) : k3(y, x, z);
}

// parse ;-delimited Expressions
static K Exprs(char c, Parser *p){
    K r=tn(0,0), t;

    do {
        t = AT_EXPR_END(peek(p))       ?  //if at expr end
            (';'==c ? ku(':') : km())  :  //return null or magic value
            expr(p);                      //else parse expression
        r = jk(r,t);                      //join to previously parsed exprs
        p->compose = false;               //(re)set flags
    } while (';'==next(p));    
    --p->current;

    return !c ? r : 1==CNT(r) ? ref(t),unref(r),t : j2(k1(ku(c)), r);
}

K parse(char *src){
    K r;

    // init parser struct
    Parser p;
    p.error = false;
    p.compose = false;
    p.src = src;
    p.current = src;

    // return if only whitespace or comment in input
    if ('/'==*src || sc("\n\0", peek(&p))){
        return (K)0;
    }

    // parse Expressions
    r = Exprs(';', &p);
    return p.error ? unref(r),(K)0 : r;
}
