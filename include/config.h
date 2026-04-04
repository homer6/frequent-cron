#pragma once

#include <string>
#include <vector>

struct Config {
    int frequency = 0;
    std::string command;
    std::string pid_filename;
    bool synchronous = true;
    bool has_pid_file = false;
};

enum class ParseResult {
    OK,
    HELP,
    ERROR
};

struct ParseOutput {
    ParseResult result;
    Config config;
    std::string message;
};

ParseOutput parse_args( int argc, char** argv );
ParseOutput parse_args( const std::vector<std::string>& args );
