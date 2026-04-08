#include "daemonize.h"

#ifdef _WIN32

bool daemonize(){
    // On Windows, run in foreground (use Task Scheduler for service behavior)
    return true;
}

#else

#include <unistd.h>
#include <fcntl.h>

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

    // Redirect stdin/stdout/stderr to /dev/null rather than closing them.
    // Closing FDs 0-2 violates POSIX: child processes launched via system()
    // inherit closed FDs, causing backgrounded commands (&) to silently fail.
    // This matches the behavior of daemon(0, 0).
    // See: https://github.com/homer6/frequent-cron/issues/17
    int fd = open("/dev/null", O_RDWR);
    if( fd != -1 ){
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if( fd > 2 ) close(fd);
    }

    return true;
}

#endif
