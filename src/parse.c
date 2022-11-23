#include "skrawl.h"
#include "parse.h"

#define EXPR_END() sc(";\n\0", *parser->current)

// foward declarations
static void Exprs(Parser *parser);

// consume whitepace
static inline void ws(Parser *p){ while(' '==*p->current) ++p->current; }

// return char class 
static char class(char c){
    //                       !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
    return (c > 126) ? 0 : "        ()      0000000000                                                                     "[c-32];
}

// parse number(s). eg "1" or "1 23"
static void parseNum(Parser *parser){
    do {
        while ('0'==class(*parser->current)) ++parser->current;
        ws(parser);
    } while ('0'==class(*parser->current));
}

// parse single expression
static void expr(Parser *parser){
    ws(parser);
    char a = *parser->current++;
    char c = class(a);

    switch (c){
    case '0': parseNum(parser); break;
    case '(': Exprs(parser); if(')'==(a=*parser->current++)){ break; } //else fallthrough
    default : printf("'error: %c\n", a);
    }

    ws(parser);
    if (EXPR_END()) return;
}

// parse ;-delimited Expressions
static void Exprs(Parser *parser){
    do expr(parser); while(';'==*parser->current++);
    --parser->current;
}

void parse(char *src){
    // init parser struct
    Parser parser;
    parser.src = src;
    parser.current = src;

    // parse Expressions
    Exprs(&parser);
}