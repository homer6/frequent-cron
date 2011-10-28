#include <sys/time.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>


#define USE(x) (x) = (x)

using namespace std;
namespace program_options = boost::program_options;

int main( int argc, char** argv ){



    //handle the command line arguments
        program_options::options_description desc("Options");
        desc.add_options()
            ( "help", "produce help message" )
            ( "frequency", program_options::value<int>(), "the timer frequency, in milliseconds" )
            ( "command", program_options::value<string>(), "the shell command that the cron will run every frequency" )
        ;

        program_options::variables_map map;
        program_options::store( program_options::parse_command_line(argc, argv, desc), map );
        program_options::notify( map );

        if( map.count("help") ){
            cout << desc << "\n";
            return 1;
        }

        if( map.count("frequency") ){
            cout << "Frequency was set to " << map["frequency"].as<int>() << ".\n";
        }else{
            cout << "Frequency was not set.\n";
        }

        return 0;




	//open log file
		FILE *log_file;
		unsigned loop_count = 0;
		string log_filename( "/tmp/console.log" );
		
		log_file = fopen( log_filename.c_str(), "a" );
		if( log_file == NULL ){
            fprintf( stdout, "%s: Couldn't open log file %s\n", argv[0], log_filename.c_str() );
            return 1;
		}

    //create a timer
        /*
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        float microtime = 0.0f;

        microtime = current_time.tv_sec;
        microtime += current_time.tv_usec/1000; // the elapsed milliseconds over 1 second(1000 milliseconds);

        time_t current_time;
        current_time = time( NULL );
        printf( "\tWall Clock): %ld\n", (long)current_time );
        return 0;
        */
		
	//start the service
		if( daemon(0,0) == -1 ){
			err( 1, NULL );
		}

	//main service loop
		while( 1 ){
			
			loop_count++;
			
            if( (loop_count % 1000) == 0 ){
				fprintf( log_file, "Loop count is %i\n", loop_count );			
			}
			
            /*if( loop_count == 100000 ){
				break;
			}
            */
		
		}
		
	//cleanup allocated resources
		fclose( log_file );
		
	return 0;
	
}
