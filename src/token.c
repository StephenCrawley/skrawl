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
    // TODO : add TOKEN debug printing
    //printf("token\nstart: %c\nlength: %d\ntype: %d\n\n", *scanner->start, token.length, token.type); //debug
    return token;
}

// TODO : add line info
static void skipWhitespace(Scanner *scanner){
    while ('\n' == *scanner->current || '\r' == *scanner->current || ' ' == *scanner->current) scanner->current++;
}

static bool isAlpha(char c){
    return ('A' <= c && 'Z' >= c) || ('a' <= c && 'z' >= c);
}

static bool isNumber(char c){
    return '0' <= c && '9' >= c;
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
    if (isNumber(*scanner->current))
        if (scanner->start != scanner->source)
            if (' ' != scanner->start[-1])
                return makeToken(scanner, TOKEN_MINUS);
    
    return numberToken(scanner, 0);
}

Token nextToken(Scanner *scanner){
    skipWhitespace(scanner);
    scanner->start = scanner->current;

    char c = *scanner->current++;
    if (isAlpha(c))  return identifierToken(scanner);
    if (isNumber(c)) return numberToken(scanner, 0);
    if ('"' == c)    return stringToken(scanner);
    
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
        case '/' : return makeToken(scanner, TOKEN_FSLASH);
        case '\\': return makeToken(scanner, TOKEN_BSLASH);
        case '\'': return makeToken(scanner, TOKEN_APOSTROPHE);
        case '(' : return makeToken(scanner, TOKEN_LPAREN);
        case ')' : return makeToken(scanner, TOKEN_RPAREN);
        case ';' : return makeToken(scanner, TOKEN_SEMICOLON);
        case '\0': return makeToken(scanner, TOKEN_EOF);
        default  : return makeToken(scanner, TOKEN_UNKNOWN);
    }
}
