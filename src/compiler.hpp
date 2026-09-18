#pragma once
#include <string>

namespace autograder {

class Compiler {
public:
    // Compiles the given C++ source file into an executable.
    // Returns true if successful, false otherwise.
    static bool Compile(const std::string& source_file, const std::string& output_exe);
};

} // namespace autograder
