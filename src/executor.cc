#include "executor.h"
#include "process_runner.h"

#include <iostream>
#include <cstdlib>
#include <cmath>

#ifndef _WIN32
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

Executor::Executor( const std::string& command, int frequency_ms, bool synchronous,
                    int jitter_ms, const std::string& jitter_distribution,
                    double fire_probability )
    : command_(command)
    , frequency_(frequency_ms)
    , synchronous_(synchronous)
    , jitter_ms_(jitter_ms)
    , jitter_distribution_(jitter_distribution)
    , fire_probability_(fire_probability)
    , rng_(std::random_device{}())
    , io_context_(std::make_shared<boost::asio::io_context>())
    , timer_(std::make_shared<boost::asio::steady_timer>(*io_context_, frequency_))
{
}

void Executor::run(){
    register_callback();
    io_context_->run();
}

void Executor::stop(){
    io_context_->stop();
}

void Executor::set_output_callback( OutputCallback callback ){
    output_callback_ = std::move(callback);
}

std::chrono::milliseconds Executor::compute_next_delay(){

    if( jitter_ms_ <= 0 ){
        return frequency_;
    }

    int offset = 0;
    if( jitter_distribution_ == "normal" ){
        std::normal_distribution<double> dist(0.0, static_cast<double>(jitter_ms_));
        offset = static_cast<int>(std::round(dist(rng_)));
        // Clamp to [-jitter_ms_, +jitter_ms_] (3-sigma can exceed range)
        offset = std::max(-jitter_ms_, std::min(jitter_ms_, offset));
    }else{
        std::uniform_int_distribution<int> dist(-jitter_ms_, jitter_ms_);
        offset = dist(rng_);
    }

    int delay = frequency_.count() + offset;
    if( delay < 1 ) delay = 1;
    return std::chrono::milliseconds(delay);
}

bool Executor::should_fire(){

    if( fire_probability_ >= 1.0 ){
        return true;
    }
    if( fire_probability_ <= 0.0 ){
        return false;
    }

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng_) < fire_probability_;
}

void Executor::register_callback(){
    timer_->expires_after( compute_next_delay() );
    timer_->async_wait( [this]( const boost::system::error_code& error ){
        timer_callback(error);
    });
}

void Executor::timer_callback( const boost::system::error_code& error ){

    if( error == boost::asio::error::operation_aborted ){
        register_callback();
    }else if( error ){
        return;
    }else{
        if( !should_fire() ){
            skip_count_++;
            register_callback();
            return;
        }
        execution_count_++;
        if( synchronous_ ){
            execute_sync();
        }else{
            execute_async();
        }
    }
}

void Executor::execute_sync(){

    if( output_callback_ ){
        auto result = run_process( command_ );
        output_callback_( result.stdout_data );
    }else{
        system( command_.c_str() );
    }
    register_callback();
}

void Executor::execute_async(){

#ifdef _WIN32
    register_callback();
    if( output_callback_ ){
        auto result = run_process( command_ );
        output_callback_( result.stdout_data );
    }else{
        std::string async_cmd = "cmd /c start /b " + command_;
        system( async_cmd.c_str() );
    }
#else
    register_callback();

    pid_t fork_result = fork();

    if( fork_result == 0 ){
        if( output_callback_ ){
            auto result = run_process( command_ );
            if( !result.stdout_data.empty() ){
                fputs(result.stdout_data.c_str(), stdout);
            }
        }else{
            system( command_.c_str() );
        }
        _exit(0);
    }else if( fork_result == -1 ){
        std::cerr << "frequent-cron: failed to fork." << std::endl;
    }else{
        int status;
        waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
        waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
    }
#endif

}
