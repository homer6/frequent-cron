#ifdef _WIN32
    #include <process.h>
    #include <io.h>
    #include <windows.h>
#else
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <memory>

#include <boost/program_options.hpp>
#include <boost/asio.hpp>

namespace po = boost::program_options;

static std::string shell_command;
static std::chrono::milliseconds frequency_in_ms;
static std::shared_ptr<boost::asio::steady_timer> timer;
static bool synchronous = true;
static std::shared_ptr<boost::asio::io_context> io_context_ptr;

static void register_callback();

static void timer_callback( const boost::system::error_code& error ){

    if( error == boost::asio::error::operation_aborted ){
        register_callback();
    }else if( error ){
        return;
    }else{

        if( synchronous ){
            system( shell_command.c_str() );
            register_callback();
        }else{

#ifdef _WIN32
            // On Windows, use cmd /c start to launch async
            register_callback();
            std::string async_cmd = "cmd /c start /b " + shell_command;
            system( async_cmd.c_str() );
#else
            register_callback();

            pid_t fork_result = fork();

            if( fork_result == 0 ){
                system( shell_command.c_str() );
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

    }

}

static void register_callback(){

    timer->expires_after( frequency_in_ms );
    timer->async_wait( timer_callback );

}


int main( int argc, char** argv ){

    // handle the command line arguments
    po::options_description desc("Options");
    desc.add_options()
        ( "help", "produce help message" )
        ( "frequency", po::value<int>(), "The timer frequency, in milliseconds." )
        ( "command", po::value<std::string>(), "The shell command that the cron will run every frequency." )
        ( "pid-file", po::value<std::string>(), "The file that this daemon will write the process ID to." )
        ( "synchronous", po::value<std::string>()->implicit_value("true"), "Whether calling the command blocks subsequent calls. Defaults to true." )
    ;

    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm );
    po::notify( vm );

    if( vm.count("help") ){
        std::cout << desc << "\n";
        return 1;
    }

    if( vm.count("frequency") ){
        std::cout << "Frequency was set to " << vm["frequency"].as<int>() << ".\n";
    }else{
        std::cout << "Frequency was not set.\n";
        return 1;
    }

    if( vm.count("command") ){
        std::cout << "Command was set to " << vm["command"].as<std::string>() << ".\n";
    }else{
        std::cout << "Command was not set.\n";
        return 1;
    }

    if( vm.count("synchronous") ){
        auto val = vm["synchronous"].as<std::string>();
        if( val != "true" && val != "True" && val != "TRUE" && val != "1" ){
            synchronous = false;
            std::cout << "Synchronous was set to false.\n";
        }else{
            std::cout << "Synchronous was set to true.\n";
        }
    }else{
        std::cout << "Synchronous was not set (defaults to true).\n";
    }

    std::string pid_filename;
    bool pid_file_set = false;
    if( vm.count("pid-file") ){
        pid_filename = vm["pid-file"].as<std::string>();
        pid_file_set = true;
        std::cout << "PID File was set to " << pid_filename << ".\n";
    }

    // setup the program variables
    int frequency = vm["frequency"].as<int>();
    shell_command = vm["command"].as<std::string>();

    // create a timer
    io_context_ptr = std::make_shared<boost::asio::io_context>();
    frequency_in_ms = std::chrono::milliseconds(frequency);
    timer = std::make_shared<boost::asio::steady_timer>( *io_context_ptr, frequency_in_ms );

    // daemonize on POSIX, run in foreground on Windows
#ifdef _WIN32
    // On Windows, run in foreground (use Task Scheduler for service behavior)
#else
    // fork and run in background
    pid_t fork_result = fork();
    if( fork_result < 0 ){
        perror("fork");
        return 1;
    }
    if( fork_result > 0 ){
        // parent exits
        _exit(0);
    }
    // child continues as daemon
    setsid();
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
#endif

    // write pid file
    if( pid_file_set ){
#ifdef _WIN32
        int pid = _getpid();
#else
        pid_t pid = getpid();
#endif
        std::ofstream pid_file( pid_filename );
        if( !pid_file ){
            std::cerr << "frequent-cron: couldn't open pid file " << pid_filename << "\n";
            return 1;
        }
        pid_file << pid;
    }

    // register the initial timer callback and run the service
    register_callback();
    io_context_ptr->run();

    return 0;
}
