#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>

#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <sys/wait.h>




using namespace std;

static void timer_callback( const boost::system::error_code& e );
static void register_callback();

static string shell_command;
static FILE *pid_file;
static boost::posix_time::time_duration frequency_in_ms;
static boost::asio::deadline_timer *timer;
static bool synchronous;
static boost::asio::io_service *io_service;


int main( int argc, char** argv ){

    //handle the command line arguments
        boost::program_options::options_description desc("Options");
        desc.add_options()
            ( "help", "produce help message" )
            ( "frequency", boost::program_options::value<int>(), "The timer frequency, in milliseconds." )
            ( "command", boost::program_options::value<string>(), "The shell command that the cron will run every frequency." )
            ( "pid-file", boost::program_options::value<string>(), "The file that this daemon will write the process ID to. This is used for starting and stopping it as a service." )
            //( "log-filename", boost::program_options::value<string>(), "The filename of the log file." )
            ( "synchronous", boost::program_options::value<string>()->implicit_value("true"), "Whether calling the command blocks subsequent calls. Defaults to true." )
        ;

        boost::program_options::variables_map variables_map;
        boost::program_options::store( boost::program_options::parse_command_line(argc, argv, desc), variables_map );
        boost::program_options::notify( variables_map );

        if( variables_map.count("help") ){
            cout << desc << "\n";
            return 1;
        }

        if( variables_map.count("frequency") ){
            cout << "Frequency was set to " << variables_map["frequency"].as<int>() << ".\n";
        }else{
            cout << "Frequency was not set.\n";
            return 1;
        }

        if( variables_map.count("command") ){
            cout << "Command was set to " << variables_map["command"].as<string>() << ".\n";
        }else{
            cout << "Command was not set.\n";
            return 1;
        }

        synchronous = true;
        if( variables_map.count("synchronous") ){
            string synchronous_str = variables_map["synchronous"].as<string>();            
            if( synchronous_str != "true" && synchronous_str != "True" && synchronous_str != "TRUE" && synchronous_str != "1" ){
                synchronous = false;
                cout << "Synchronous was set to false.\n";
            }else{
                cout << "Synchronous was set to true.\n";
            }
        }else{
            cout << "Synchronous was not set (defaults to true).\n";
        }

        string pid_filename;
        bool pid_file_set = false;
        if( variables_map.count("pid-file") ){
            pid_filename = variables_map["pid-file"].as<string>();
            pid_file_set = true;
            cout << "PID File was set to " << pid_filename << ".\n";
        }


        /*
        string log_filename;
        if( variables_map.count("log-filename") ){
            log_filename = variables_map["log-filename"].as<string>();
        }else{
            log_filename = string("/tmp/console.log");
        }

        */


    //setup the program variables
        int frequency = variables_map["frequency"].as<int>();
        shell_command = variables_map["command"].as<string>();


    //create a timer
        io_service = new boost::asio::io_service();
        frequency_in_ms = boost::posix_time::millisec(frequency);
        timer = new boost::asio::deadline_timer( *io_service, frequency_in_ms );

	//start the service
        if( daemon(0,0) == -1 ){
            err( 1, NULL );
        }

    //open pid file
        if( pid_file_set ){
            pid_file = fopen( pid_filename.c_str(), "w+" );
            if( pid_file == NULL ){
                fprintf( stdout, "%s: Couldn't open log file %s\n", argv[0], pid_filename.c_str() );
                return 1;
            }else{
                fprintf( pid_file, "%d", getpid() );
                fclose( pid_file );
            }
        }

    //register the initial timer callback and run the service
        register_callback();
        io_service->run();
		
    //cleanup allocated resources
        //fclose( log_file );
        delete timer;
        delete io_service;
		
	return 0;
	
}


/**
* This function is call every "frequency".
*
*
*/
static void timer_callback( const boost::system::error_code& error ){

    if( error == boost::asio::error::operation_aborted ){
        //std::cout << "Timer was canceled" << std::endl;
        register_callback();
    }else if( error ){
        //std::cout << "Timer error: " << error.message() << std::endl;
    }else{

        if( synchronous ){
            system( shell_command.c_str() );
            register_callback();
        }else{

            register_callback();

            pid_t fork_result = fork();

            if( fork_result == 0 ){
                //child
                system( shell_command.c_str() );
                exit(0);

            }else if( fork_result == -1 ){
                //fork error
                std::cerr << "Frequent-cron: failed to execute fork." << std::endl;

            }else{
                //parent
                //clear up old zombie processes
                int status;
                waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED );
                waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED );

            }



        }

    }

}


/**
* This function is call every "frequency".
*
*
*/
static void register_callback(){

    timer->expires_from_now( frequency_in_ms );
    timer->async_wait( timer_callback );

}

