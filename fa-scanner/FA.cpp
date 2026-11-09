#include "FA.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

FA::FA(const std::string& fileName) {
    std::ifstream in(fileName);
    if (!in) throw std::runtime_error("cannot open " + fileName);

    std::string line, word;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        if (!(ss >> word)) continue;
        if (word == "states:")       { while (ss >> word) states.push_back(word); }
        else if (word == "initial:") { ss >> initial; }
        else if (word == "final:")   { while (ss >> word) finals.push_back(word); }
        else {
            Transition t;
            t.from = word;
            ss >> t.symbol >> t.to;
            transitions.push_back(t);
        }
    }
}

bool FA::isFinal(const std::string& state) const {
    for (const std::string& f : finals)
        if (f == state) return true;
    return false;
}

bool FA::inClass(char c, const std::string& cls) {
    unsigned char u = c;
    if (cls == "letter")  return std::isalpha(u);
    if (cls == "digit")   return std::isdigit(u);
    if (cls == "zero")    return c == '0';
    if (cls == "nonzero") return c >= '1' && c <= '9';
    if (cls == "hex")     return std::isxdigit(u);
    if (cls == "hash")    return c == '#';
    if (cls == "quote")   return c == '"';
    if (cls == "strchar") return c != '"' && c != '\n' && c != '\r';
    return false;
}

int FA::longestMatch(const std::string& text, size_t start) const {
    std::string state = initial;
    int best = 0;
    for (size_t i = start; i < text.size(); i++) {
        const Transition* next = nullptr;
        for (const Transition& t : transitions)
            if (t.from == state && inClass(text[i], t.symbol)) { next = &t; break; }
        if (next == nullptr) break;
        state = next->to;
        if (isFinal(state)) best = (int)(i - start + 1);
    }
    return best;
}
