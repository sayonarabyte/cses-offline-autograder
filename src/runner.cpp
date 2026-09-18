#include "runner.hpp"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace autograder {

RunResult Runner::Run(const std::string& executable_path, const std::string& input_file, double time_limit_sec) {
    RunResult result;
    result.success = false;
    result.time_limit_exceeded = false;

    std::string temp_out_file = "temp_runner_out.txt";
    // Simple execution using shell redirection.
    // For a robust implementation, fork/exec with pipes and alarm() should be used.
    std::string command = executable_path + " < " + input_file + " > " + temp_out_file;

    auto start_time = std::chrono::high_resolution_clock::now();
    int sys_ret = std::system(command.c_str());
    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = end_time - start_time;
    result.time_taken_sec = diff.count();

    if (result.time_taken_sec > time_limit_sec) {
        result.time_limit_exceeded = true;
        // The process might have finished but took too long.
        // True hard timeout requires killing the child process.
    }

    if (sys_ret == 0) {
        result.success = true;
    }

    std::ifstream out_fs(temp_out_file);
    if (out_fs.is_open()) {
        std::ostringstream ss;
        ss << out_fs.rdbuf();
        result.output = ss.str();
    }

    std::remove(temp_out_file.c_str());

    return result;
}

} // namespace autograder
