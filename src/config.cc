#include "config.h"

#include <sstream>
#include <map>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

static const std::map<std::string, Subcommand> subcommand_map = {
    { "run",     Subcommand::RUN },
    { "install", Subcommand::INSTALL },
    { "remove",  Subcommand::REMOVE },
    { "start",   Subcommand::START },
    { "stop",    Subcommand::STOP },
    { "status",  Subcommand::STATUS },
    { "list",    Subcommand::LIST },
    { "logs",    Subcommand::LOGS },
};

static po::options_description make_run_options(){
    po::options_description desc("Options");
    desc.add_options()
        ( "help", "produce help message" )
        ( "frequency", po::value<int>(), "The timer frequency, in milliseconds." )
        ( "command", po::value<std::string>(), "The shell command that the cron will run every frequency." )
        ( "pid-file", po::value<std::string>(), "The file that this daemon will write the process ID to." )
        ( "synchronous", po::value<std::string>()->implicit_value("true"), "Whether calling the command blocks subsequent calls. Defaults to true." )
        ( "data-dir", po::value<std::string>(), "Override the data directory path." )
        ( "jitter", po::value<int>(), "Maximum timing variance in milliseconds. The actual interval is frequency +/- jitter." )
        ( "jitter-distribution", po::value<std::string>(), "Distribution for jitter: 'uniform' or 'normal'. Defaults to 'uniform'." )
        ( "fire-probability", po::value<double>(), "Probability of firing on each tick (0.0-1.0). Defaults to 1.0." )
    ;
    return desc;
}

static po::options_description make_install_options(){
    po::options_description desc("Install options");
    desc.add_options()
        ( "help", "produce help message" )
        ( "frequency", po::value<int>(), "The timer frequency, in milliseconds." )
        ( "command", po::value<std::string>(), "The shell command that the cron will run every frequency." )
        ( "synchronous", po::value<std::string>()->implicit_value("true"), "Whether calling the command blocks subsequent calls. Defaults to true." )
        ( "data-dir", po::value<std::string>(), "Override the data directory path." )
        ( "jitter", po::value<int>(), "Maximum timing variance in milliseconds. The actual interval is frequency +/- jitter." )
        ( "jitter-distribution", po::value<std::string>(), "Distribution for jitter: 'uniform' or 'normal'. Defaults to 'uniform'." )
        ( "fire-probability", po::value<double>(), "Probability of firing on each tick (0.0-1.0). Defaults to 1.0." )
    ;
    return desc;
}

static po::options_description make_name_only_options(){
    po::options_description desc("Options");
    desc.add_options()
        ( "help", "produce help message" )
        ( "data-dir", po::value<std::string>(), "Override the data directory path." )
    ;
    return desc;
}

static void parse_common_options( const po::variables_map& vm, Config& config ){
    if( vm.count("data-dir") ){
        config.data_dir = vm["data-dir"].as<std::string>();
    }
    if( vm.count("synchronous") ){
        auto val = vm["synchronous"].as<std::string>();
        if( val != "true" && val != "True" && val != "TRUE" && val != "1" ){
            config.synchronous = false;
        }
    }
    if( vm.count("jitter") ){
        config.jitter_ms = vm["jitter"].as<int>();
    }
    if( vm.count("jitter-distribution") ){
        config.jitter_distribution = vm["jitter-distribution"].as<std::string>();
    }
    if( vm.count("fire-probability") ){
        config.fire_probability = vm["fire-probability"].as<double>();
    }
}

static ParseOutput parse_run_args( int argc, char** argv, Subcommand subcmd ){

    auto desc = make_run_options();
    ParseOutput output;
    output.config.subcommand = subcmd;

    try {
        po::variables_map vm;
        po::store( po::parse_command_line(argc, argv, desc), vm );
        po::notify( vm );

        if( vm.count("help") ){
            std::ostringstream oss;
            oss << "Usage: frequent-cron run --frequency=<ms> --command=<cmd>\n\n" << desc;
            output.result = ParseResult::HELP;
            output.message = oss.str();
            return output;
        }

        if( !vm.count("frequency") ){
            output.result = ParseResult::PARSE_ERROR;
            output.message = "Frequency was not set.";
            return output;
        }

        if( !vm.count("command") ){
            output.result = ParseResult::PARSE_ERROR;
            output.message = "Command was not set.";
            return output;
        }

        output.config.frequency = vm["frequency"].as<int>();
        output.config.command = vm["command"].as<std::string>();

        if( vm.count("pid-file") ){
            output.config.pid_filename = vm["pid-file"].as<std::string>();
            output.config.has_pid_file = true;
        }

        parse_common_options( vm, output.config );
        output.result = ParseResult::OK;

    } catch( const std::exception& e ){
        output.result = ParseResult::PARSE_ERROR;
        output.message = e.what();
    }

    return output;
}

static ParseOutput parse_install_args( int argc, char** argv, const std::string& service_name ){

    auto desc = make_install_options();
    ParseOutput output;
    output.config.subcommand = Subcommand::INSTALL;
    output.config.service_name = service_name;

    try {
        po::variables_map vm;
        po::store( po::parse_command_line(argc, argv, desc), vm );
        po::notify( vm );

        if( vm.count("help") ){
            std::ostringstream oss;
            oss << "Usage: frequent-cron install <name> --frequency=<ms> --command=<cmd>\n\n" << desc;
            output.result = ParseResult::HELP;
            output.message = oss.str();
            return output;
        }

        if( !vm.count("frequency") ){
            output.result = ParseResult::PARSE_ERROR;
            output.message = "Frequency was not set.";
            return output;
        }

        if( !vm.count("command") ){
            output.result = ParseResult::PARSE_ERROR;
            output.message = "Command was not set.";
            return output;
        }

        output.config.frequency = vm["frequency"].as<int>();
        output.config.command = vm["command"].as<std::string>();
        parse_common_options( vm, output.config );
        output.result = ParseResult::OK;

    } catch( const std::exception& e ){
        output.result = ParseResult::PARSE_ERROR;
        output.message = e.what();
    }

    return output;
}

static ParseOutput parse_name_subcommand( int argc, char** argv, Subcommand subcmd, const std::string& service_name ){

    auto desc = make_name_only_options();
    ParseOutput output;
    output.config.subcommand = subcmd;
    output.config.service_name = service_name;

    try {
        po::variables_map vm;
        po::store( po::parse_command_line(argc, argv, desc), vm );
        po::notify( vm );

        if( vm.count("help") ){
            std::ostringstream oss;
            oss << desc;
            output.result = ParseResult::HELP;
            output.message = oss.str();
            return output;
        }

        parse_common_options( vm, output.config );
        output.result = ParseResult::OK;

    } catch( const std::exception& e ){
        output.result = ParseResult::PARSE_ERROR;
        output.message = e.what();
    }

    return output;
}


ParseOutput parse_args( int argc, char** argv ){

    ParseOutput output;

    if( argc < 2 ){
        std::ostringstream oss;
        oss << "Usage: frequent-cron <command> [options]\n\n"
            << "Commands:\n"
            << "  run       Run a command at the specified frequency\n"
            << "  install   Register a named service\n"
            << "  remove    Unregister a named service\n"
            << "  start     Start a registered service\n"
            << "  stop      Stop a running service\n"
            << "  status    Show status of services\n"
            << "  list      List all registered services\n"
            << "  logs      Show logs for a service\n"
            << "\nRun 'frequent-cron <command> --help' for more information.\n";
        output.result = ParseResult::PARSE_ERROR;
        output.message = oss.str();
        return output;
    }

    std::string first_arg = argv[1];

    // Top-level --help: show full command list
    if( first_arg == "--help" || first_arg == "-h" ){
        std::ostringstream oss;
        oss << "Usage: frequent-cron <command> [options]\n\n"
            << "A daemon that runs shell commands at millisecond intervals (sub-second cron).\n\n"
            << "Commands:\n"
            << "  run       Run a command at the specified frequency\n"
            << "  install   Register a named service\n"
            << "  remove    Unregister a named service\n"
            << "  start     Start a registered service\n"
            << "  stop      Stop a running service\n"
            << "  status    Show status of services\n"
            << "  list      List all registered services\n"
            << "  logs      Show logs for a service\n"
            << "\nRun 'frequent-cron <command> --help' for command-specific options.\n";
        output.result = ParseResult::HELP;
        output.message = oss.str();
        return output;
    }

    // Legacy mode: if first arg starts with '-', treat as run
    if( first_arg[0] == '-' ){
        return parse_run_args( argc, argv, Subcommand::LEGACY );
    }

    // Look up subcommand
    auto it = subcommand_map.find(first_arg);
    if( it == subcommand_map.end() ){
        output.result = ParseResult::PARSE_ERROR;
        output.message = "Unknown command: " + first_arg;
        return output;
    }

    Subcommand subcmd = it->second;

    // For subcommands, shift argv past the subcommand name
    int sub_argc = argc - 1;
    char** sub_argv = argv + 1;

    switch( subcmd ){
        case Subcommand::RUN:
            return parse_run_args( sub_argc, sub_argv, Subcommand::RUN );

        case Subcommand::INSTALL: {
            if( sub_argc < 2 || sub_argv[1][0] == '-' ){
                output.result = ParseResult::PARSE_ERROR;
                output.message = "Usage: frequent-cron install <name> --frequency=<ms> --command=<cmd>";
                return output;
            }
            std::string name = sub_argv[1];
            int install_argc = sub_argc - 1;
            char** install_argv = sub_argv + 1;
            return parse_install_args( install_argc, install_argv, name );
        }

        case Subcommand::REMOVE:
        case Subcommand::START:
        case Subcommand::STOP:
        case Subcommand::LOGS: {
            if( sub_argc < 2 || sub_argv[1][0] == '-' ){
                output.result = ParseResult::PARSE_ERROR;
                output.message = "Usage: frequent-cron " + first_arg + " <name>";
                return output;
            }
            std::string name = sub_argv[1];
            int name_argc = sub_argc - 1;
            char** name_argv = sub_argv + 1;
            return parse_name_subcommand( name_argc, name_argv, subcmd, name );
        }

        case Subcommand::STATUS: {
            // Optional name argument
            std::string name;
            int status_argc = sub_argc;
            char** status_argv = sub_argv;
            if( sub_argc >= 2 && sub_argv[1][0] != '-' ){
                name = sub_argv[1];
                status_argc = sub_argc - 1;
                status_argv = sub_argv + 1;
            }
            return parse_name_subcommand( status_argc, status_argv, Subcommand::STATUS, name );
        }

        case Subcommand::LIST: {
            output.config.subcommand = Subcommand::LIST;
            // Parse optional --data-dir
            auto desc = make_name_only_options();
            try {
                po::variables_map vm;
                po::store( po::parse_command_line(sub_argc, sub_argv, desc), vm );
                po::notify( vm );
                if( vm.count("help") ){
                    output.result = ParseResult::HELP;
                    output.message = "Usage: frequent-cron list\n";
                    return output;
                }
                parse_common_options( vm, output.config );
            } catch( const std::exception& e ){
                output.result = ParseResult::PARSE_ERROR;
                output.message = e.what();
                return output;
            }
            output.result = ParseResult::OK;
            return output;
        }

        default:
            output.result = ParseResult::PARSE_ERROR;
            output.message = "Unknown command.";
            return output;
    }
}

ParseOutput parse_args( const std::vector<std::string>& args ){

    std::vector<const char*> argv_ptrs;
    argv_ptrs.push_back("frequent-cron");
    for( const auto& arg : args ){
        argv_ptrs.push_back(arg.c_str());
    }

    return parse_args( static_cast<int>(argv_ptrs.size()), const_cast<char**>(argv_ptrs.data()) );
}
