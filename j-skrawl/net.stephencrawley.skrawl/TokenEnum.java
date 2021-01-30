package net.stephencrawley.skrawl;

public enum TokenEnum
{
    // punctuators
    LEFT_PAREN, RIGHT_PAREN, LEFT_SQUARE, RIGHT_SQUARE,
    LEFT_BRACE, RIGHT_BRACE,

    // primitives  +-*%!  $^&@~  #',.<  >_?\/  |`=:;
    PLUS, MINUS, STAR, OBELUS, BANG, 
    DOLLAR, UPTICK, AMPERSAND, AT, TILDE, 
    HASH, APOSTROPHE, COMMA, DOT, LESS,
    GREATER, UNDERSCORE, QUESTION, BACKSLASH, FRONTSLASH,
    PIPE, BACKTICK, EQUAL, COLON, SEMICOLON,

    // composites(comparison) ==  !=  >=  <=
    EQUAL_EQUAL, BANG_EQUAL, GREATER_EQUAL, LESS_EQUAL,

    // composites(monads) @:  !:  $:  %:  ^:  &:  *:  ~:  ?:
    TYPE, TIL, STRING, SQRT, NULL, WHERE, FIRST, COUNT, NOT, RAND,

    // literals
    IDENTIFIER, FUNC, STR, SYM, LONG, FLOAT, BOOL,

    EOF
}
