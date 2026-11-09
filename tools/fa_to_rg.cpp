#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "usage: " << argv[0] << " file.fa\n";
        return 1;
    }
    ifstream in(argv[1]);
    if (!in) {
        cout << "cannot open " << argv[1] << "\n";
        return 1;
    }

    vector<string> states, finals;
    vector<string> from, symbol, to;
    string initial, line, word;

    while (getline(in, line)) {
        istringstream ss(line);
        if (!(ss >> word) || word[0] == '#') continue;
        if (word == "states:")       while (ss >> word) states.push_back(word);
        else if (word == "initial:") ss >> initial;
        else if (word == "final:")   while (ss >> word) finals.push_back(word);
        else {
            from.push_back(word);
            ss >> word; symbol.push_back(word);
            ss >> word; to.push_back(word);
        }
    }

    auto isFinal = [&](const string& s) {
        for (const string& f : finals) if (f == s) return true;
        return false;
    };

    cout << "N = {";
    for (size_t i = 0; i < states.size(); i++)
        cout << (i ? ", " : "") << states[i];

    cout << "}\nSigma = {";
    vector<string> seen;
    for (const string& a : symbol) {
        bool printed = false;
        for (const string& x : seen) if (x == a) printed = true;
        if (printed) continue;
        cout << (seen.empty() ? "" : ", ") << a;
        seen.push_back(a);
    }
    cout << "}\nS = " << initial << "\nP:\n";

    for (const string& q : states) {
        string rights;
        for (size_t i = 0; i < from.size(); i++) {
            if (from[i] != q) continue;
            rights += " | " + symbol[i] + " " + to[i];
            if (isFinal(to[i])) rights += " | " + symbol[i];
        }
        if (q == initial && isFinal(q)) rights += " | eps";
        if (!rights.empty())
            cout << "  " << q << " ->" << rights << "\n";
    }
}
