#include <exception>
#include <iostream>
#include <string>

#include "Scanner.h"


int main(int argc, char** argv) {
    const std::string dir = PROJECT_DIR;

    try {
        Scanner scanner(dir + "/tokens.txt", dir + "/fa");

        if (argc > 1) {
            for (int i = 1; i < argc; i++) scanner.scan(argv[i], dir + "/out");
        } else {
            scanner.scan(dir + "/examples/scarf.cml", dir + "/out");
            scanner.scan(dir + "/examples/blanket.cml", dir + "/out");
            scanner.scan(dir + "/examples/lexical-errors.cml", dir + "/out");
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
