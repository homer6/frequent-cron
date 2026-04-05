#include <gtest/gtest.h>
#include "process_runner.h"

TEST(Process, CapturesStdout) {
    auto result = run_process("echo hello");
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_NE(result.stdout_data.find("hello"), std::string::npos);
}

TEST(Process, CapturesExitCode) {
    auto result = run_process("exit 42");
    EXPECT_EQ(result.exit_code, 42);
}

TEST(Process, CapturesStderr) {
    // stderr is redirected to stdout via 2>&1 in run_process
    // Use a subshell to ensure ordering
    auto result = run_process("sh -c 'echo error 1>&2'");
    EXPECT_NE(result.stdout_data.find("error"), std::string::npos);
}

TEST(Process, EmptyCommand) {
    auto result = run_process("true");
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_TRUE(result.stdout_data.empty());
}

TEST(Process, MultilineOutput) {
    auto result = run_process("echo line1 && echo line2");
    EXPECT_NE(result.stdout_data.find("line1"), std::string::npos);
    EXPECT_NE(result.stdout_data.find("line2"), std::string::npos);
}
