#include "Scanner.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

Scanner::Scanner(const std::string& tokensFile, const std::string& faFolder)
    : identifierFA(faFolder + "/identifier.fa"),
      intFA(faFolder + "/int.fa"),
      colorFA(faFolder + "/color.fa"),
      stringFA(faFolder + "/string.fa") {
    std::ifstream in(tokensFile);
    if (!in) throw std::runtime_error("cannot open " + tokensFile);
    std::string line, token;
    int code;
    while (std::getline(in, line)) {
        if (line.rfind("//", 0) == 0) continue;
        std::istringstream ss(line);
        if (ss >> token >> code) {
            codes[token] = code;
            names[code] = token;
        }
    }
}

void Scanner::scan(const std::string& programFile, const std::string& outFolder) {
    std::ifstream in(programFile);
    if (!in) throw std::runtime_error("cannot open " + programFile);
    std::stringstream buffer;
    buffer << in.rdbuf();

    text = buffer.str();
    pos = 0;
    line = 1;
    st = SymbolTable();
    pif.clear();
    errors.clear();

    while (true) {
        skipSpacesAndComments();
        if (pos >= text.size()) break;
        scanToken();
    }

    writeOutput(programFile, outFolder);
}

void Scanner::skipSpacesAndComments() {
    while (pos < text.size()) {
        if (text[pos] == '\n') {
            line++;
            pos++;
        } else if (std::isspace((unsigned char)text[pos])) {
            pos++;
        } else if (text.compare(pos, 2, "//") == 0) {
            while (pos < text.size() && text[pos] != '\n') pos++;
        } else {
            break;
        }
    }
}

void Scanner::scanToken() {
    char c = text[pos];

    // identifier or reserved word
    int len = identifierFA.longestMatch(text, pos);
    if (len > 0) {
        std::string word = text.substr(pos, len);
        if (codes.count(word)) addPif(codes[word], -1);
        else addPif(codes["identifier"], st.insert(word, "identifier"));
        pos += len;
        return;
    }

    // integer constant
    if (std::isdigit((unsigned char)c)) {
        len = intFA.longestMatch(text, pos);
        size_t end = wordEnd(pos);
        if (end == pos + len) { addConstant(len, "int constant"); return; }

        //the FA stopped before the end of the word
        bool hasLetter = false;
        for (size_t i = pos; i < end; i++)
            if (!std::isdigit((unsigned char)text[i])) hasLetter = true;
        addError(end, hasLetter ? "identifier cannot start with a digit" : "number with a leading zero");
        return;
    }

    //color constant
    if (c == '#') {
        len = colorFA.longestMatch(text, pos);
        size_t end = wordEnd(pos + 1);
        if (len > 0 && end == pos + len) { addConstant(len, "color constant"); return; }
        addError(end, "malformed color, expected # followed by exactly 6 hex digits");
        return;
    }

    //string constant
    if (c == '"') {
        len = stringFA.longestMatch(text, pos);
        if (len > 0) { addConstant(len, "string constant"); return; }

        size_t end = text.find('\n', pos);//missing end quote
        if (end == std::string::npos) end = text.size();
        if (end > pos && text[end - 1] == '\r') end--;
        addError(end, "unterminated string");
        return;
    }

    //operator or separator
    for (int n = 2; n >= 1; n--) {
        std::string op = text.substr(pos, n);
        if (codes.count(op)) {
            addPif(codes[op], -1);
            pos += n;
            return;
        }
    }

    //nothing matched
    addError(pos + 1, "unexpected character");
}

void Scanner::addPif(int code, int stPos) {
    pif.push_back({code, stPos, line});
}

void Scanner::addConstant(int length, const std::string& type) {
    addPif(codes["constant"], st.insert(text.substr(pos, length), type));
    pos += length;
}

void Scanner::addError(size_t end, const std::string& message) {
    std::string lexeme = text.substr(pos, end - pos);
    errors.push_back("Lexical error at line " + std::to_string(line) + ": " + message + ": \"" + lexeme + "\"");
    std::cerr << errors.back() << "\n";
    pos = end;
}

size_t Scanner::wordEnd(size_t from) const {
    while (from < text.size() && (std::isalnum((unsigned char)text[from]) || text[from] == '_')) from++;
    return from;
}

void Scanner::writeOutput(const std::string& programFile, const std::string& outFolder) const {
    namespace fs = std::filesystem;
    fs::create_directories(outFolder);
    std::string base = outFolder + "/" + fs::path(programFile).stem().string();

    std::ofstream pifOut(base + ".PIF.out");
    pifOut << std::left << std::setw(7) << "code" << std::setw(13) << "token" << std::setw(7) << "ST pos" << "line\n";
    for (const PifEntry& e : pif) {
        std::string stPos = e.stPos >= 0 ? std::to_string(e.stPos) : "-";
        pifOut << std::setw(7) << e.code << std::setw(13) << names.at(e.code) << std::setw(7) << stPos << e.line << "\n";
    }

    std::ofstream stOut(base + ".ST.out");
    st.print(stOut);

    std::ofstream errOut(base + ".errors.out");
    if (errors.empty()) errOut << "no lexical errors\n";
    for (const std::string& err : errors) errOut << err << "\n";

    if (errors.empty())
        std::cout << programFile << ": lexically correct (" << pif.size() << " PIF entries, " << st.size() << " ST entries)\n";
    else
        std::cout << programFile << ": " << errors.size() << " lexical error(s) found\n";
}
