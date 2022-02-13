#include <stdlib.h>
#include "a.h"
#include "token.h"

Scanner* initNewScanner(const char *source){
    Scanner *scanner = malloc(sizeof(Scanner));
    scanner->source = source;
    scanner->start = source;
    scanner->current = source;
    return scanner;
}

static Token makeToken(Scanner *scanner, TokenType type){
    Token token;
    token.start = scanner->start;
    token.length = (uint16_t)(scanner->current - scanner->start);
    token.type = type;

#ifdef DBG_TOKEN
    printf("length: %d   type: %-2d   token: %.*s \n", token.length, token.type, token.length, token.start);
#endif

    return token;
}

// TODO : add line info
// TODO : newlines to be treated as expression separators
static void skipWhitespace(Scanner *scanner){
    while ('\n' == *scanner->current || '\r' == *scanner->current || ' ' == *scanner->current) scanner->current++;
}

static bool isAlpha(char c){
    return ('A' <= c && 'Z' >= c) || ('a' <= c && 'z' >= c);
}

static bool isNumber(char c){
    return '0' <= c && '9' >= c;
}

static bool isOperator(char c){
    static const char ops[] = KOPS;
    for (int i = 0, n = sizeof(ops) / sizeof(char); i < n; ++i){
        if (c == ops[i]) return true;
    }
    return false;
}

static Token identifierToken(Scanner *scanner){
    while (isAlpha(*scanner->current) || isNumber(*scanner->current)){
        scanner->current++;
    }
    return makeToken(scanner, TOKEN_ID);
}

static Token stringToken(Scanner *scanner){
    while ('"' != *scanner->current++) 
        ;
    return makeToken(scanner, TOKEN_STRING);
}

static Token symbolToken(Scanner *scanner){
    while (isAlpha(*scanner->current) || isNumber(*scanner->current)){
        scanner->current++;
    }
    return makeToken(scanner, TOKEN_SYMBOL);
}

static Token numberToken(Scanner *scanner, uint16_t acc){
    while (isNumber(*scanner->current) || '.' == *scanner->current){
        if('.' == *scanner->current) acc++;
        scanner->current++;
    }
    return makeToken(scanner, 0 == acc ? TOKEN_NUMBER : 1 == acc ? TOKEN_FLOAT : TOKEN_ERROR);
}

static Token dotToken(Scanner *scanner){ 
    if (isNumber(*scanner->current)) return numberToken(scanner, 1);
    return makeToken(scanner, TOKEN_DOT);
}

// this function is used to decide whether '-' is a MINUS token or part of a negative number
// for example, k parses these 2 expressions differently depending on whitespace:
// 1)  -3+1 (parsed as -3 + 1. evaluates to minus -2)
// 2) - 3+1 (parsed as -(3+1), or "negate 3+1". evaluates to -4)
static Token minusToken(Scanner *scanner){
    if (isNumber(*scanner->current)){
        if (scanner->source == scanner->start)
            return numberToken(scanner, 0);
        else {
            if (' ' == scanner->start[-1] || isOperator(scanner->start[-1]))
                return numberToken(scanner, 0);
        }
    }
    
    return makeToken(scanner, TOKEN_MINUS);
}

// \ and / can mean scan or over respectively, or can start the \: and /: digraphs
// TODO? : monad digraphs
static Token digraphToken(Scanner *scanner){
    if (':' == *scanner->current){
        scanner->current++;
        return makeToken(scanner, ('/' == *scanner->start) ? TOKEN_EACHR : ('\\' == *scanner->start) ? TOKEN_EACHL : TOKEN_EACHPRIOR);
    }

    return makeToken(scanner, ('/' == *scanner->start) ? TOKEN_FSLASH : ('\\' == *scanner->start) ? TOKEN_BSLASH : TOKEN_APOSTROPHE);
}

Token nextToken(Scanner *scanner){
    skipWhitespace(scanner);
    scanner->start = scanner->current;

    char c = *scanner->current++;
    if (isAlpha(c))  return identifierToken(scanner);
    if (isNumber(c)) return numberToken(scanner, 0);
    if ('"' == c)    return stringToken(scanner);
    if ('`' == c)    return symbolToken(scanner);
    
    switch(c){
        case '+' : return makeToken(scanner, TOKEN_PLUS);
        case '*' : return makeToken(scanner, TOKEN_STAR);
        case '-' : return minusToken(scanner);
        case '%' : return makeToken(scanner, TOKEN_DIVIDE);
        case '!' : return makeToken(scanner, TOKEN_BANG);
        case '&' : return makeToken(scanner, TOKEN_AND);
        case '|' : return makeToken(scanner, TOKEN_PIPE);
        case ',' : return makeToken(scanner, TOKEN_COMMA);
        case '@' : return makeToken(scanner, TOKEN_AT);
        case '.' : return  dotToken(scanner);
        case '#' : return makeToken(scanner, TOKEN_HASH);
        case '^' : return makeToken(scanner, TOKEN_CARET);
        case '$' : return makeToken(scanner, TOKEN_DOLLAR);
        case '~' : return makeToken(scanner, TOKEN_TILDE);
        case '=' : return makeToken(scanner, TOKEN_EQUAL);
        case '<' : return makeToken(scanner, TOKEN_LANGLE);
        case '>' : return makeToken(scanner, TOKEN_RANGLE);
        case '?' : return makeToken(scanner, TOKEN_QMARK);
        case '_' : return makeToken(scanner, TOKEN_USCORE);
        case ':' : return makeToken(scanner, TOKEN_COLON);
        case '/' : return digraphToken(scanner);
        case '\\': return digraphToken(scanner);
        case '\'': return digraphToken(scanner);
        case '(' : return makeToken(scanner, TOKEN_LPAREN);
        case ')' : return makeToken(scanner, TOKEN_RPAREN);
        case ';' : return makeToken(scanner, TOKEN_SEMICOLON);
        case '\0': return makeToken(scanner, TOKEN_EOF);
        default  : return makeToken(scanner, TOKEN_UNKNOWN);
    }
}
