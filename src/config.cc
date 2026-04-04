#include "config.h"

#include <sstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

static po::options_description make_description(){
    po::options_description desc("Options");
    desc.add_options()
        ( "help", "produce help message" )
        ( "frequency", po::value<int>(), "The timer frequency, in milliseconds." )
        ( "command", po::value<std::string>(), "The shell command that the cron will run every frequency." )
        ( "pid-file", po::value<std::string>(), "The file that this daemon will write the process ID to." )
        ( "synchronous", po::value<std::string>()->implicit_value("true"), "Whether calling the command blocks subsequent calls. Defaults to true." )
    ;
    return desc;
}

ParseOutput parse_args( int argc, char** argv ){

    auto desc = make_description();
    ParseOutput output;

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

        output.config.synchronous = true;
        if( vm.count("synchronous") ){
            auto val = vm["synchronous"].as<std::string>();
            if( val != "true" && val != "True" && val != "TRUE" && val != "1" ){
                output.config.synchronous = false;
            }
        }

        output.result = ParseResult::OK;

    } catch( const std::exception& e ){
        output.result = ParseResult::PARSE_ERROR;
        output.message = e.what();
    }

    return output;
}

ParseOutput parse_args( const std::vector<std::string>& args ){

    // Build argc/argv from vector
    std::vector<const char*> argv_ptrs;
    argv_ptrs.push_back("frequent-cron");
    for( const auto& arg : args ){
        argv_ptrs.push_back(arg.c_str());
    }

    return parse_args( static_cast<int>(argv_ptrs.size()), const_cast<char**>(argv_ptrs.data()) );
}
