#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <functional>

#include <boost/asio.hpp>

class Executor {
public:
    Executor( const std::string& command, int frequency_ms, bool synchronous );

    void run();
    void stop();

    // Exposed for testing
    int execution_count() const { return execution_count_; }

private:
    void register_callback();
    void timer_callback( const boost::system::error_code& error );
    void execute_sync();
    void execute_async();

    std::string command_;
    std::chrono::milliseconds frequency_;
    bool synchronous_;
    int execution_count_ = 0;

    std::shared_ptr<boost::asio::io_context> io_context_;
    std::shared_ptr<boost::asio::steady_timer> timer_;
};
