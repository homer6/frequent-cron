#pragma once

#include <string>

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_data;
    std::string stderr_data;
};

// Run a shell command synchronously, capturing stdout and stderr.
ProcessResult run_process( const std::string& command );
