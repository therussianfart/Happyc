#include <iostream>
#include <string>
#include <sstream>
#include <vector>

static std::string hc_input() {
    std::string s; std::getline(std::cin, s); return s;
}

template<typename A, typename B>
static std::string hc_concat(A a, B b) {
    std::ostringstream ss; ss << a << b; return ss.str();
}

template<typename A, typename B>
static bool hc_eq(A a, B b) { return a == b; }



int main() {
    auto nums = std::vector<decltype(1)>{1, 2, 3};
    std::cout << nums[0] << "\n";
    std::cout.flush();
    nums[0] = 99;
    std::cout << nums[0] << "\n";
    std::cout.flush();
}

