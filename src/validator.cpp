#include "validator.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

namespace autograder {

static std::string trimRight(const std::string& s) {
    auto end = std::find_if_not(s.rbegin(), s.rend(), [](int c) {
        return std::isspace(c);
    });
    return std::string(s.begin(), end.base());
}

static std::vector<std::string> splitLines(std::istream& is) {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(is, line)) {
        lines.push_back(trimRight(line));
    }
    // Remove trailing empty lines
    while (!lines.empty() && lines.back().empty()) {
        lines.pop_back();
    }
    return lines;
}

bool Validator::CompareOutput(const std::string& expected_file, const std::string& actual_output) {
    std::ifstream expected_fs(expected_file);
    if (!expected_fs.is_open()) return false;

    std::vector<std::string> expected_lines = splitLines(expected_fs);
    
    std::istringstream actual_is(actual_output);
    std::vector<std::string> actual_lines = splitLines(actual_is);

    if (expected_lines.size() != actual_lines.size()) return false;

    for (size_t i = 0; i < expected_lines.size(); ++i) {
        if (expected_lines[i] != actual_lines[i]) return false;
    }

    return true;
}

} // namespace autograder
