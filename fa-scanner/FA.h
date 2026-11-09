#ifndef FA_H
#define FA_H

#include <string>
#include <vector>

struct Transition {
    std::string from;
    std::string symbol;
    std::string to;
};

class FA {
public:
    explicit FA(const std::string& fileName);
    int longestMatch(const std::string& text, size_t start) const;

private:
    std::vector<std::string> states;
    std::string initial;
    std::vector<std::string> finals;
    std::vector<Transition> transitions;

    bool isFinal(const std::string& state) const;
    static bool inClass(char c, const std::string& cls);
};

#endif
