#pragma once

#include <filesystem>
#include <string>

class LogManager {
public:
    LogManager( const std::filesystem::path& log_dir, size_t max_file_size = 10 * 1024 * 1024, int max_files = 5 );

    // Append output from a command execution to the service's log
    void write( const std::string& service_name, const std::string& output );

    // Read the last N lines from a service's log
    std::string read_log( const std::string& service_name, int num_lines = 100 );

    // Rotate log if it exceeds max size
    void rotate_if_needed( const std::string& service_name );

private:
    std::filesystem::path log_dir_;
    size_t max_file_size_;
    int max_files_;

    std::filesystem::path log_path( const std::string& service_name );
};
