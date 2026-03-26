#pragma once
#include "lexer.h"
#include <memory>
#include <vector>

struct Node { virtual ~Node() = default; };
using NodePtr = std::unique_ptr<Node>;

struct LetNode : Node {
    std::string name;
    NodePtr value;
};

struct FuncNode : Node {
    std::string name;
    std::vector<std::string> params;
    std::vector<NodePtr> body;
};

struct IfNode : Node {
    NodePtr lhs;
    NodePtr rhs;
    std::vector<NodePtr> body;
};

struct PrintNode : Node {
    NodePtr expr;
    bool newline = true;
};

struct BinOpNode : Node {
    std::string op;
    NodePtr left, right;
};

struct LiteralNode : Node {
    std::string value;
    TokenType tokenType;
};

struct IdentNode : Node {
    std::string name;
};

struct CallNode : Node {
    std::string name;
    std::vector<NodePtr> args;
};

struct UserInputNode : Node {};

struct ReturnNode : Node {
    NodePtr value; // nullptr for bare return
};

std::vector<NodePtr> parse(std::vector<Token> tokens);
