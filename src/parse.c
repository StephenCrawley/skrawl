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
    return TOKEN_EOF == type || TOKEN_RPAREN == type || TOKEN_SEMICOLON == type;
}

static bool isNoun(TokenType type){
    return TOKEN_NUMBER == type || TOKEN_LPAREN == type || TOKEN_STRING == type || 
           TOKEN_FLOAT  == type || TOKEN_SYMBOL == type || TOKEN_ID     == type;
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

static K parseParens(Scanner *scanner, Parser *parser){
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
        rk[n++] = expression(scanner, parser);
    } while (TOKEN_SEMICOLON == parser->current.type);

    if (TOKEN_RPAREN != parser->current.type){
        REPORT_ERROR("Parse error! Expected ')'\n");
        while (n--) unref(rk[n]);
        free(r);
        return KNUL;
    }
    rn = n;
    if (1 == n){
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

    // if number. eg 123 or 4.56
    if (TOKEN_NUMBER == parser->current.type || TOKEN_FLOAT == parser->current.type){
        r = parseNumber(scanner, parser);
    }
    // if string. eg "foobar"
    else if (TOKEN_STRING == parser->current.type){
        r = parseString(scanner, parser);
    }
    // if parens. eg (1+2)%3 
    else if (TOKEN_LPAREN == parser->current.type){
        r = parseParens(scanner, parser);
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

    return r;
}

static K parseVerb(Scanner *scanner, Parser *parser, int8_t type){
    K r = k(type, 1);
    rc[0] = (char) parser->current.type;
    advance(scanner, parser);
    return r;
}

// an arbitrary number of adverbs can be combined. eg:
// x g\:/: y
static K parseAdverbIter(Scanner *scanner, Parser *parser, K x){
    K adverb, t;
    K r = x;
    while (isAdverb(parser->current.type)){
        adverb = k(KA, 1);
        CHAR(adverb)[0] = (char) parser->current.type;
        t = k(KK, 2);
        tk[0] = adverb;
        tk[1] = r;
        r = t;
        advance(scanner, parser);
    }
    return r; 
}

// <expression>   ::=  <noun> <verb> <expression>  |  <term> <expr>  |  empty
static K expression(Scanner *scanner, Parser *parser){
    // error!
    if (TOKEN_EOF == parser->current.type){
        return Kerr("error! EOF");
    }

    K prefix, infix, r;
    bool prefixIsAdverb = false;
    if (NULL == parser->prefix){
        prefix = 
            isNoun(parser->current.type) ? parseNoun(scanner, parser) :
            isVerb(parser->current.type) && isAdverb(peekToken(scanner).type) ? parseVerb(scanner, parser, KV) :
            isVerb(parser->current.type) && atExprEnd(peekToken(scanner).type) ? parseVerb(scanner, parser, KV) :
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
        r = k(KK, 2);
        rk[0] = prefix;
        rk[1] = expression(scanner, parser);
    }
    else if (isNoun(parser->current.type)){
        infix = parseNoun(scanner, parser);
        if (isAdverb(parser->current.type)){
            infix = parseAdverbIter(scanner, parser, infix);
            if (atExprEnd(parser->current.type)){
                r = k(KK, 2);
                rk[0] = prefix;
                rk[1] = infix;
            }
            else {
                r = k(KK, 3);
                rk[0] = infix;
                rk[1] = prefix;
                rk[2] = expression(scanner, parser);
            }
        }
        else {
            if (atExprEnd(parser->current.type)){
                r = k(KK, 2);
                rk[0] = prefix;
                rk[1] = infix;
            }
            else {
                r = k(KK, 2);
                rk[0] = prefix;
                parser->prefix = infix;
                rk[1] = expression(scanner, parser);
            }
        }
    }
    else {
        infix = parseVerb(scanner, parser, KV);
        if (isAdverb(parser->current.type))
            infix = parseAdverbIter(scanner, parser, infix);
        
        r = k(KK, 3);
        rk[0] = infix;
        rk[1] = prefix;
        rk[2] = atExprEnd(parser->current.type) ? k(-KN, 0) : expression(scanner, parser);
    }
    
    return r;
}

// <Expressions>  ::=  <Exprs> ";" <expression>  |  <expression>
static K Expressions(Scanner *scanner, Parser *parser){
    K t, r = k(KK, 0);

    // parse expressions
    do {
        advance(scanner, parser);
        t = TOKEN_EOF == parser->current.type ? KNUL : expression(scanner, parser);
        
        if (TOKEN_SEMICOLON == parser->current.type && 0 == rn){
            r = k(KK, 2);
            rk[0] = Kc(';');
            rk[1] = t;
        }
        else {
            r = appendExpression(r, t);
        }
    } while (TOKEN_SEMICOLON == parser->current.type);
    
    if (TOKEN_EOF != parser->current.type){
        // error. we shouldn't reach this branch
        REPORT_ERROR("Unexpected token '%.*s'\n",parser->current.length,parser->current.start);
        unref(r);
        return k(KE, 0);
    }

    return r;
}

bool parse(const char *source, Chunk *chunk){ 
    // init a new scanner instance
    Scanner *scanner = initNewScanner(source);
    // init the parser. the parser is simple struct containing previous&current token and a panic flag
    Parser parser;
    parser.prefix = NULL;
    parser.panic = false;

    K r = Expressions(scanner, &parser);
    chunk->parseTree = r;

    if (parser.panic){
        freeChunk(chunk);
        free(scanner);
        chunk = NULL;
        return false;
    }

    free(scanner);
    return true;
}
