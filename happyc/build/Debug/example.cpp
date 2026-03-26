#include <iostream>
#include <string>
#include <sstream>

// HappyC runtime helpers
static std::string hc_input() {
    std::string s; std::getline(std::cin, s); return s;
}

template<typename A, typename B>
static std::string hc_concat(A a, B b) {
    std::ostringstream ss; ss << a << b; return ss.str();
}

template<typename A, typename B>
static bool hc_eq(A a, B b) { return a == b; }


auto greet();

auto greet() {
    return std::string("Hello World");
}

int main() {
    std::cout << greet() << "\n";
    std::cout.flush();
}

