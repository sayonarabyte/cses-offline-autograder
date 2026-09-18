#include "compiler.hpp"
#include <cstdlib>
#include <iostream>

namespace autograder {

bool Compiler::Compile(const std::string& source_file, const std::string& output_exe) {
    std::string command = "g++ -O2 -std=c++17 -Wall -Wextra " + source_file + " -o " + output_exe;
    int result = std::system(command.c_str());
    return result == 0;
}

} // namespace autograder
