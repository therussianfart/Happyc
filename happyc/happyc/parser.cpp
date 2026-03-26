#include "parser.h"
#include <stdexcept>

struct Parser {
    std::vector<Token> tokens;
    size_t pos = 0;

    Token& peek() { return tokens[pos]; }
    Token consume() { return tokens[pos++]; }

    static std::string typeName(TokenType t) {
        switch (t) {
            case TokenType::LET:        return "'let'";
            case TokenType::FUNC:       return "'func'";
            case TokenType::IF:         return "'if'";
            case TokenType::ELSE:       return "'else'";
            case TokenType::RETURN:     return "'return'";
            case TokenType::IDENT:      return "identifier";
            case TokenType::ASSIGN:     return "'='";
            case TokenType::SEMICOLON:  return "';'";
            case TokenType::LPAREN:     return "'('";
            case TokenType::RPAREN:     return "')'";
            case TokenType::LBRACE:     return "'{'";
            case TokenType::RBRACE:     return "'}'";
            case TokenType::COMMA:      return "','";
            case TokenType::END_OF_FILE: return "end of file";
            default: return "token";
        }
    }

    Token expect(TokenType t) {
        if (peek().type != t)
            throw std::runtime_error("Expected " + typeName(t) + " but got '" + peek().value + "'");
        return consume();
    }

    NodePtr parseExpr() {
        NodePtr left = parseTerm();
        while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS) {
            std::string op = consume().value;
            auto node = std::make_unique<BinOpNode>();
            node->op = op;
            node->left = std::move(left);
            node->right = parseTerm();
            left = std::move(node);
        }
        return left;
    }

    NodePtr parseTerm() {
        NodePtr left = parsePrimary();
        while (peek().type == TokenType::STAR || peek().type == TokenType::SLASH) {
            std::string op = consume().value;
            auto node = std::make_unique<BinOpNode>();
            node->op = op;
            node->left = std::move(left);
            node->right = parsePrimary();
            left = std::move(node);
        }
        return left;
    }

    NodePtr parsePrimary() {
        if (peek().type == TokenType::STRING) {
            auto n = std::make_unique<LiteralNode>();
            n->tokenType = TokenType::STRING;
            n->value = consume().value;
            return n;
        }
        if (peek().type == TokenType::NUMBER) {
            auto n = std::make_unique<LiteralNode>();
            n->tokenType = TokenType::NUMBER;
            n->value = consume().value;
            return n;
        }
        if (peek().type == TokenType::BOOL_LIT) {
            auto n = std::make_unique<LiteralNode>();
            n->tokenType = TokenType::BOOL_LIT;
            n->value = consume().value;
            return n;
        }
        if (peek().type == TokenType::USER_INPUT) {
            consume();
            expect(TokenType::LPAREN);
            expect(TokenType::RPAREN);
            return std::make_unique<UserInputNode>();
        }
        if (peek().type == TokenType::LBRACKET) {
            consume();
            auto node = std::make_unique<ArrayNode>();
            while (peek().type != TokenType::RBRACKET) {
                if (peek().type == TokenType::END_OF_FILE)
                    throw std::runtime_error("Unexpected end of file in array literal");
                node->elements.push_back(parseExpr());
                if (peek().type == TokenType::COMMA) consume();
            }
            expect(TokenType::RBRACKET);
            return node;
        }
        if (peek().type == TokenType::IDENT) {
            std::string name = consume().value;
            // index access: ident[expr]
            if (peek().type == TokenType::LBRACKET) {
                consume();
                auto node = std::make_unique<IndexNode>();
                node->name = name;
                node->index = parseExpr();
                expect(TokenType::RBRACKET);
                return node;
            }
            if (peek().type == TokenType::LPAREN) {
                consume();
                auto node = std::make_unique<CallNode>();
                node->name = name;
                while (peek().type != TokenType::RPAREN) {
                    if (peek().type == TokenType::END_OF_FILE)
                        throw std::runtime_error("Unexpected end of file in argument list");
                    node->args.push_back(parseExpr());
                    if (peek().type == TokenType::COMMA) consume();
                }
                expect(TokenType::RPAREN);
                return node;
            }
            auto n = std::make_unique<IdentNode>();
            n->name = name;
            return n;
        }
        throw std::runtime_error("Expected expression, got: '" + peek().value + "'");
    }

    std::unique_ptr<ConditionNode> parseCondition() {
        // parse one comparison, then chain with && or ||
        auto parseSingle = [&]() -> std::unique_ptr<ConditionNode> {
            bool isNot = false;
            if (peek().type == TokenType::NOT) { consume(); isNot = true; }
            auto cond = std::make_unique<ConditionNode>();
            cond->isNot = isNot;
            cond->lhs = parsePrimary();
            auto t = peek().type;
            if (t == TokenType::ASSIGN || t == TokenType::EQ || t == TokenType::NEQ ||
                t == TokenType::LT || t == TokenType::GT ||
                t == TokenType::LTE || t == TokenType::GTE) {
                cond->op = consume().value;
                cond->rhs = parsePrimary();
            } else {
                throw std::runtime_error("Expected comparison operator, got '" + peek().value + "'");
            }
            return cond;
        };

        auto left = parseSingle();

        while (peek().type == TokenType::AND || peek().type == TokenType::OR) {
            std::string op = consume().value;
            auto chain = std::make_unique<ConditionNode>();
            chain->op = op;
            // wrap left as a LiteralNode placeholder — store as lhs/rhs ConditionNodes
            // We'll use a trick: store left condition as lhs via a wrapper
            chain->lhs = std::move(left);
            chain->rhs = parseSingle();
            left = std::move(chain);
        }

        return left;
    }

    std::vector<NodePtr> parseBlock() {
        expect(TokenType::LBRACE);
        std::vector<NodePtr> stmts;
        while (true) {
            if (peek().type == TokenType::END_OF_FILE)
                throw std::runtime_error("Unexpected end of file: missing '}'");
            if (peek().type == TokenType::RBRACE) break;
            stmts.push_back(parseStatement());
        }
        expect(TokenType::RBRACE);
        return stmts;
    }

    NodePtr parseStatement() {
        if (peek().type == TokenType::LET) {
            consume();
            auto node = std::make_unique<LetNode>();
            node->name = expect(TokenType::IDENT).value;
            expect(TokenType::ASSIGN);
            node->value = parseExpr();
            expect(TokenType::SEMICOLON);
            return node;
        }
        if (peek().type == TokenType::RETURN) {
            consume();
            auto node = std::make_unique<ReturnNode>();
            if (peek().type != TokenType::SEMICOLON)
                node->value = parseExpr();
            expect(TokenType::SEMICOLON);
            return node;
        }
        if (peek().type == TokenType::PRINT || peek().type == TokenType::PRINTLN) {
            bool nl = peek().type == TokenType::PRINTLN;
            consume();
            expect(TokenType::LPAREN);
            auto node = std::make_unique<PrintNode>();
            node->expr = parseExpr();
            node->newline = nl;
            expect(TokenType::RPAREN);
            expect(TokenType::SEMICOLON);
            return node;
        }
        if (peek().type == TokenType::FOR) {
            consume();
            auto node = std::make_unique<ForNode>();
            // init: ident = expr
            node->initName = expect(TokenType::IDENT).value;
            expect(TokenType::ASSIGN);
            node->initValue = parseExpr();
            expect(TokenType::SEMICOLON);
            // condition
            node->cond = parseCondition();
            expect(TokenType::SEMICOLON);
            // step: ident = expr
            node->stepName = expect(TokenType::IDENT).value;
            expect(TokenType::ASSIGN);
            node->stepValue = parseExpr();
            node->body = parseBlock();
            return node;
        }
        if (peek().type == TokenType::WHILE) {
            consume();
            auto node = std::make_unique<WhileNode>();
            node->cond = parseCondition();
            node->body = parseBlock();
            return node;
        }
        if (peek().type == TokenType::IF) {
            consume();
            auto node = std::make_unique<IfNode>();
            node->cond = parseCondition();
            node->body = parseBlock();
            if (peek().type == TokenType::ELSE) {
                consume();
                node->elseBody = parseBlock();
            }
            return node;
        }
        if (peek().type == TokenType::FUNC) {
            consume();
            auto node = std::make_unique<FuncNode>();
            node->name = expect(TokenType::IDENT).value;
            expect(TokenType::LPAREN);
            while (peek().type != TokenType::RPAREN) {
                node->params.push_back(expect(TokenType::IDENT).value);
                if (peek().type == TokenType::COMMA) consume();
            }
            expect(TokenType::RPAREN);
            node->body = parseBlock();
            return node;
        }
        // index assignment: ident[expr] = expr;
        if (peek().type == TokenType::IDENT && pos + 1 < tokens.size() && tokens[pos + 1].type == TokenType::LBRACKET) {
            std::string name = consume().value;
            consume(); // [
            auto idx = parseExpr();
            expect(TokenType::RBRACKET);
            expect(TokenType::ASSIGN);
            auto val = parseExpr();
            expect(TokenType::SEMICOLON);
            auto node = std::make_unique<IndexAssignNode>();
            node->name = name;
            node->index = std::move(idx);
            node->value = std::move(val);
            return node;
        }
        // assignment: ident = expr;
        if (peek().type == TokenType::IDENT && pos + 1 < tokens.size() && tokens[pos + 1].type == TokenType::ASSIGN) {
            auto node = std::make_unique<AssignNode>();
            node->name = consume().value;
            consume(); // =
            node->value = parseExpr();
            expect(TokenType::SEMICOLON);
            return node;
        }
        // function call as statement: ident(...);
        if (peek().type == TokenType::IDENT && pos + 1 < tokens.size() && tokens[pos + 1].type == TokenType::LPAREN) {
            auto call = parsePrimary();
            expect(TokenType::SEMICOLON);
            return call;
        }
        throw std::runtime_error("Unknown statement: '" + peek().value + "'");
    }

    std::vector<NodePtr> parseAll() {
        std::vector<NodePtr> stmts;
        while (peek().type != TokenType::END_OF_FILE)
            stmts.push_back(parseStatement());
        return stmts;
    }
};

std::vector<NodePtr> parse(std::vector<Token> tokens) {
    Parser p;
    p.tokens = std::move(tokens);
    return p.parseAll();
}
