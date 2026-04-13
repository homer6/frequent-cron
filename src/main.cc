#include "executor.h"
#include "config.h"
#include "daemonize.h"
#include "pid_file.h"
#include "data_dir.h"
#include "log_manager.h"
#include "service_registry.h"

#include <iostream>
#include <filesystem>
#include <memory>

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

    if( !daemonize() ){
        std::cerr << "frequent-cron: failed to daemonize.\n";
        return 1;
    }

    if( config.has_pid_file ){
        if( !write_pid_file(config.pid_filename) ){
            return 1;
        }
    }

    // Set up log capture when running as a managed service
    std::shared_ptr<LogManager> log_mgr;
    if( !config.service_name.empty() ){
        auto dir = config.data_dir.empty()
            ? data_dir::get_default()
            : std::filesystem::path(config.data_dir);
        data_dir::ensure_exists(dir);
        auto log_dir = dir / "logs";
        std::filesystem::create_directories(log_dir);
        log_mgr = std::make_shared<LogManager>(
            log_dir,
            static_cast<size_t>(config.log_max_size_mb) * 1024 * 1024,
            config.log_max_files
        );
    }

    Executor executor( config.command, config.frequency, config.synchronous,
                       config.jitter_ms, config.jitter_distribution, config.fire_probability );

    if( log_mgr ){
        auto name = config.service_name;
        executor.set_output_callback( [log_mgr, name]( const std::string& output ){
            log_mgr->write( name, output );
        });
    }

    executor.run();
    return 0;
}

static ServiceRegistry make_registry( const Config& config ){
    auto dir = config.data_dir.empty()
        ? data_dir::get_default()
        : std::filesystem::path(config.data_dir);
    data_dir::ensure_exists(dir);
    return ServiceRegistry(dir);
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

        case Subcommand::INSTALL: {
            auto registry = make_registry(config);
            return registry.cmd_install(config);
        }

        case Subcommand::REMOVE: {
            auto registry = make_registry(config);
            return registry.cmd_remove(config.service_name);
        }

        case Subcommand::START: {
            auto registry = make_registry(config);
            return registry.cmd_start(config.service_name);
        }

        case Subcommand::STOP: {
            auto registry = make_registry(config);
            return registry.cmd_stop(config.service_name);
        }

        case Subcommand::STATUS: {
            auto registry = make_registry(config);
            return registry.cmd_status(config.service_name);
        }

        case Subcommand::LIST: {
            auto registry = make_registry(config);
            return registry.cmd_list();
        }

        case Subcommand::LOGS: {
            auto registry = make_registry(config);
            return registry.cmd_logs(config.service_name);
        }
    }

    return 1;
}
