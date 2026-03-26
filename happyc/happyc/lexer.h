#pragma once
#include <string>
#include <vector>

enum class TokenType {
    LET, FUNC, IF, RETURN, PRINT, PRINTLN, USER_INPUT,
    IDENT, NUMBER, STRING, BOOL_LIT,
    ASSIGN, PLUS, ARROW, COLON,
    LPAREN, RPAREN, LBRACE, RBRACE,
    SEMICOLON, COMMA, END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
};

std::vector<Token> tokenize(const std::string& source);
