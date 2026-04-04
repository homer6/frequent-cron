#include "config.h"
#include "daemonize.h"
#include "pid_file.h"
#include "executor.h"

#include <iostream>

int main( int argc, char** argv ){

    auto output = parse_args( argc, argv );

    if( output.result == ParseResult::HELP ){
        std::cout << output.message << "\n";
        return 1;
    }

    if( output.result == ParseResult::PARSE_ERROR ){
        std::cout << output.message << "\n";
        return 1;
    }

    auto& config = output.config;

    std::cout << "Frequency was set to " << config.frequency << ".\n";
    std::cout << "Command was set to " << config.command << ".\n";

    if( config.synchronous ){
        std::cout << "Synchronous was set to true.\n";
    }else{
        std::cout << "Synchronous was set to false.\n";
    }

    if( config.has_pid_file ){
        std::cout << "PID File was set to " << config.pid_filename << ".\n";
    }

    Executor executor( config.command, config.frequency, config.synchronous );

    if( !daemonize() ){
        std::cerr << "frequent-cron: failed to daemonize.\n";
        return 1;
    }

    if( config.has_pid_file ){
        if( !write_pid_file(config.pid_filename) ){
            return 1;
        }
    }

    executor.run();

    return 0;
}
