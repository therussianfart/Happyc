#include "codegen.h"
#include <fstream>
#include <stdexcept>
#include <sstream>

static std::string emitExpr(Node* node);

static std::string emitExpr(Node* node) {
    if (auto* lit = dynamic_cast<LiteralNode*>(node)) {
        if (lit->tokenType == TokenType::STRING)
            return "std::string(\"" + lit->value + "\")";
        return lit->value;
    }
    if (auto* id = dynamic_cast<IdentNode*>(node))
        return id->name;
    if (auto* call = dynamic_cast<CallNode*>(node)) {
        std::string s = call->name + "(";
        for (size_t i = 0; i < call->args.size(); i++) {
            if (i) s += ", ";
            s += emitExpr(call->args[i].get());
        }
        return s + ")";
    }
    if (auto* arr = dynamic_cast<ArrayNode*>(node)) {
        std::string inner;
        for (size_t i = 0; i < arr->elements.size(); i++) {
            if (i) inner += ", ";
            inner += emitExpr(arr->elements[i].get());
        }
        return "std::vector<decltype(" + emitExpr(arr->elements[0].get()) + ")>{" + inner + "}";
    }
    if (auto* idx = dynamic_cast<IndexNode*>(node))
        return idx->name + "[" + emitExpr(idx->index.get()) + "]";
    if (dynamic_cast<UserInputNode*>(node))
        return "hc_input()";
    if (auto* bin = dynamic_cast<BinOpNode*>(node)) {
        return "(" + emitExpr(bin->left.get()) + " " + bin->op + " " + emitExpr(bin->right.get()) + ")";
    }
    throw std::runtime_error("Unknown expression in codegen");
}

static std::string emitCond(ConditionNode* cond) {
    // &&/|| chain — lhs and rhs are both ConditionNodes
    if (cond->op == "&&" || cond->op == "||") {
        auto* l = dynamic_cast<ConditionNode*>(cond->lhs.get());
        auto* r = dynamic_cast<ConditionNode*>(cond->rhs.get());
        return "(" + emitCond(l) + " " + cond->op + " " + emitCond(r) + ")";
    }
    // single comparison
    std::string op = (cond->op == "=") ? "==" : cond->op;
    std::string s = "(" + emitExpr(cond->lhs.get()) + " " + op + " " + emitExpr(cond->rhs.get()) + ")";
    if (cond->isNot) s = "!" + s;
    return s;
}

static bool hasReturn(const std::vector<NodePtr>& body) {
    for (auto& s : body)
        if (dynamic_cast<ReturnNode*>(s.get())) return true;
    return false;
}

static void emitStmt(Node* node, std::ostream& out, int indent) {
    std::string pad(indent * 4, ' ');

    if (auto* ret = dynamic_cast<ReturnNode*>(node)) {
        if (ret->value)
            out << pad << "return " << emitExpr(ret->value.get()) << ";\n";
        else
            out << pad << "return;\n";
        return;
    }
    if (auto* call = dynamic_cast<CallNode*>(node)) {
        out << pad << emitExpr(call) << ";\n";
        return;
    }
    if (auto* ia = dynamic_cast<IndexAssignNode*>(node)) {
        out << pad << ia->name << "[" << emitExpr(ia->index.get()) << "] = " << emitExpr(ia->value.get()) << ";\n";
        return;
    }
    if (auto* asgn = dynamic_cast<AssignNode*>(node)) {
        out << pad << asgn->name << " = " << emitExpr(asgn->value.get()) << ";\n";
        return;
    }
    if (auto* let = dynamic_cast<LetNode*>(node)) {
        out << pad << "auto " << let->name << " = " << emitExpr(let->value.get()) << ";\n";
        return;
    }
    if (auto* pr = dynamic_cast<PrintNode*>(node)) {
        out << pad << "std::cout << " << emitExpr(pr->expr.get());
        if (pr->newline) out << " << \"\\n\"";
        out << ";\n";
        out << pad << "std::cout.flush();\n";
        return;
    }
    if (auto* f = dynamic_cast<ForNode*>(node)) {
        std::string op = (f->cond->op == "=") ? "==" : f->cond->op;
        out << pad << "for (auto " << f->initName << " = " << emitExpr(f->initValue.get()) << "; "
            << emitCond(f->cond.get()) << "; "
            << f->stepName << " = " << emitExpr(f->stepValue.get()) << ") {\n";
        for (auto& s : f->body) emitStmt(s.get(), out, indent + 1);
        out << pad << "}\n";
        return;
    }
    if (auto* wh = dynamic_cast<WhileNode*>(node)) {
        out << pad << "while " << emitCond(wh->cond.get()) << " {\n";
        for (auto& s : wh->body) emitStmt(s.get(), out, indent + 1);
        out << pad << "}\n";
        return;
    }
    if (auto* ifn = dynamic_cast<IfNode*>(node)) {
        out << pad << "if " << emitCond(ifn->cond.get()) << " {\n";
        for (auto& s : ifn->body) emitStmt(s.get(), out, indent + 1);
        out << pad << "}";
        if (!ifn->elseBody.empty()) {
            out << " else {\n";
            for (auto& s : ifn->elseBody) emitStmt(s.get(), out, indent + 1);
            out << pad << "}";
        }
        out << "\n";
        return;
    }
    if (auto* fn = dynamic_cast<FuncNode*>(node)) {
        if (fn->name == "main") {
            out << "int main() {\n";
        } else {
            std::string retType = hasReturn(fn->body) ? "auto" : "void";
            out << retType << " " << fn->name << "(";
            for (size_t i = 0; i < fn->params.size(); i++) {
                if (i) out << ", ";
                out << "auto " << fn->params[i];
            }
            out << ") {\n";
        }
        for (auto& s : fn->body) emitStmt(s.get(), out, indent + 1);
        out << "}\n\n";
        return;
    }
    throw std::runtime_error("Unknown statement in codegen");
}

void generateCpp(const std::vector<NodePtr>& ast, const std::string& outPath) {
    std::ofstream out(outPath);
    if (!out) throw std::runtime_error("Cannot write to: " + outPath);

    out << "#include <iostream>\n";
    out << "#include <string>\n";
    out << "#include <sstream>\n";
    out << "#include <vector>\n\n";

    out << "static std::string hc_input() {\n";
    out << "    std::string s; std::getline(std::cin, s); return s;\n";
    out << "}\n\n";

    out << "template<typename A, typename B>\n";
    out << "static std::string hc_concat(A a, B b) {\n";
    out << "    std::ostringstream ss; ss << a << b; return ss.str();\n";
    out << "}\n\n";

    out << "template<typename A, typename B>\n";
    out << "static bool hc_eq(A a, B b) { return a == b; }\n\n";

    // top-level lets as globals
    for (auto& node : ast) {
        if (dynamic_cast<LetNode*>(node.get()))
            emitStmt(node.get(), out, 0);
        else if (!dynamic_cast<FuncNode*>(node.get()))
            throw std::runtime_error("Statements with blocks (if, while, etc.) must be inside a function");
    }
    out << "\n";

    // forward declare all non-main functions
    for (auto& node : ast) {
        if (auto* fn = dynamic_cast<FuncNode*>(node.get())) {
            if (fn->name == "main") continue;
            std::string retType = hasReturn(fn->body) ? "auto" : "void";
            out << retType << " " << fn->name << "(";
            for (size_t i = 0; i < fn->params.size(); i++) {
                if (i) out << ", ";
                out << "auto " << fn->params[i];
            }
            out << ");\n";
        }
    }
    out << "\n";

    // emit non-main functions first, then main
    for (auto& node : ast) {
        if (auto* fn = dynamic_cast<FuncNode*>(node.get()))
            if (fn->name != "main") emitStmt(node.get(), out, 0);
    }
    for (auto& node : ast) {
        if (auto* fn = dynamic_cast<FuncNode*>(node.get()))
            if (fn->name == "main") emitStmt(node.get(), out, 0);
    }
}
