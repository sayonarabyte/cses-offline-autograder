#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "compiler.hpp"
#include "runner.hpp"
#include "validator.hpp"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file.cpp> [test_dir]\n";
        return 1;
    }

    std::string source_file = argv[1];
    std::string test_dir = "tests"; // default
    if (argc >= 3) {
        test_dir = argv[2];
    }

    std::string executable = "./solution_exe";

    std::cout << "Compiling " << source_file << "...\n";
    if (!autograder::Compiler::Compile(source_file, executable)) {
        std::cerr << "Compilation failed!\n";
        return 1;
    }
    std::cout << "Compilation successful.\n";

    if (!fs::exists(test_dir) || !fs::is_directory(test_dir)) {
        std::cerr << "Test directory not found: " << test_dir << "\n";
        return 1;
    }

    int passed = 0;
    int total = 0;

    // We assume test cases are named 1.in, 1.out, 2.in, 2.out, etc.
    std::vector<std::string> test_inputs;
    for (const auto& entry : fs::directory_iterator(test_dir)) {
        if (entry.path().extension() == ".in") {
            test_inputs.push_back(entry.path().string());
        }
    }

    std::sort(test_inputs.begin(), test_inputs.end());

    for (const auto& in_file : test_inputs) {
        std::string base = in_file.substr(0, in_file.find_last_of('.'));
        std::string out_file = base + ".out";

        if (!fs::exists(out_file)) {
            std::cerr << "Warning: No matching .out file for " << in_file << "\n";
            continue;
        }

        total++;
        std::cout << "Running test " << in_file << "... ";
        
        auto result = autograder::Runner::Run(executable, in_file);
        
        if (result.time_limit_exceeded) {
            std::cout << "[TLE] (" << result.time_taken_sec << "s)\n";
        } else if (!result.success) {
            std::cout << "[RTE] Runtime Error\n";
        } else {
            if (autograder::Validator::CompareOutput(out_file, result.output)) {
                std::cout << "[PASS] (" << result.time_taken_sec << "s)\n";
                passed++;
            } else {
                std::cout << "[FAIL] Output mismatch\n";
                // In a full version, we might print a diff here
            }
        }
    }

    std::cout << "---------------------------------\n";
    std::cout << "Result: " << passed << "/" << total << " Passed\n";

    std::remove(executable.c_str());
    return passed == total ? 0 : 1;
}
