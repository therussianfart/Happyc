#pragma once
#include <string>
#include <vector>

enum class TokenType {
    LET, FUNC, IF, ELSE, WHILE, FOR, RETURN, PRINT, PRINTLN, USER_INPUT,
    IDENT, NUMBER, STRING, BOOL_LIT,
    ASSIGN, PLUS, MINUS, STAR, SLASH, ARROW, COLON,
    EQ, NEQ, LT, GT, LTE, GTE,
    AND, OR, NOT,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    SEMICOLON, COMMA, END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
};

std::vector<Token> tokenize(const std::string& source);
