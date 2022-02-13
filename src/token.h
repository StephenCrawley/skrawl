#ifndef TOKEN_H
#define TOKEN_H

#include "a.h"

typedef enum {
    // operators
    TOKEN_PLUS, TOKEN_STAR, TOKEN_MINUS, TOKEN_DIVIDE, TOKEN_DOT, TOKEN_BANG,
    TOKEN_PIPE, TOKEN_AND, TOKEN_LANGLE, TOKEN_RANGLE, TOKEN_EQUAL, TOKEN_TILDE, TOKEN_QMARK, 
    TOKEN_COMMA, TOKEN_AT, TOKEN_HASH, TOKEN_CARET, TOKEN_DOLLAR, TOKEN_USCORE, TOKEN_COLON,
    // adverbs
    TOKEN_FSLASH, TOKEN_BSLASH, TOKEN_APOSTROPHE, TOKEN_EACHL, TOKEN_EACHR, TOKEN_EACHPRIOR,
    // identifier
    TOKEN_ID,
    // literals
    TOKEN_NUMBER, TOKEN_FLOAT, TOKEN_STRING, TOKEN_SYMBOL,
    // punctuation
    TOKEN_SEMICOLON, TOKEN_LPAREN, TOKEN_RPAREN,
    // end and errors
    TOKEN_EOF, TOKEN_ERROR, TOKEN_UNKNOWN
} TokenType;

// TODO: add line info
typedef struct {
    const char *start;
    uint16_t   length;
    TokenType  type;
} Token;

typedef struct {
    const char *source;
    const char *start;
    const char *current;
} Scanner;

// public function declarations
Token nextToken(Scanner *scanner);
Scanner* initNewScanner(const char *source);

#endif
