#include "config.h"
#include "daemonize.h"
#include "pid_file.h"
#include "executor.h"
#include "data_dir.h"

#include <iostream>
#include <filesystem>

static int cmd_run( const Config& config ){

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


int main( int argc, char** argv ){

    auto output = parse_args( argc, argv );

    if( output.result == ParseResult::HELP ){
        std::cout << output.message << "\n";
        return 0;
    }

    if( output.result == ParseResult::PARSE_ERROR ){
        std::cerr << output.message << "\n";
        return 1;
    }

    auto& config = output.config;

    switch( config.subcommand ){

        case Subcommand::RUN:
        case Subcommand::LEGACY:
            return cmd_run( config );

        case Subcommand::INSTALL:
            std::cerr << "install: not yet implemented\n";
            return 1;

        case Subcommand::REMOVE:
            std::cerr << "remove: not yet implemented\n";
            return 1;

        case Subcommand::START:
            std::cerr << "start: not yet implemented\n";
            return 1;

        case Subcommand::STOP:
            std::cerr << "stop: not yet implemented\n";
            return 1;

        case Subcommand::STATUS:
            std::cerr << "status: not yet implemented\n";
            return 1;

        case Subcommand::LIST:
            std::cerr << "list: not yet implemented\n";
            return 1;

        case Subcommand::LOGS:
            std::cerr << "logs: not yet implemented\n";
            return 1;
    }

    return 1;
}
