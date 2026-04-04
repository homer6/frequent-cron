#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <process.h>
    #include <windows.h>
#else
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#include "executor.h"

#include <iostream>
#include <cstdlib>

Executor::Executor( const std::string& command, int frequency_ms, bool synchronous )
    : command_(command)
    , frequency_(frequency_ms)
    , synchronous_(synchronous)
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

void Executor::register_callback(){
    timer_->expires_after( frequency_ );
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
        execution_count_++;
        if( synchronous_ ){
            execute_sync();
        }else{
            execute_async();
        }
    }
}

void Executor::execute_sync(){
    system( command_.c_str() );
    register_callback();
}

void Executor::execute_async(){

#ifdef _WIN32
    register_callback();
    std::string async_cmd = "cmd /c start /b " + command_;
    system( async_cmd.c_str() );
#else
    register_callback();

    pid_t fork_result = fork();

    if( fork_result == 0 ){
        system( command_.c_str() );
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
