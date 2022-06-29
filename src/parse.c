#include "a.h"
#include "parse.h"
#include "vm.h"
#include "token.h"
#include "chunk.h"
#include "object.h"
#include "verb.h"

#define REPORT_ERROR(...)         \
        if(!parser->panic){       \
            printf(__VA_ARGS__);  \
            parser->panic = true; \
        }

#define COMPOSE(x, y) JOIN3(Kv(KA, '\''), x, y)

// full grammar...
//      <Exprs>  ::=  <Exprs> ";" <expr>  |  <expr>
//      <expr>   ::=  <noun> <verb> <expr>  |  <term> <expr>  |  empty
//      <term>   ::=  <noun>  |  <verb>
//      <verb>   ::=  <term> <Adverb>  |  <Verb>
//      <noun>   ::=  <term> "[" <Exprs> "]"  |  "(" <Exprs> ")"  |  "{" <Exprs> "}"  |  <Noun>

// forward declarations
static K expression(Scanner *scanner, Parser *parser);

// get the next token
static void advance(Scanner *scanner, Parser *parser){
    parser->current = nextToken(scanner);
}

static K appendExpression(K x, K y){
    if (0 == xn) 
        return unref(x), y;

    // copy x
    K r = k(KK, xn+1);
    for (uint64_t i = 0 ; i < xn; ++i) rk[i] = xk[i];

    // copy y
    rk[xn] = y;

    free(x);
    return r;
}

static bool atExprEnd(TokenType type){
    return TOKEN_EOF == type || TOKEN_RPAREN == type || TOKEN_RSQUARE == type || TOKEN_SEMICOLON == type;
}

static bool isNoun(TokenType type){
    return TOKEN_NUMBER == type  || TOKEN_LPAREN == type || TOKEN_STRING == type || 
           TOKEN_FLOAT  == type  || TOKEN_SYMBOL == type || TOKEN_ID     == type ||
           // or noun error
           TOKEN_UNCLOSED_STRING == type;
}

static bool isVerb(TokenType type){
    return  TOKEN_PLUS  <= type && TOKEN_COLON >= type;
}

static bool isAdverb(TokenType type){
    return TOKEN_FSLASH <= type && TOKEN_EACHPRIOR >= type;
}

static K parseNumber(Scanner *scanner, Parser *parser){
    K r, t;
    bool isFloat = (TOKEN_FLOAT == parser->current.type);
    uint16_t n = 0;
    r = k(isFloat ? -KF : -KI, 1);

    while (TOKEN_NUMBER == parser->current.type || TOKEN_FLOAT == parser->current.type) {
        if (rn == n){
            t = k(ABS(rt), GROW_CAPACITY(rn));
            if (isFloat)
                for (uint16_t i = 0; i < rn; ++i) tf[i] = rf[i];
            else 
                for (uint16_t i = 0; i < rn; ++i) ti[i] = ri[i];

            unref(r);
            r = t;
        }
        if (isFloat){
            rf[n++] = strtod(parser->current.start, NULL);
        }
        else {
            if (TOKEN_FLOAT == parser->current.type){
                isFloat = true;
                t = k(KF, rn);
                for (uint16_t i = 0; i < rn; ++i) tf[i] = (F)ri[i];
                unref(r);
                r = t;
                rf[n++] = strtod(parser->current.start, NULL);
            }
            else {
                ri[n++] = strtol(parser->current.start, NULL, 10);
            }
        }
        advance(scanner, parser);
    }

    rn = n;
    return r;
}

static K parseString(Scanner *scanner, Parser *parser){
    Token token = parser->current;
    uint64_t n = token.length - 2; // -2 to ignore the ""
    K r = k(1 == n ? -KC : KC, n);
    const C *s = token.start + 1; // source
    C *d = rc; // dest
    while(n--) { *d++ = *s++ ;}
    advance(scanner, parser);
    return r;
}

static K parseParens(Scanner *scanner, Parser *parser, TokenType paren){
    // check if empty generic list ()
    if (TOKEN_LPAREN == paren && TOKEN_RPAREN == peekToken(scanner).type){
        advance(scanner, parser);
        advance(scanner, parser);
        return k(KK, 0);
    }

    // check if [] 
    if (TOKEN_LSQUARE == paren && TOKEN_RSQUARE == peekToken(scanner).type){
        advance(scanner, parser);
        advance(scanner, parser);
        return KNUL;
    }
    
    uint16_t n = 0;
    K r, t;
    r = k(KK, 1);

    do {
        if (n == rn){
            t = k(KK, GROW_CAPACITY(rn));
            for (uint16_t i = 0; i < rn; ++i) tk[i] = ref(rk[i]);
            unref(r);
            r = t;
        }
        advance(scanner, parser);
        rk[n++] = atExprEnd(parser->current.type) ? k(-KN, 0) : 
                  TOKEN_DOUBLECOLON == parser->current.type ? ( advance(scanner, parser), KNUL ) :
                  expression(scanner, parser);
        parser->compose = false;
                  
    } while (TOKEN_SEMICOLON == parser->current.type);

    if ((TOKEN_LPAREN  == paren && TOKEN_RPAREN  != parser->current.type) || 
        (TOKEN_LSQUARE == paren && TOKEN_RSQUARE != parser->current.type) ){
        REPORT_ERROR("Parse error! Expected ')' or ']'\n");
        while (n--) unref(rk[n]);
        free(r);
        return KNUL;
    }

    if (TOKEN_LSQUARE == paren && 8 < n){
        REPORT_ERROR("Limit error! Too many arguments (max 8)\n");
        while (n--) unref(rk[n]);
        free(r);
        return KNUL;
    }
    
    rn = n;
    if (TOKEN_LSQUARE == paren){
        t = k(KK, rn);
        for (uint16_t i = 0; i < rn; ++i) tk[i] = rk[i];
        free(r);
        r = t;
    }
    else if (1 == n){
        t = rk[0];
        free(r);
        r = t;
    }
    else {
        t = k(KK, rn+1);
        tk[0] = k(KU, 1);
        CHAR( tk[0] )[0] = (char) TOKEN_COMMA;
        for (uint16_t i = 0; i < rn; ++i) tk[i+1] = rk[i];
        free(r);
        r = t;
    }
    advance(scanner, parser);
    return r;
}

// m-expression +[;] 
static K parseMExpr(Scanner *scanner, Parser *parser, K x){
    while (TOKEN_LSQUARE == parser->current.type){
        x = cat(enlist(x), parseParens(scanner, parser, TOKEN_LSQUARE));
    }
    return x;
}

static K parseSymbol(Scanner *scanner, Parser *parser){
    K r, t;
    uint64_t n = 0, j;
    r = k(-KS, 1);
    const char *str;
    while (TOKEN_SYMBOL == parser->current.type){
        if (n == rn){
            t = k(KS, GROW_CAPACITY(rn));
            for (uint64_t i = 0; i < rn; ++i) ti[i] = ri[i];
            unref(r);
            r = t;
        }
        str = &parser->current.start[1];
        j = n++;
        MAKE_SYMBOL(ri[j], str, parser->current.length-1);
        advance(scanner, parser);
    }
    rn = n;
    return enlist(r);
}

static K parseId(Scanner *scanner, Parser *parser){
    K r = k(-KS, 1);
    const char *str = &parser->current.start[0];
    MAKE_SYMBOL(ri[0], str, parser->current.length);
    advance(scanner, parser);
    return r;
}

static K parseNoun(Scanner *scanner, Parser *parser){
    K r;

    // if error
    if (TOKEN_UNCLOSED_STRING == parser->current.type){
        REPORT_ERROR("Parse error! Unclosed string.\n");
        advance(scanner, parser);
        r = KNUL;
    }
    // if number. eg 123 or 4.56
    else if (TOKEN_NUMBER == parser->current.type || TOKEN_FLOAT == parser->current.type){
        r = parseNumber(scanner, parser);
    }
    // if string. eg "foobar"
    else if (TOKEN_STRING == parser->current.type){
        r = parseString(scanner, parser);
    }
    // if parens. eg (1+2)%3 
    else if (TOKEN_LPAREN == parser->current.type){
        r = parseParens(scanner, parser, TOKEN_LPAREN);
    }
    else if (TOKEN_SYMBOL == parser->current.type){
        r = parseSymbol(scanner, parser);
    }
    else if (TOKEN_ID == parser->current.type){
        r = parseId(scanner, parser);
    }
    else {
        REPORT_ERROR("Error! Noun not yet implemented.\n");
        r = KNUL;
    }

    return parseMExpr(scanner, parser, r);
}

static K parseVerb(Scanner *scanner, Parser *parser, int8_t type){
    K r = k(type, 1);
    rc[0] = (char) parser->current.type;
    advance(scanner, parser);
    return parseMExpr(scanner, parser, r);
}

// an arbitrary number of adverbs can be combined. eg:
// x g\:/: y
static K parseAdverbIter(Scanner *scanner, Parser *parser, K x){
    K adverb, t;
    while (isAdverb(parser->current.type)){
        adverb = k(KA, 1);
        CHAR(adverb)[0] = (char) parser->current.type;
        t = JOIN2(adverb, x);
        x = t;
        advance(scanner, parser);
    }
    return parseMExpr(scanner, parser, x); 
}

// <expression>   ::=  <noun> <verb> <expression>  |  <term> <expr>  |  empty
static K expression(Scanner *scanner, Parser *parser){
    // error!
    if (TOKEN_EOF == parser->current.type){
        return Kerr("error! EOF");
    }

    K prefix, infix, r, t;
    bool prefixIsAdverb = false;
    if (NULL == parser->prefix){
        prefix = 
            isNoun(parser->current.type) ? parseNoun(scanner, parser) :
            isVerb(parser->current.type) && isAdverb(peekToken(scanner).type) ? parseVerb(scanner, parser, KV) :
            isVerb(parser->current.type) && atExprEnd(peekToken(scanner).type) ? parseVerb(scanner, parser, KV) :
            isVerb(parser->current.type) && TOKEN_LSQUARE == peekToken(scanner).type ? parseVerb(scanner, parser, KV) :
            isAdverb(parser->current.type) ? parseVerb(scanner, parser, KA) :
            parseVerb(scanner, parser, KU) ;
    }
    else {
        prefix = parser->prefix;
        parser->prefix = NULL;
    }

    if (isAdverb(parser->current.type)){
        prefix = parseAdverbIter(scanner, parser, prefix);
        prefixIsAdverb = true;
    }

    if (atExprEnd(parser->current.type)){
        r = prefix;
    }
    else if (prefixIsAdverb || KU == TYPE(prefix)){
        t = expression(scanner, parser);
        r = ( parser->compose ) ? COMPOSE(prefix, t) : JOIN2(prefix, t);
    }
    else if (isNoun(parser->current.type)){
        infix = parseNoun(scanner, parser);
        if (isAdverb(parser->current.type)){
            infix = parseAdverbIter(scanner, parser, infix);
            if (atExprEnd(parser->current.type)){
                r = JOIN2(prefix, infix);
            }
            else {
                t = expression(scanner, parser);
                r = ( parser->compose ) ? COMPOSE(JOIN2(infix, prefix), t) : JOIN3(infix, prefix, t);
            }
        }
        else {
            if (atExprEnd(parser->current.type)){
                r = JOIN2(prefix, infix);
            }
            else {
                parser->prefix = infix;
                t = expression(scanner, parser);
                r = ( parser->compose ) ? COMPOSE(prefix, t) : JOIN2(prefix, t);
            }
        }
    }
    else {
        infix = parseVerb(scanner, parser, KV);
        if (isAdverb(parser->current.type))
            infix = parseAdverbIter(scanner, parser, infix);
        if (atExprEnd(parser->current.type)){
            parser->compose = true;
            r = JOIN3(infix, prefix, k(-KN, 0));
        }
        else {
            t = expression(scanner, parser);
            r = ( parser->compose ) ? COMPOSE(JOIN2(infix, prefix), t) : JOIN3(infix, prefix, t);
        }
    }
    
    return r;
}

// <Expressions>  ::=  <Exprs> ";" <expression>  |  <expression>
static K Expressions(Scanner *scanner, Parser *parser){
    K t, r = k(KK, 0);

    // parse expressions
    do {
        advance(scanner, parser);
        t = ( TOKEN_EOF == parser->current.type ) ? KNUL : expression(scanner, parser);
        
        if (TOKEN_SEMICOLON == parser->current.type && 0 == rn){
            unref(r);
            r = JOIN2(Kc(';'), t);
        }
        else {
            r = appendExpression(r, t);
        }
        parser->compose = false;
    } while (TOKEN_SEMICOLON == parser->current.type);
    
    if (TOKEN_EOF != parser->current.type){
        // error. we shouldn't reach this branch
        REPORT_ERROR("Unexpected token '%.*s'\n",parser->current.length,parser->current.start);
        unref(r);
        return k(KE, 0);
    }

    return r;
}

K parse(const char *source){ 
    // init a new scanner instance
    Scanner *scanner = initNewScanner(source);
    // init the parser. the parser is simple struct containing previous&current token and a panic flag
    Parser parser;
    parser.prefix = NULL;
    parser.compose = false;
    parser.panic = false;

    K r = Expressions(scanner, &parser);

    if (parser.panic){
        unref(r);
        free(scanner);
        return NULL;
    }

    free(scanner);
    return r;
}
