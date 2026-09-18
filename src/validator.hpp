#pragma once
#include <string>

namespace autograder {

class Validator {
public:
    // Compares actual output with expected output.
    // Ignores trailing whitespace on each line and empty lines at the end.
    static bool CompareOutput(const std::string& expected_file, const std::string& actual_output);
};

} // namespace autograder
