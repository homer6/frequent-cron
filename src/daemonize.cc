#include "daemonize.h"

#ifdef _WIN32

bool daemonize(){
    // On Windows, run in foreground (use Task Scheduler for service behavior)
    return true;
}

#else

#include <unistd.h>

bool daemonize(){
    pid_t pid = fork();
    if( pid < 0 ){
        return false;
    }
    if( pid > 0 ){
        _exit(0);
    }
    setsid();
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    return true;
}

#endif
