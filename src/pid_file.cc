#ifdef _WIN32
    #include <process.h>
#else
    #include <unistd.h>
#endif

#include "pid_file.h"

#include <fstream>

bool write_pid_file( const std::string& filename ){

    std::ofstream file( filename );
    if( !file ){
        return false;
    }

#ifdef _WIN32
    file << _getpid();
#else
    file << getpid();
#endif

    return true;
}
