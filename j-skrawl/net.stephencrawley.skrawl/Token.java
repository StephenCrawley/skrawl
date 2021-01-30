package net.stephencrawley.skrawl;

class Token 
{
    private TokenEnum type;
    private String lexeme;
    private Object literal; /* Object lets us declare a variable without a primitive type */
    private int line;

    Token(TokenEnum type, String lexeme, Object literal, int line)
    {
        this.type = type;
        this.lexeme = lexeme;
        this.literal = literal;
        this.line = line;
    }

    public void printToken()
    {
        System.out.println(type + ", " + lexeme + ", " + literal);
    }
}
