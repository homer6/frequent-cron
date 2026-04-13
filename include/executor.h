#pragma once

#ifdef _WIN32
    #include <winsock2.h>    // must come before <windows.h> to prevent WinSock 1 conflict
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <process.h>     // _beginthreadex, required by Boost ASIO's win_thread
#endif

#include <string>
#include <chrono>
#include <memory>
#include <functional>
#include <random>

#include <boost/asio.hpp>

using OutputCallback = std::function<void(const std::string& output)>;

class Executor {
public:
    Executor( const std::string& command, int frequency_ms, bool synchronous,
              int jitter_ms = 0, const std::string& jitter_distribution = "uniform",
              double fire_probability = 1.0 );

    void run();
    void stop();

    // Set a callback to receive command output (for log capture).
    // When set, uses run_process() instead of system().
    void set_output_callback( OutputCallback callback );

    // Exposed for testing
    int execution_count() const { return execution_count_; }
    int skip_count() const { return skip_count_; }

private:
    void register_callback();
    void timer_callback( const boost::system::error_code& error );
    void execute_sync();
    void execute_async();
    std::chrono::milliseconds compute_next_delay();
    bool should_fire();

    std::string command_;
    std::chrono::milliseconds frequency_;
    bool synchronous_;
    int jitter_ms_;
    std::string jitter_distribution_;
    double fire_probability_;
    int execution_count_ = 0;
    int skip_count_ = 0;
    OutputCallback output_callback_;

    std::mt19937 rng_;

    std::shared_ptr<boost::asio::io_context> io_context_;
    std::shared_ptr<boost::asio::steady_timer> timer_;
};
