#ifndef SCANNER_H
#define SCANNER_H

#include <map>
#include <string>
#include <vector>

#include "FA.h"
#include "SymbolTable.h"


struct PifEntry {
    int code;
    int stPos;
    int line;
};

class Scanner {
public:
    Scanner(const std::string& tokensFile, const std::string& faFolder);
    void scan(const std::string& programFile, const std::string& outFolder);

private:
    std::map<std::string, int> codes;
    std::map<int, std::string> names;

    FA identifierFA, intFA, colorFA, stringFA;

    std::string text;
    size_t pos = 0;
    int line = 1;
    SymbolTable st;
    std::vector<PifEntry> pif;
    std::vector<std::string> errors;

    void skipSpacesAndComments();
    void scanToken();

    void addPif(int code, int stPos);
    void addConstant(int length, const std::string& type);
    void addError(size_t end, const std::string& message);
    size_t wordEnd(size_t from) const;
    void writeOutput(const std::string& programFile, const std::string& outFolder) const;
};

#endif
