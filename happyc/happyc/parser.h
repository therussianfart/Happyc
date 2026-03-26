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

struct AssignNode : Node {
    std::string name;
    NodePtr value;
};

struct FuncNode : Node {
    std::string name;
    std::vector<std::string> params;
    std::vector<NodePtr> body;
};

struct ConditionNode : Node {
    NodePtr lhs;
    std::string op; // =, ==, !=, <, >, <=, >=, &&, ||
    NodePtr rhs;
    bool isNot = false; // for ! prefix
};

struct IfNode : Node {
    std::unique_ptr<ConditionNode> cond;
    std::vector<NodePtr> body;
    std::vector<NodePtr> elseBody;
};

struct WhileNode : Node {
    std::unique_ptr<ConditionNode> cond;
    std::vector<NodePtr> body;
};

struct ForNode : Node {
    std::string initName;  // variable name
    NodePtr initValue;     // initial value
    std::unique_ptr<ConditionNode> cond;
    std::string stepName;  // variable to update
    NodePtr stepValue;     // update expression
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

struct ArrayNode : Node {
    std::vector<NodePtr> elements;
};

struct IndexNode : Node {
    std::string name;
    NodePtr index;
};

struct IndexAssignNode : Node {
    std::string name;
    NodePtr index;
    NodePtr value;
};

struct ReturnNode : Node {
    NodePtr value;
};

std::vector<NodePtr> parse(std::vector<Token> tokens);
