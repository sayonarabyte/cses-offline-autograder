#pragma once
#include <string>

namespace autograder {

struct RunResult {
    bool success;
    std::string output;
    double time_taken_sec;
    bool time_limit_exceeded;
};

class Runner {
public:
    // Runs the executable, piping in the input file, and captures output.
    static RunResult Run(const std::string& executable_path, const std::string& input_file, double time_limit_sec = 1.0);
};

} // namespace autograder
