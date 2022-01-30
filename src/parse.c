#include "a.h"
#include "parse.h"
#include "vm.h"
#include "token.h"
#include "chunk.h"
#include "object.h"

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
static void advance(Parser *parser, Scanner *scanner){
    parser->previous = parser->current;
    parser->current = nextToken(scanner);
}

static K append(K x, K y){
    uint64_t n = xn+1;                                    
    K r = k(KK, n);
    // copy x
    if (KK == xt){
        for (uint64_t i = 0 ; i < xn; ++i) rk[i] = xk[i];
    }
    else {
        rk[0] = x;
    }
    // copy y
    rk[xn] = y;

    return r;
}

// TODO : this function is buggy
// join/append logic in general needs to be improved. can probably do without it
// join flat. eg join( (1 2;"ab"), (3.0; 8) ) 
// returns a K object with count 4: (1 2;"ab";3.0;8)
static K join(K x, K y){
    K r;
    if (KK == xt && KK == yt){
        r = k(KK, xn+yn);
        for (uint64_t i = 0 ; i < xn   ; ++i) rk[i] = xk[i];
        for (uint64_t i = xn; i < xn+yn; ++i) rk[i+xn] = yk[i];
    }
    else if (KK == xt){
        r = k(KK, xn+1);
        for (uint64_t i = 0 ; i < xn   ; ++i) rk[i] = xk[i];
        rk[rn-1] = y;
    }
    else if (KK == yt){
        r = k(KK, yn+1);
        rk[0] = x;
        for (uint64_t i = 1, n = yn+1 ; i < n; ++i) rk[i] = yk[i];
    }
    else {
        r = k(KK, 2);
        rk[0] = x;
        rk[1] = y;
    }
    return r;
}

static bool atExprEnd(TokenType type){
    return TOKEN_EOF == type || TOKEN_RPAREN == type || TOKEN_SEMICOLON == type;
}

static bool atNoun(TokenType type){
    return TOKEN_NUMBER == type || TOKEN_LPAREN == type || TOKEN_STRING == type || TOKEN_FLOAT == type || TOKEN_SYMBOL == type;
}

static bool atVerb(TokenType type){
    return  TOKEN_PLUS  == type || TOKEN_STAR   == type || TOKEN_MINUS  == type || TOKEN_DIVIDE == type ||
            TOKEN_PIPE  == type || TOKEN_BANG   == type || TOKEN_COMMA  == type || TOKEN_AND    == type ||
            TOKEN_AT    == type || TOKEN_HASH   == type || TOKEN_CARET  == type || TOKEN_DOLLAR == type ||
            TOKEN_TILDE == type || TOKEN_EQUAL  == type || TOKEN_LANGLE == type || TOKEN_RANGLE == type ||
            TOKEN_QMARK == type || TOKEN_USCORE == type;
}

static bool atAdverb(TokenType type){
    return TOKEN_FSLASH == type || TOKEN_BSLASH == type || TOKEN_APOSTROPHE == type || TOKEN_EACHR == type || TOKEN_EACHL == type || TOKEN_EACHPRIOR == type;
}

// to parse a list of numbers we do the following:
//   store the first number as a K atom in temp array t
//   while the next token is a number/float ->
//     if we reached the capacity of t, grow t
//     convert the next token into an atom and append to t
//     check whether any of the tokens are floats
//   then we copy the temp array into a compact K object (simple K list), int or float depending if there were any floats
// TODO : consider different approach:
// copy items into simple K array directly, casting as necessary
// eg if parsing "1 2 3. 4" 
//   1st elem is int so copy 1 into KI object, then 2 into the same object. when "3." is encountered, copy 1 2 into a new KF object
//   then append 3. into the object, and any remaining elems, casting as necessary
static K parseNumber(Parser *parser, Scanner *scanner){
    uint64_t capacity = 8;
    K *t = malloc( sizeof(K) * capacity );
    t[0] = (TOKEN_NUMBER == parser->previous.type) ? Ki(strtol(parser->previous.start, NULL, 10)) : Kf(strtod(parser->previous.start, NULL));
    bool anyFloats = (TOKEN_FLOAT == parser->previous.type);

    // j is the index to place the next token / count of the numbers parsed
    uint64_t j = 1;

    // while the next token is a number, stick it into temp array t
    while (TOKEN_NUMBER == parser->current.type || TOKEN_FLOAT == parser->current.type){
        // if we reach capacity, expand the array
        if (j == capacity){
            capacity = GROW_CAPACITY(capacity);
            t = realloc(t, sizeof(K) * capacity);
            if (NULL == t){
                printf("Failed to realloc during number literal parse");
                exit(1);
            }
        }
        // append token to end of temp array
        advance(parser, scanner);
        t[j] = (TOKEN_NUMBER == parser->previous.type) ? Ki(strtol(parser->previous.start, NULL, 10)) : Kf(strtod(parser->previous.start, NULL));
        anyFloats = MAX(anyFloats, TOKEN_FLOAT == parser->previous.type);
        ++j;
    }

    // copy the temp array into a K object
    int8_t rtype = anyFloats ? KF : KI;
    rtype = (1 == j) ? -rtype : rtype; 
    K r = k(rtype, j);
    if (anyFloats)
        for (uint64_t i = 0; i < j; ++i) rf[i] = (-KF == TYPE(t[i])) ? FLOAT(t[i])[0] : (F)INT(t[i])[0], unref(t[i]);
    else {
        for (uint64_t i = 0; i < j; ++i) ri[i] = INT(t[i])[0], unref(t[i]);
    }

    free(t);
    return r;
}

static K parseString(Parser *parser, Scanner *scanner){
    Token token = parser->previous;
    uint64_t n = token.length - 2; // -2 to ignore the ""
    K r = k(1 == n ? -KC : KC, n);
    const C *s = token.start + 1; // source
    C *d = rc; // dest
    while(n--) { *d++ = *s++ ;}
    return r;
}

static K parseParens(Parser *parser, Scanner *scanner){
    if (TOKEN_EOF == parser->current.type){
        REPORT_ERROR("Expected expression after '('\n");
    }

    uint64_t capacity = 8;
    K *arr = malloc( capacity * sizeof(K) );
    uint64_t n = 1;
    arr[0] = expression(scanner, parser);
    while (TOKEN_SEMICOLON == parser->current.type){
        if (n == capacity){
            capacity = GROW_CAPACITY(capacity);
            arr = realloc(arr, capacity * sizeof(K) );
            if (NULL == arr){
                printf("Failed to realloc during general list parse");
                exit(1);            
            }
        }
        advance(parser, scanner);
        arr[n] = expression(scanner, parser);
        ++n;
    }
    if (TOKEN_RPAREN != parser->current.type){
        free(arr);
        REPORT_ERROR("Expected ')'\n");
        if (TOKEN_EOF == parser->current.type){
            return KNUL;
        }
    }
    K r;
    if (1 == n) {
        r = arr[0];
    }
    else {
        // if the parens were a general list - eg (1;`a) - we return list (,:;1;`a)
        // where first elem is the enlist operator
        r = k(KK, n+1);
        for (uint64_t i = 0, j = 1; i < n; ++i,++j) rk[j] = arr[i];
        K t = k(KM, 1);
        tc[0] = (char) TOKEN_COMMA; //enlist op
        rk[0] = t;
    }
    free(arr);
    advance(parser, scanner);
    return r;
}

static K parseSymbol(Parser *parser, Scanner *scanner){
    K r, t;
    uint64_t capacity = 1, j = 1;
    r = k(-KS, capacity);
    const char *str = &parser->previous.start[0];
    str++;
    MAKE_SYMBOL(ri[0], str, parser->previous.length-1);
    while (TOKEN_SYMBOL == parser->current.type){
        if (j == capacity){
            capacity = GROW_CAPACITY(capacity);
            t = k(KS, capacity);
            for (uint64_t i = 0; i < rn; ++i) ti[i] = ri[i];
            unref(r);
            r = t;
        }
        advance(parser, scanner);
        str = &parser->previous.start[0];
        str++;
        MAKE_SYMBOL(ri[j], str, parser->previous.length-1);
        ++j;
    }
    rn = j;
    return r;
}

static K parseNoun(Parser *parser, Scanner *scanner){
    K r;

    // if number. eg 123 or 4.56
    if (TOKEN_NUMBER == parser->previous.type || TOKEN_FLOAT == parser->previous.type){
        r = parseNumber(parser, scanner);
    }
    // if string. eg "foobar"
    else if (TOKEN_STRING == parser->previous.type){
        r = parseString(parser, scanner);
    }
    // if parens. eg (1+2)%3 
    else if (TOKEN_LPAREN == parser->previous.type){
        r = parseParens(parser, scanner);
    }
    else if (TOKEN_SYMBOL == parser->previous.type){
        r = parseSymbol(parser, scanner);
    }
    else {
        REPORT_ERROR("Error! Noun not yet implemented.\n");
        r = KNUL;
    }

    return r;
}

static K parseMonad(Parser *parser, Scanner *scanner){
    K r = k(KM, 1);
    rc[0] = (char) parser->previous.type;
    return r; 
}

static K parseDyad(Parser *parser, Scanner *scanner){
    K r = k(KD, 1);
    rc[0] = (char) parser->previous.type;
    return r; 
}

// an arbitrary number of adverbs can be combined. eg:
// x g\:/: y
// f/''
// we use a while loop to parse adverbs
static K parseAdverb(Parser *parser, Scanner *scanner, K x){
    K adverb = k(KA, 1), r = k(KK, 2), t;
    CHAR(adverb)[0] = (char) parser->previous.type;
    rk[0] = adverb;
    rk[1] = x;
    while (atAdverb(parser->current.type)){
        advance(parser, scanner);
        adverb = k(KA, 1);
        CHAR(adverb)[0] = (char) parser->previous.type;
        t = k(KK, 2);
        tk[0] = adverb;
        tk[1] = r;
        r = t;
    }
    return r; 
}

// <expression>   ::=  <noun> <verb> <expression>  |  <term> <expr>  |  empty
// term is noun or verb
// verb is monadic/dyadic primitives (eg +-*%) or noun adverb
// adverb is a higher-order function that can be applied to verbs and nouns
// noun is data (arrays, literals, variables, functions)
static K expression(Scanner *scanner, Parser *parser){
    // this should only be true from a call from Expressions (for an expression ending with ;)
    if (TOKEN_EOF == parser->current.type){
        return KNUL;
    }
    advance(parser, scanner);
    K r;

    if (atNoun(parser->previous.type)){
        // expression - noun Adverb expersion  (noun Adverb == verb in k's grammar)
        //            | noun verb expression
        //            | noun expression
        //            | noun
        K prefix = parseNoun(parser, scanner);
        if (atExprEnd(parser->current.type))
            r = prefix;
        else if (atAdverb(parser->current.type)){
            advance(parser, scanner);
            prefix = parseAdverb(parser, scanner, prefix);
            if (TOKEN_EOF == parser->current.type) 
                return prefix;
            else {
                r = k(KK, 2);
                rk[0] = prefix;
                rk[1] = expression(scanner, parser);
            } 
        }
        else if (atVerb(parser->current.type)){
            advance(parser, scanner);
            K infix = parseDyad(parser, scanner);
            if (TOKEN_EOF == parser->current.type) // 1+
                r = join(infix, prefix);
            else if (atAdverb(parser->current.type)){
                advance(parser, scanner);
                infix = parseAdverb(parser, scanner, infix);
                if (TOKEN_EOF == parser->current.type) {
                    r = k(KK, 2);
                    rk[0] = infix;
                    rk[1] = prefix;
                    return r;
                }
                else {
                    r = k(KK, 3);
                    rk[0] = infix;
                    rk[1] = prefix;
                    rk[2] = expression(scanner, parser);
                } 
            }
            else { // 1+2
                r = k(KK, 3);
                rk[0] = infix;
                rk[1] = prefix;
                rk[2] = expression(scanner, parser);
            }
        }
        else { // noun expression
            r = k(KK, 2);
            rk[0] = prefix;
            rk[1] = expression(scanner, parser);
        }
    }
    else if (atVerb(parser->previous.type)){
        // expression - verb adverb expression 
        //            | verb expression
        //            | verb
        K prefix;
        if (atAdverb(parser->current.type)){
            prefix = parseDyad(parser, scanner);
            advance(parser, scanner);
            prefix = parseAdverb(parser, scanner, prefix);
            if (TOKEN_EOF == parser->current.type)
                r = prefix;
            else {
                r = k(KK, 2);
                rk[0] = prefix;
                rk[1] = expression(scanner, parser);
            }

        }
        else {
            prefix = parseMonad(parser, scanner);
        }
        if (atExprEnd(parser->current.type)) 
            return r;
        else { // verb expression
            r = k(KK, 2);
            rk[0] = prefix;
            rk[1] = expression(scanner, parser);
        }
    }
    else { // no input or error. return generic null
        parser->panic = true;

        REPORT_ERROR("Unexpected token '%.*s'\n.",parser->current.length,parser->current.start);
        return KNUL;
    }

    return r;
}

// <Expressions>  ::=  <Exprs> ";" <expression>  |  <expression>
static K Expressions(Scanner *scanner, Chunk *chunk, Parser *parser){
    // parse an expression
    K r = expression(scanner, parser);
    
    // parse semicolon-delimited expressions
    if (TOKEN_SEMICOLON == parser->current.type){
        K t = k(KK, 2);
        tk[0] = Kc(';');
        tk[1] = r;
        r = t;
        while (TOKEN_SEMICOLON == parser->current.type){
            advance(parser, scanner);
            t = expression(scanner, parser);
            r = append(r, t);
        }
    }
    else if (TOKEN_EOF  == parser->current.type)
        ; // do nothing
    else {
        // error. we shouldn't reach this branch
        REPORT_ERROR("Unexpected token '%.*s'\n.",parser->current.length,parser->current.start);
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
    parser.panic = false;

    advance(&parser, scanner);
    if (TOKEN_EOF == parser.current.type){
        chunk->parseTree = KNUL;
        return true;
    }

    K r = Expressions(scanner, chunk, &parser);
    chunk->parseTree = r;

    if (parser.panic){
        printf("Parse error.\n");
        freeChunk(chunk);
        free(scanner);
        chunk = NULL;
        return false;
    }

    free(scanner);
    return true;
}
