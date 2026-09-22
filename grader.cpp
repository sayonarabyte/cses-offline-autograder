#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>

using namespace std;

string escape_json(const string& s) {
    string res;
    for (char c : s) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else if (c >= 0 && c < 32) {
            // ignore non-printable
        } else {
            res += c;
        }
    }
    return res;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        cerr << "Usage: " << argv[0] << " <target_binary> <in_file> <out_file> <time_limit_sec> <mem_limit_mb>" << endl;
        return 1;
    }

    string target_binary = argv[1];
    string in_file = argv[2];
    string out_file = argv[3];
    double time_limit_sec = stod(argv[4]);
    double mem_limit_mb = stod(argv[5]);

    FILE* stdout_fp = tmpfile();
    FILE* stderr_fp = tmpfile();
    int stdout_fd = fileno(stdout_fp);
    int stderr_fd = fileno(stderr_fp);

    pid_t pid = fork();
    if (pid < 0) {
        cerr << "Fork failed" << endl;
        return 1;
    }

    if (pid == 0) {
        // Child
        int in_fd = open(in_file.c_str(), O_RDONLY);
        if (in_fd < 0) _exit(127);
        dup2(in_fd, STDIN_FILENO);
        dup2(stdout_fd, STDOUT_FILENO);
        dup2(stderr_fd, STDERR_FILENO);
        close(in_fd);

        struct rlimit rl_cpu;
        rl_cpu.rlim_cur = rl_cpu.rlim_max = (rlim_t)time_limit_sec + 1; // +1 to allow catching it
        setrlimit(RLIMIT_CPU, &rl_cpu);

        struct rlimit rl_fsize;
        rl_fsize.rlim_cur = rl_fsize.rlim_max = 5 * 1024 * 1024; // 5MB
        setrlimit(RLIMIT_FSIZE, &rl_fsize);

        char* args[] = {(char*)target_binary.c_str(), NULL};
        execv(target_binary.c_str(), args);
        _exit(127);
    }

    // Parent
    int status;
    struct rusage ru;
    wait4(pid, &status, 0, &ru);

    double time_seconds = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6 + 
                          ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6;
    long max_rss_kb = ru.ru_maxrss / 1024; // macOS ru_maxrss is in bytes

    string verdict;
    string error_message = "null";

    if (max_rss_kb > mem_limit_mb * 1024) {
        verdict = "MLE";
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        if (sig == SIGXCPU) verdict = "TLE";
        else if (sig == SIGXFSZ) verdict = "OLE";
        else {
            verdict = "RE";
            error_message = "\"Killed by signal " + to_string(sig) + "\"";
        }
    } else if (WIFEXITED(status)) {
        int ext = WEXITSTATUS(status);
        if (ext == 127) verdict = "RE";
        else if (ext != 0) {
            verdict = "RE";
            error_message = "\"Non-zero exit status " + to_string(ext) + "\"";
        }
    }

    if (verdict.empty()) {
        verdict = "AC"; // tentatively
    }

    // Capture stderr if RE
    if (verdict == "RE") {
        lseek(stderr_fd, 0, SEEK_SET);
        char buf[1024];
        ssize_t bytes = read(stderr_fd, buf, sizeof(buf) - 1);
        if (bytes > 0) {
            buf[bytes] = '\0';
            error_message = "\"" + escape_json(string(buf)) + "\"";
        }
    }

    // Diff output if AC
    if (verdict == "AC") {
        lseek(stdout_fd, 0, SEEK_SET);
        
        ifstream f_expected(out_file);
        
        FILE* read_stdout_fp = fdopen(dup(stdout_fd), "r");
        
        
        bool ok = true;
        string t1, t2;
        while (true) {
            char buf_exp[4096];
            char buf_out[4096];
            int r1 = fscanf(read_stdout_fp, "%4095s", buf_out);
            int r2 = f_expected >> t2 ? 1 : EOF;
            
            if (r1 == EOF && r2 == EOF) break;
            if (r1 == EOF || r2 == EOF) { ok = false; break; }
            if (string(buf_out) != t2) { ok = false; break; }
        }
        fclose(read_stdout_fp);
        
        if (!ok) verdict = "WA";
    }

    fclose(stdout_fp);
    fclose(stderr_fp);

    cout << "{"
         << "\"status\": \"" << verdict << "\", "
         << "\"time_seconds\": " << time_seconds << ", "
         << "\"max_rss_kb\": " << max_rss_kb << ", "
         << "\"error_message\": " << error_message
         << "}" << endl;

    return 0;
}
