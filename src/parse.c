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
static K Expressions(K head, Scanner *scanner, Parser *parser);

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
    // check if empty generic list () or if []
    bool p;
    if ((p = (TOKEN_LPAREN  == paren && TOKEN_RPAREN == peekToken(scanner).type)) || 
             (TOKEN_LSQUARE == paren && TOKEN_RSQUARE == peekToken(scanner).type)){
        advance(scanner, parser);
        advance(scanner, parser);
        return p ? k(KK, 0) : KNUL;
    }
    
    K r = Expressions( (TOKEN_LSQUARE == paren) ? NULL : Kv(KU, ','), scanner, parser);

    if ((TOKEN_LPAREN  == paren && TOKEN_RPAREN  != parser->current.type) || 
        (TOKEN_LSQUARE == paren && TOKEN_RSQUARE != parser->current.type) ){
        REPORT_ERROR("Parse error! Expected ')' or ']'\n");
        unref(r);
        return KNUL;
    }

    if (TOKEN_LSQUARE == paren && 8 < rn){
        REPORT_ERROR("Limit error! Too many arguments (max 8)\n");
        unref(r);
        return KNUL;
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

static K parseNounSwitch(Scanner *scanner, Parser *parser){
    switch(parser->current.type){
        case TOKEN_NUMBER:
        case TOKEN_FLOAT:  return parseNumber(scanner, parser); 
        case TOKEN_STRING: return parseString(scanner, parser); 
        case TOKEN_LPAREN: return parseParens(scanner, parser, TOKEN_LPAREN);
        case TOKEN_SYMBOL: return parseSymbol(scanner, parser);
        case TOKEN_ID:     return parseId(scanner, parser);
        case TOKEN_UNCLOSED_STRING:{
            REPORT_ERROR("Parse error! Unclosed string.\n"); 
            advance(scanner, parser);
            break;
        }
        default: REPORT_ERROR("Error! Noun not yet implemented.\n");
    }
    return KNUL;
}

static K parseNoun(Scanner *scanner, Parser *parser){
    return parseMExpr(scanner, parser, parseNounSwitch(scanner, parser));
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
    // error! (is this needed?)
    if (TOKEN_EOF == parser->current.type){
        return Kerr("error! EOF");
    }

    K prefix, infix, r, t;
    bool prefixIsAdverb = false;
    prefix = 
        isNoun(parser->current.type) ? parseNoun(scanner, parser) :
        isVerb(parser->current.type) && TOKEN_APOSTROPHE == peekToken(scanner).type ? parseVerb(scanner, parser, KU) :
        isVerb(parser->current.type) && isAdverb(peekToken(scanner).type) ? parseVerb(scanner, parser, KV) :
        isVerb(parser->current.type) && atExprEnd(peekToken(scanner).type) ? parseVerb(scanner, parser, KV) :
        isVerb(parser->current.type) && TOKEN_LSQUARE == peekToken(scanner).type ? parseVerb(scanner, parser, KV) :
        isAdverb(parser->current.type) ? parseVerb(scanner, parser, KA) :
        parseVerb(scanner, parser, KU) ;

    if (isAdverb(parser->current.type)){
        prefix = parseAdverbIter(scanner, parser, prefix);
        prefixIsAdverb = true;
    }

    if (atExprEnd(parser->current.type)){
        if (KU == TYPE(prefix) || KV == TYPE(prefix)){
            parser->compose = true;
        }
        r = prefix;
    }
    else if (prefixIsAdverb || KU == TYPE(prefix)){
        t = expression(scanner, parser);
        r = ( parser->compose ) ? COMPOSE(prefix, t) : JOIN2(prefix, t);
    }
    else if (isVerb(parser->current.type) && TOKEN_LSQUARE != peekToken(scanner).type){
        infix = parseVerb(scanner, parser, KV);
        if (isAdverb(parser->current.type))
            infix = parseAdverbIter(scanner, parser, infix);
        if (atExprEnd(parser->current.type)){
            parser->compose = true;
            r = JOIN3(infix, prefix, k(-KN, 0));
        }
        else {
            t = expression(scanner, parser);
            r = ( parser->compose && TOKEN_COLON != *CHAR(infix) ) ? COMPOSE(JOIN2(infix, prefix), t) : JOIN3(infix, prefix, t);
        }
    }
    else /*if (isNoun(parser->current.type))*/{
        Scanner stemp = *scanner;
        Parser  ptemp = *parser;
        infix = ( isVerb(parser->current.type) ) ? parseVerb(scanner, parser, KV) : parseNoun(scanner, parser);
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
                unref(infix);
                *scanner = stemp;
                *parser  = ptemp;
                t = expression(scanner, parser);
                r = ( parser->compose ) ? COMPOSE(prefix, t) : JOIN2(prefix, t);
            }
        }
    }
    
    return r;
}

// <Expressions>  ::=  <Exprs> ";" <expression>  |  <expression>
static K Expressions(K head, Scanner *scanner, Parser *parser){
    K t, r = k(KK, 0);
    bool b = true;

    // parse expressions
    do {
        advance(scanner, parser);
        t = ( TOKEN_EOF == parser->current.type ) ? KNUL : expression(scanner, parser);
        
        if (TOKEN_SEMICOLON == parser->current.type && 0 == rn){
            unref(r);
            r = ( NULL == head ) ? ENLIST(t) : (b = false, JOIN2(head, t)); // r = JOIN2(Kc(';'), t);
        }
        else {
            r = appendExpression(r, t);
        }
        parser->compose = false;
    } while (TOKEN_SEMICOLON == parser->current.type);
    
    if (b && NULL!=head) unref(head);
    return r;
}

K parse(const char *source){ 
    // init a new scanner instance
    Scanner *scanner = initNewScanner(source);
    // init the parser. the parser is simple struct containing current token, a compose flag, and a panic flag
    Parser parser;
    parser.compose = false;
    parser.panic = false;

    K r = Expressions(Kc(';'), scanner, &parser);

    if (TOKEN_EOF != parser.current.type){
        // error. we shouldn't reach this branch
        if (!parser.panic) printf("Unexpected token '%.*s'\n",parser.current.length,parser.current.start);
        parser.panic = true;
        unref(r);
        r = k(KE, 0);
    }

    if (parser.panic){
        unref(r);
        free(scanner);
        return NULL;
    }

    free(scanner);
    return r;
}
