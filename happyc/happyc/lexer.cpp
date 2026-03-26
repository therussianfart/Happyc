#include "lexer.h"
#include <stdexcept>
#include <cctype>

std::vector<Token> tokenize(const std::string& src) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < src.size()) {
        if (std::isspace(src[i])) { i++; continue; }

        // single-line comment
        if (src[i] == '/' && i+1 < src.size() && src[i+1] == '/') {
            while (i < src.size() && src[i] != '\n') i++;
            continue;
        }

        // string literal
        if (src[i] == '"') {
            std::string s;
            size_t start = i++;
            while (i < src.size() && src[i] != '"') s += src[i++];
            if (i >= src.size())
                throw std::runtime_error("Unterminated string literal at position " + std::to_string(start));
            i++;
            tokens.push_back({TokenType::STRING, s});
            continue;
        }

        // number
        if (std::isdigit(src[i])) {
            std::string n;
            bool hasDot = false;
            while (i < src.size() && (std::isdigit(src[i]) || src[i] == '.')) {
                if (src[i] == '.') {
                    if (hasDot) throw std::runtime_error("Malformed number: " + n + ".");
                    hasDot = true;
                }
                n += src[i++];
            }
            tokens.push_back({TokenType::NUMBER, n});
            continue;
        }

        // identifier or keyword
        if (std::isalpha(src[i]) || src[i] == '_') {
            std::string word;
            while (i < src.size() && (std::isalnum(src[i]) || src[i] == '_')) word += src[i++];
            if      (word == "let")        tokens.push_back({TokenType::LET, word});
            else if (word == "func")       tokens.push_back({TokenType::FUNC, word});
            else if (word == "if")         tokens.push_back({TokenType::IF, word});
            else if (word == "else")        tokens.push_back({TokenType::ELSE, word});
            else if (word == "while")       tokens.push_back({TokenType::WHILE, word});
            else if (word == "for")         tokens.push_back({TokenType::FOR, word});
            else if (word == "return")     tokens.push_back({TokenType::RETURN, word});
            else if (word == "print")      tokens.push_back({TokenType::PRINT, word});
            else if (word == "println")    tokens.push_back({TokenType::PRINTLN, word});
            else if (word == "User_input") tokens.push_back({TokenType::USER_INPUT, word});
            else if (word == "true" || word == "false") tokens.push_back({TokenType::BOOL_LIT, word});
            else tokens.push_back({TokenType::IDENT, word});
            continue;
        }

        switch (src[i]) {
            case '=':
                if (i+1 < src.size() && src[i+1] == '=') { tokens.push_back({TokenType::EQ, "=="}); i++; }
                else tokens.push_back({TokenType::ASSIGN, "="});
                break;
            case '&':
                if (i+1 < src.size() && src[i+1] == '&') { tokens.push_back({TokenType::AND, "&&"}); i++; }
                else throw std::runtime_error("Unknown character: &");
                break;
            case '|':
                if (i+1 < src.size() && src[i+1] == '|') { tokens.push_back({TokenType::OR, "||"}); i++; }
                else throw std::runtime_error("Unknown character: |");
                break;
            case '!':
                if (i+1 < src.size() && src[i+1] == '=') { tokens.push_back({TokenType::NEQ, "!="}); i++; }
                else tokens.push_back({TokenType::NOT, "!"});
                break;
            case '<':
                if (i+1 < src.size() && src[i+1] == '=') { tokens.push_back({TokenType::LTE, "<="}); i++; }
                else tokens.push_back({TokenType::LT, "<"});
                break;
            case '>':
                if (i+1 < src.size() && src[i+1] == '=') { tokens.push_back({TokenType::GTE, ">="}); i++; }
                else tokens.push_back({TokenType::GT, ">"});
                break;
            case '+': tokens.push_back({TokenType::PLUS, "+"}); break;
            case '-': tokens.push_back({TokenType::MINUS, "-"}); break;
            case '*': tokens.push_back({TokenType::STAR, "*"}); break;
            case '/': tokens.push_back({TokenType::SLASH, "/"}); break;
            case '(': tokens.push_back({TokenType::LPAREN, "("}); break;
            case ')': tokens.push_back({TokenType::RPAREN, ")"}); break;
            case '{': tokens.push_back({TokenType::LBRACE, "{"}); break;
            case '}': tokens.push_back({TokenType::RBRACE, "}"}); break;
            case '[': tokens.push_back({TokenType::LBRACKET, "["}); break;
            case ']': tokens.push_back({TokenType::RBRACKET, "]"}); break;
            case ';': tokens.push_back({TokenType::SEMICOLON, ";"}); break;
            case ',': tokens.push_back({TokenType::COMMA, ","}); break;
            case ':': tokens.push_back({TokenType::COLON, ":"}); break;
            default: throw std::runtime_error(std::string("Unknown character: ") + src[i]);
        }
        i++;
    }

    tokens.push_back({TokenType::END_OF_FILE, ""});
    return tokens;
}
