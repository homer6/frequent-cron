#include "process.h"

#include <array>
#include <cstdio>

#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#else
    #include <sys/wait.h>
#endif

ProcessResult run_process( const std::string& command ){

    ProcessResult result;

    // Redirect stderr to stdout so we capture both
    std::string full_cmd = command + " 2>&1";

    FILE* pipe = popen( full_cmd.c_str(), "r" );
    if( !pipe ){
        result.exit_code = -1;
        result.stderr_data = "Failed to execute command";
        return result;
    }

    std::array<char, 4096> buffer;
    while( fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) ){
        result.stdout_data += buffer.data();
    }

    int status = pclose(pipe);

#ifdef _WIN32
    result.exit_code = status;
#else
    if( WIFEXITED(status) ){
        result.exit_code = WEXITSTATUS(status);
    }else{
        result.exit_code = -1;
    }
#endif

    return result;
}
