#pragma once

#include <string>
#include <vector>

enum class Subcommand {
    RUN,
    INSTALL,
    REMOVE,
    START,
    STOP,
    STATUS,
    LIST,
    LOGS,
    LEGACY
};

struct Config {
    Subcommand subcommand = Subcommand::LEGACY;
    std::string service_name;
    int frequency = 0;
    std::string command;
    std::string pid_filename;
    bool synchronous = true;
    bool has_pid_file = false;
    std::string data_dir;
    int log_max_size_mb = 10;
    int log_max_files = 5;
    int jitter_ms = 0;
    std::string jitter_distribution = "uniform";  // "uniform" or "normal"
    double fire_probability = 1.0;
};

enum class ParseResult {
    OK,
    HELP,
    PARSE_ERROR
};

struct ParseOutput {
    ParseResult result;
    Config config;
    std::string message;
};

ParseOutput parse_args( int argc, char** argv );
ParseOutput parse_args( const std::vector<std::string>& args );
