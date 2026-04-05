#include "log_manager.h"

#include <chrono>
#include <deque>
#include <fstream>
#include <iomanip>
#include <sstream>


LogManager::LogManager( const std::filesystem::path& log_dir, size_t max_file_size, int max_files )
    : log_dir_(log_dir)
    , max_file_size_(max_file_size)
    , max_files_(max_files)
{
    std::filesystem::create_directories(log_dir_);
}

std::filesystem::path LogManager::log_path( const std::string& service_name ){
    return log_dir_ / (service_name + ".log");
}

void LogManager::write( const std::string& service_name, const std::string& output ){

    if( output.empty() ) return;

    rotate_if_needed(service_name);

    auto path = log_path(service_name);
    std::ofstream file( path, std::ios::app );
    if( !file ) return;

    // Timestamp each write
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    file << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
    file << output;

    // Ensure trailing newline
    if( !output.empty() && output.back() != '\n' ){
        file << "\n";
    }
}

std::string LogManager::read_log( const std::string& service_name, int num_lines ){

    auto path = log_path(service_name);
    if( !std::filesystem::exists(path) ){
        return "";
    }

    std::ifstream file(path);
    std::deque<std::string> lines;
    std::string line;

    while( std::getline(file, line) ){
        lines.push_back(line);
        if( static_cast<int>(lines.size()) > num_lines ){
            lines.pop_front();
        }
    }

    std::ostringstream result;
    for( const auto& l : lines ){
        result << l << "\n";
    }
    return result.str();
}

void LogManager::rotate_if_needed( const std::string& service_name ){

    auto path = log_path(service_name);
    if( !std::filesystem::exists(path) ) return;

    auto size = std::filesystem::file_size(path);
    if( size < max_file_size_ ) return;

    // Rotate: .log.4 -> delete, .log.3 -> .log.4, ... .log -> .log.1
    for( int i = max_files_ - 1; i >= 1; i-- ){
        auto src = log_dir_ / (service_name + ".log." + std::to_string(i));
        auto dst = log_dir_ / (service_name + ".log." + std::to_string(i + 1));
        if( std::filesystem::exists(src) ){
            std::filesystem::rename(src, dst);
        }
    }

    auto rotated = log_dir_ / (service_name + ".log.1");
    std::filesystem::rename(path, rotated);
}
