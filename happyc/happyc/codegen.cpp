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
    if (dynamic_cast<UserInputNode*>(node))
        return "hc_input()";
    if (auto* bin = dynamic_cast<BinOpNode*>(node))
        return "hc_concat(" + emitExpr(bin->left.get()) + ", " + emitExpr(bin->right.get()) + ")";
    throw std::runtime_error("Unknown expression in codegen");
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
    if (auto* ifn = dynamic_cast<IfNode*>(node)) {
        out << pad << "if (hc_eq(" << emitExpr(ifn->lhs.get()) << ", " << emitExpr(ifn->rhs.get()) << ")) {\n";
        for (auto& s : ifn->body) emitStmt(s.get(), out, indent + 1);
        out << pad << "}\n";
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
    out << "#include <sstream>\n\n";

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
        if (dynamic_cast<FuncNode*>(node.get())) continue;
        emitStmt(node.get(), out, 0);
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
