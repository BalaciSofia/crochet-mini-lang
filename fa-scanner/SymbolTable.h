#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <list>
#include <ostream>
#include <string>
#include <vector>


class SymbolTable {
public:
    static const int SIZE = 211;
    SymbolTable() : buckets(SIZE) {}
    int insert(const std::string& key, const std::string& type);
    int lookup(const std::string& key) const;
    int size() const { return count; }
    void print(std::ostream& out) const;
    static int hash(const std::string& key);

private:
    struct Entry {
        std::string key;
        std::string type;
        int position;
    };
    std::vector<std::list<Entry>> buckets;
    int count = 0;
};

#endif
