#include <gtest/gtest.h>

#ifndef _WIN32

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <sys/wait.h>
#include <filesystem>

// Regression tests for https://github.com/homer6/frequent-cron/issues/17
//
// The bug: daemonize() closed FDs 0-2 instead of redirecting to /dev/null.
// POSIX declares execution with closed standard FDs "non-conforming", causing
// backgrounded commands (command &) to silently fail on some platforms.
//
// These tests verify the FD state after daemonization setup, which is the
// deterministic root cause regardless of platform shell behavior.

static std::string make_temp_path( const std::string& name ){
    return std::filesystem::temp_directory_path() / ("fc_test_" + name + "_" + std::to_string(getpid()));
}

TEST(Daemonize, ClosedFDsAreInvalid) {
    // Prove that closing FDs 0-2 leaves them invalid.
    // This is the broken state that caused the regression.

    pid_t pid = fork();
    ASSERT_NE(pid, -1);

    if( pid == 0 ){
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // FDs 0-2 should be invalid after close
        int r0 = fcntl(STDIN_FILENO, F_GETFD);
        int r1 = fcntl(STDOUT_FILENO, F_GETFD);
        int r2 = fcntl(STDERR_FILENO, F_GETFD);

        // All three should return -1 (invalid)
        _exit((r0 == -1 && r1 == -1 && r2 == -1) ? 0 : 1);
    }

    int status;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0) << "Closed FDs should be invalid";
}

TEST(Daemonize, ClosedFDsCauseReuse) {
    // Prove the secondary hazard: after closing FDs 0-2, the next open()
    // reuses FD 0, so any library writing to "stderr" would corrupt that file.

    pid_t pid = fork();
    ASSERT_NE(pid, -1);

    if( pid == 0 ){
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // Next open() should get FD 0 (lowest available)
        int fd = open("/dev/null", O_RDONLY);
        _exit(fd == 0 ? 0 : 1);
    }

    int status;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0) << "Next open() after closing FDs 0-2 should reuse FD 0";
}

TEST(Daemonize, DevNullRedirectKeepsFDsValid) {
    // Prove that redirecting to /dev/null (the correct behavior) leaves
    // FDs 0-2 open and valid.

    pid_t pid = fork();
    ASSERT_NE(pid, -1);

    if( pid == 0 ){
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if( fd > 2 ) close(fd);

        // FDs 0-2 should all be valid
        int r0 = fcntl(STDIN_FILENO, F_GETFD);
        int r1 = fcntl(STDOUT_FILENO, F_GETFD);
        int r2 = fcntl(STDERR_FILENO, F_GETFD);

        _exit((r0 != -1 && r1 != -1 && r2 != -1) ? 0 : 1);
    }

    int status;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0) << "FDs should remain valid after /dev/null redirect";
}

TEST(Daemonize, DevNullRedirectDoesNotReuseNextFD) {
    // After correct redirect, next open() should NOT get FD 0/1/2.

    pid_t pid = fork();
    ASSERT_NE(pid, -1);

    if( pid == 0 ){
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if( fd > 2 ) close(fd);

        int next_fd = open("/dev/null", O_RDONLY);
        _exit((next_fd > 2) ? 0 : 1);
    }

    int status;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0) << "Next open() should get FD > 2 when 0-2 are occupied";
}

TEST(Daemonize, BackgroundCommandWorksWithDevNull) {
    // End-to-end: backgrounded command via system() works with /dev/null redirect.

    std::string marker = make_temp_path("devnull");
    std::remove(marker.c_str());

    pid_t pid = fork();
    ASSERT_NE(pid, -1);

    if( pid == 0 ){
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if( fd > 2 ) close(fd);

        std::string cmd = "touch " + marker + " &";
        system(cmd.c_str());

        usleep(200000);

        struct stat st;
        int exists = (stat(marker.c_str(), &st) == 0) ? 0 : 1;
        _exit(exists);
    }

    int status;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0) << "Background command should work with /dev/null redirect";

    std::remove(marker.c_str());
}

TEST(Daemonize, BackgroundCommandWithOutputWorksWithDevNull) {
    // Backgrounded command that writes to stdout/stderr — the scenario from issue #17.
    // The reporter's script backgrounds a CLI binary that produces output.

    std::string marker = make_temp_path("output");
    std::remove(marker.c_str());

    pid_t pid = fork();
    ASSERT_NE(pid, -1);

    if( pid == 0 ){
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if( fd > 2 ) close(fd);

        // Command that writes to both stdout and stderr before creating marker
        std::string cmd = "sh -c 'echo stdout_test; echo stderr_test >&2; touch " + marker + "' &";
        system(cmd.c_str());

        usleep(200000);

        struct stat st;
        int exists = (stat(marker.c_str(), &st) == 0) ? 0 : 1;
        _exit(exists);
    }

    int status;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0) << "Background command with output should work with /dev/null redirect";

    std::remove(marker.c_str());
}

#endif // _WIN32
