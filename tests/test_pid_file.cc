#include <gtest/gtest.h>
#include "pid_file.h"

#include <fstream>
#include <cstdio>
#include <string>

#ifdef _WIN32
    #include <process.h>
    #define GET_PID _getpid()
#else
    #include <unistd.h>
    #define GET_PID getpid()
#endif

class PidFileTest : public ::testing::Test {
protected:
    std::string tmp_path;

    void SetUp() override {
        tmp_path = testing::TempDir() + "frequent_cron_test_pid";
    }

    void TearDown() override {
        std::remove(tmp_path.c_str());
    }
};

TEST_F(PidFileTest, CreatesFile) {
    EXPECT_TRUE(write_pid_file(tmp_path));
    std::ifstream file(tmp_path);
    EXPECT_TRUE(file.good());
}

TEST_F(PidFileTest, ContainsCurrentPid) {
    EXPECT_TRUE(write_pid_file(tmp_path));
    std::ifstream file(tmp_path);
    int pid;
    file >> pid;
    EXPECT_EQ(pid, GET_PID);
}

TEST_F(PidFileTest, InvalidPathReturnsFalse) {
    EXPECT_FALSE(write_pid_file("/nonexistent/dir/test.pid"));
}
