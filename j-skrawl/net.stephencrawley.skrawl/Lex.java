package net.stephencrawley.skrawl;

import java.util.ArrayList;
import java.util.List;

import jdk.javadoc.internal.tool.ToolOption;

public class Lex 
{
    private final String source;
    private final List<Token> tokens = new ArrayList<>();
    private int start = 0;
    private int current = 0;  /* current index of char */
    private int line = 1;     /* line num is 1-indexed */

    public Lex(String source)
    {
        this.source = source;
    }

    // loop over the source 
    public List<Token> Tokenize()
    {
        while (!atEnd())
        {
            start = current;
            tokenize();
        }

        tokens.add(new Token(TokenEnum.EOF, "", null, line));
        return tokens;
    }

    private void tokenize()
    {
        char c = advance();
        switch(c)
        {
            // see TokenEnum.java
            // ()[]{}  +-*%!  $^&@~  #',.<  >_?\/  |`=:;
            case '(': addToken(TokenEnum.LEFT_PAREN);   break;
            case ')': addToken(TokenEnum.RIGHT_PAREN);  break;
            case '[': addToken(TokenEnum.LEFT_SQUARE);  break;
            case ']': addToken(TokenEnum.RIGHT_SQUARE); break;
            case '{': addToken(TokenEnum.LEFT_BRACE);   break;
            case '}': addToken(TokenEnum.RIGHT_BRACE);  break;
            case ':': addToken(TokenEnum.COLON);        break;
            case ';': addToken(TokenEnum.SEMICOLON);    break;
            case '~': addToken(TokenEnum.TILDE);        break;
            case '+': addToken(TokenEnum.PLUS);         break;
            case '-': addToken(TokenEnum.MINUS);        break;
            // comparison operators //
            case '=': addToken(match('=') ? TokenEnum.EQUAL_EQUAL   : TokenEnum.EQUAL); break;
            case '>': addToken(match('=') ? TokenEnum.GREATER_EQUAL : TokenEnum.EQUAL); break;
            case '<': addToken(match('=') ? TokenEnum.LESS_EQUAL    : TokenEnum.EQUAL); break;
            // monads //
            case '@': addToken(match(':') ? TokenEnum.TYPE   : TokenEnum.AT);        break;
            case '$': addToken(match(':') ? TokenEnum.STRING : TokenEnum.DOLLAR);    break;
            case '%': addToken(match(':') ? TokenEnum.SQRT   : TokenEnum.OBELUS);    break;
            case '^': addToken(match(':') ? TokenEnum.NULL   : TokenEnum.UPTICK);    break;
            case '&': addToken(match(':') ? TokenEnum.WHERE  : TokenEnum.AMPERSAND); break;
            case '*': addToken(match(':') ? TokenEnum.FIRST  : TokenEnum.STAR);      break;
            //case '~': addToken(match(':') ? TokenEnum.NOT    : TokenEnum.TILDE);     break;
            case '?': addToken(match(':') ? TokenEnum.RAND   : TokenEnum.QUESTION);  break;

            // comparison or monad //
            // !=  !:  !
            case '!': 
                if (match('=')) 
                {
                    addToken(TokenEnum.BANG_EQUAL); 
                    break;
                }
                else if (match(':'))
                {
                    addToken(TokenEnum.TIL);
                    break;
                }
                else
                {
                    addToken(TokenEnum.BANG);
                    break;
                }

            // if newline increment line and break
            case '\n': line++; break;

            default:
                System.out.println("Line " + line + ", Unexpected character: " + c);
        }
    }

    private char advance()
    {
        // increment the index of the current character
        current++;
        // return the character before new current
        return source.charAt(current - 1);
    }

    // does the ingested character match
    private boolean match(char expected)
    {
        if (atEnd()) return false;
        if (peek() != expected) return false;

        current++;
        return true;
    }

    // return the character under the 'current' index
    private char peek()
    {
        return source.charAt(current);
    }

    // are we at the end of the source string?
    private boolean atEnd()
    {
        return current >= source.length();
    }

    // add a Token object to the tokens list
    private void addToken(TokenEnum t)
    {
        String s = source.substring(start, current);
        tokens.add(new Token(t, s, null, line));
    }
}
