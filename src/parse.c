#include "parse.h"

#define AT_EXPR_END(c) sc(";)]}\n\0", (c))
#define COMPOSE(x,y) __extension__({K _x=(x),_y=(y); k3(kw(0),_x,_y) ;})
#define HEAD_IS_ADVERB(x) ( KK==TYP(x) && KW==TYP(*OBJ(x)) )
#define IS_K_SET(x) (TAG_TYP(x) ? 0==TAG_VAL(x) : 0)

// foward declarations
static K expr(Parser *p);
static K Exprs(char c, Parser *p);

// set error flag, print error, return error object
// 'a' is flag to decide error msg to print. if >=0, it is an unexpected char in input stream
static K handleError(Parser *p, char a){
    if (p->error) return ke(); //if already error, return KE object
    p->error = true;
    // print src code and an arrow to the char that caused error
    // can't print src when invalid arg (-3) as we don't keep track of which arg has the error
    if (-2<=a) { puts(p->src); printf("%*s^\n",(int)(p->current-p->src),""); };
    -3==a ? printf("'parse! invalid lambda args\n") :
    -2==a ? printf("'parse! unclosed string\n")     :
    -1==a ? printf("'parse! invalid number\n")      : 
       !a ? printf("'parse! unexpected EOL\n")      : 
            printf("'parse! unexpected token: %c\n",a);
    return ke();
}

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

// parse ({[ Expressions... ]})
static K parseFenced(Parser *p, char b){
    char a=peek(p);
    // parse Expressions
    K r = ')'==a?tn(0,0):sc("}]",a)?kuc(':'):Exprs(')'==b?',':'}'==b?';':0, p);

    // if not properly closed, return error
    if (b != (a=peek(p)))
        return unref(r), handleError(p, a);

    return inc(p), r;
}

// parse x'  
static K parseAdverb(Parser *p, K x){
    char t; //type
    while ( '/' == class(peek(p)) ){
        t = iadverb(*p->current++);
        if (':'==*p->current){ ++p->current; t+=3; }
        // special case: if x==0 we're parsing bare adverbs (eg "'/")
        x = x ? k2(kw(t), x) : kw(t);
    }
    return x;
}

// parse x[y;z]
static K parseMExpr(Parser *p, K x){
    K r;
    while ('[' == peek(p)){
        // parse [ ... ] and return if error. if empty [] (type KU) then box r
        r = parseFenced(inc(p),']');
        if (p->error)return unref(x), r;
        if (KU==TYP(r)) r = k1(r);
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
    return '['==peek(p) ? parsePostfix(p,x) : x;
}

// parse "abc", "", etc
static K parseStr(Parser *p){
    const char *s = p->current;
    do { // if we hit EOL, error
        if (!*p->current) return handleError(p,-2);
    } while ('"' != *p->current++);
    i64 n = p->current-s-1;
    K r = tn(1 == n ? -KC : KC, n);
    memcpy(CHR(r), s, n);
    return r;
}

// parse 123
static i64 parseInt(const char *s, i32 len){
    if ('-' == *s) return -parseInt(++s, len-1);
    i64 n = 0;
    for (i64 i=0; i<len; i++) n = (n*10) + (s[i]-'0'); 
    return n;
}

// parse 1.23
static double parseFlt(const char *s, i32 len, i32 d){
    if ('-' == *s) return -parseFlt(++s, len-1, d-1);
    double l, r; //left of dot, right of dot
    l = (double)(d       ? parseInt(s, d) : 0);
    r = (double)(d<len-1 ? parseInt(s+d+1, len-d-1) : 0);
    for (i32 i=0, n=len-d-1; i<n; i++) r/=10.0;
    return l + r;
}

// parse number(s). "1","1 23"
static K parseNum(Parser *p){
    char a; //current char
    i8 n, d, f=0; //num length, count of dots '.', any floats?
    K r = tn(KI, 0), t;
    do {
        d = 0;
        n = ('-'==*p->current);
        do d+=('.'==p->current[n]), a=p->current[++n]; while ('0'==class(a) || '.'==a);

        // if more than one dot, error
        if (d > 1)
            return unref(r), r=handleError(p,-1), p->current+=n, r;

        f |= d;
        t = d ? kf(parseFlt(p->current, n, ic((char *)p->current, '.'))) : ki(parseInt(p->current, n));
        r = jk(r, t);
        p->current += n;

        // if not whitespace, then we're done parsing num literals
        if (' '!=*p->current) break;
        a = peek(p);
    } while ('0'==class(a) || '0'==class('-'==a?p->current[1+('.'==p->current[1])]:'.'==a?p->current[1]:0));

    // if just one number, return
    if (1==CNT(r)) return ref(t), unref(r), t;

    // else squeeze into compact form
    t = tn(f?KF:KI, CNT(r));
    if (f)
        for (i64 i=0, n=CNT(t); i<n; i++) FLT(t)[i] = -KI==TYP(OBJ(r)[i]) ? (double)*INT(OBJ(r)[i]) : *FLT(OBJ(r)[i]);
    else 
        for (i64 i=0, n=CNT(t); i<n; i++) INT(t)[i] = *INT(OBJ(r)[i]);
    
    return unref(r), t;
}

static i64 encodeSym(Parser *p){
    i64 n = 0;
    char c;
    const char *s = p->current;
    while ('a'==(c=class(*p->current)) || '0'==c) { ++p->current; }
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

// parse func args {[...] }. must be sym list
static K parseArgs(Parser *p){
    if (']'==peek(p)) 
        return ++p->current, squeeze(k1(ks('x')));
    K r = squeeze(parseFenced(p,']'));
    if (p->error) return r;
    return KS==TYP(r) ? r : (unref(r),handleError(p,-3));
}

static K classSwitch(Parser *p){
    char a, c; //current char, char class, 
    const char *s; // start of object
    K x, y; //x:parsed object, y:lambda params
    bool f=0; //is lambda function?

    a = next(p);
    // if "-" or "-." followed by digit, we're parsing a number, so classify appropriately
    c = '0'==class('-'==a?p->current['.'==*p->current]:'.'==a?*p->current:a) ? '0' : class(a);

    switch (c){
    case '0': x=parseNum(dec(p)); if (p->error)return x; break;
    case '`': x=parseSym(dec(p)); break;
    case 'a': x=parseVar(dec(p)); break;
    case '"': x=parseStr(p); break;
    case '+': x=kvc(a); a=peek(p); if(':'==a?inc(p),1:!AT_EXPR_END(a)&&'['!=a&&('/'!=class(a)||'\''==a)) x=tx(KU,x); break;
    case '/': x=parseAdverb(dec(p),0); break;
    case '{': f=1,s=p->current-1,y='['==peek(p)?parseArgs(inc(p)):squeeze(k1(ks('x'))); if (p->error) return y; //else FALLTHROUGH
    case '(': x=parseFenced(p,")}"[f]); if (p->error){ if(f)unref(y); return x; } break;
    default : return handleError(p, a);
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

    bool va = ')'!=p->current[-1] && (IS_VERB(x) || HEAD_IS_ADVERB(x));

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
    const char *temp = p->current; // so we can rewind if y is a noun
    a = *p->current++;
    // switch based on whether a space precedes the term
    c = ' '==p->current[-2] ? ' ' : '+'==class(a) ? "+?"['.'==a&&'0'==class(*p->current)] : '?';
    switch (c){
    case ' ': if('+'!=class(a) || '0'==class('-'==a?p->current['.'==*p->current]:'.'==a?*p->current:0)){ y=classSwitch(dec(p)); break; } //else FALLTHROUGH
    case '+': y=':'==*p->current?inc(p),kuc(a):kvc(a); y=parsePostfix(p, y); break;
    default : y=classSwitch(dec(p));
    }

    // if y is a noun 
    if ( ')'==p->current[-1] || !( IS_VERB(y) || HEAD_IS_ADVERB(y) ) ){
        // return (x;y)
        if (AT_EXPR_END(peek(p))) return k2(x, y); 

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
            (';'==c ? kuc(':') : km())  :  //return null or magic value
            expr(p);                      //else parse expression
        r = jk(r,t);                      //join to previously parsed exprs
        p->compose = false;               //(re)set flags
    } while (';'==next(p));    
    --p->current;

    return !c ? r : 1==CNT(r) ? ref(t),unref(r),t : j2(k1(kuc(c)), r);
}

K parse(const char *src){
    K r;

    // init parser struct
    Parser p;
    p.error = false;
    p.compose = false;
    p.src = src;
    p.current = src;

    // return if only whitespace or comment in input
    if ('/'==*src || sc("\n\0", peek(&p)))
        return knul();

    // parse Expressions
    r = Exprs(';', &p);
    // should be at EOL after calling Exprs()
    r = !peek(&p) ? r : (unref(r),handleError(&p,*p.current));
    return p.error ? unref(r),ke() : r;
}

#define SRC_MAX 128  //max repl source length

K readK(){
    // print prompt and read from stdin
    char src[SRC_MAX], *s;
    putchar(' ');
    fgets(src, SRC_MAX, stdin);

    // replace newline with 0
    if ((s = sc(src,'\n'))) *s = 0;

    // if not a system command, parse and return
    if ('\\' != *(s=src))
        return parse(s);

    if (!*++s || '\\'==*s)
        exit(0);

    return printf("'error! unknown command\n"), (K)0;
}
