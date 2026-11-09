#include "SymbolTable.h"

#include <iomanip>

int SymbolTable::hash(const std::string& key) {
    int sum = 0;
    for (unsigned char c : key) sum += c;
    return sum % SIZE;
}

int SymbolTable::lookup(const std::string& key) const {
    for (const Entry& e : buckets[hash(key)])
        if (e.key == key) return e.position;
    return -1;
}

int SymbolTable::insert(const std::string& key, const std::string& type) {
    int pos = lookup(key);
    if (pos != -1) return pos;
    buckets[hash(key)].push_front({key, type, count});
    return count++;
}

void SymbolTable::print(std::ostream& out) const {
    std::vector<const Entry*> byPos(count);
    for (const auto& chain : buckets)
        for (const Entry& e : chain) byPos[e.position] = &e;
    out << "Symbol table (hash table, " << SIZE << " buckets, " << count << " entries)\n";
    out << std::left << std::setw(10) << "position" << std::setw(9) << "bucket"
        << std::setw(17) << "type" << "lexeme\n";
    for (const Entry* e : byPos)
        out << std::setw(10) << e->position << std::setw(9) << hash(e->key)
            << std::setw(17) << e->type << e->key << "\n";
}
