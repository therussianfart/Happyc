#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: hc <file.hc> [output]\n";
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = (argc >= 3) ? argv[2] : "output";

    std::string base = inputPath;
    if (base.size() > 3 && base.substr(base.size() - 3) == ".hc")
        base = base.substr(0, base.size() - 3);

    std::string cppPath = base + ".cpp";

    std::ifstream file(inputPath);
    if (!file) { std::cerr << "Cannot open: " << inputPath << "\n"; return 1; }
    std::ostringstream ss; ss << file.rdbuf();

    try {
        auto tokens = tokenize(ss.str());
        auto ast    = parse(std::move(tokens));
        generateCpp(ast, cppPath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::string cmd = "g++ -std=c++17 -o \"" + outputPath + "\" \"" + cppPath + "\"";
    std::cout << "Compiling: " << cmd << "\n";
    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "Compilation failed.\n";
        return 1;
    }

    std::cout << "Built: " << outputPath << "\n";
    return 0;
}
