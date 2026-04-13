#include <gtest/gtest.h>
#include "executor.h"

#include <fstream>
#include <cstdio>
#include <string>
#include <thread>
#include <chrono>

class ExecutorTest : public ::testing::Test {
protected:
    std::string tmp_path;

    void SetUp() override {
        tmp_path = testing::TempDir() + "frequent_cron_test_executor";
    }

    void TearDown() override {
        std::remove(tmp_path.c_str());
    }

    int count_lines(const std::string& path) {
        std::ifstream file(path);
        int count = 0;
        std::string line;
        while (std::getline(file, line)) count++;
        return count;
    }
};

TEST_F(ExecutorTest, ExecutesCommandSynchronously) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 100, true);

    // Run in a thread, stop after a bit
    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(550));
    executor.stop();
    t.join();

    int lines = count_lines(tmp_path);
    EXPECT_GE(lines, 3);
    EXPECT_GE(executor.execution_count(), 3);
}

TEST_F(ExecutorTest, SyncBlocksOnSlowCommand) {
    std::string cmd = "sleep 1 && echo done >> " + tmp_path;
    Executor executor(cmd, 50, true);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    executor.stop();
    t.join();

    int lines = count_lines(tmp_path);
    EXPECT_LE(lines, 3);
}

TEST_F(ExecutorTest, ExecutionCountIncrements) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 50, true);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    executor.stop();
    t.join();

    EXPECT_GE(executor.execution_count(), 3);
}

TEST_F(ExecutorTest, StopHaltsExecution) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 50, true);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    executor.stop();
    t.join();

    int count_at_stop = count_lines(tmp_path);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    int count_after = count_lines(tmp_path);

    EXPECT_EQ(count_at_stop, count_after);
}

TEST_F(ExecutorTest, NonexistentCommandDoesNotCrash) {
    Executor executor("/nonexistent/script.sh", 100, true);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(350));
    executor.stop();
    t.join();

    EXPECT_GE(executor.execution_count(), 1);
}

TEST_F(ExecutorTest, FrequencyOneMillisecond) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 1, true);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    executor.stop();
    t.join();

    // Just verify it ran without crashing; count varies by platform
    EXPECT_GE(executor.execution_count(), 1);
}

#ifndef _WIN32
TEST_F(ExecutorTest, AsyncAllowsConcurrentExecution) {
    std::string cmd = "sleep 1 && echo done >> " + tmp_path;
    Executor executor(cmd, 200, false);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    executor.stop();
    t.join();

    // Wait for forked children to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    int lines = count_lines(tmp_path);
    EXPECT_GE(lines, 4);
}
#endif

// === Jitter tests ===

TEST_F(ExecutorTest, JitterZeroMatchesBaseFrequency) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 100, true, 0);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(550));
    executor.stop();
    t.join();

    EXPECT_GE(executor.execution_count(), 3);
}

TEST_F(ExecutorTest, JitterProducesExecutions) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 100, true, 50, "uniform");

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    executor.stop();
    t.join();

    // With jitter +-50ms on 100ms base, interval ranges 50-150ms
    // In 1000ms we expect at least a few executions
    EXPECT_GE(executor.execution_count(), 3);
}

TEST_F(ExecutorTest, JitterNormalDistribution) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 100, true, 30, "normal");

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    executor.stop();
    t.join();

    EXPECT_GE(executor.execution_count(), 3);
}

// === Probability tests ===

TEST_F(ExecutorTest, ProbabilityOneAlwaysFires) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 50, true, 0, "uniform", 1.0);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(350));
    executor.stop();
    t.join();

    EXPECT_GE(executor.execution_count(), 3);
    EXPECT_EQ(executor.skip_count(), 0);
}

TEST_F(ExecutorTest, ProbabilityZeroNeverFires) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 50, true, 0, "uniform", 0.0);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(350));
    executor.stop();
    t.join();

    EXPECT_EQ(executor.execution_count(), 0);
    EXPECT_GE(executor.skip_count(), 3);
}

TEST_F(ExecutorTest, ProbabilityPartialReducesExecutions) {
    std::string cmd = "echo tick >> " + tmp_path;
    // 50% probability with fast frequency -- run many ticks to get statistical significance
    Executor executor(cmd, 10, true, 0, "uniform", 0.5);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    executor.stop();
    t.join();

    int total = executor.execution_count() + executor.skip_count();
    EXPECT_GE(total, 20);
    // Both executions and skips should have occurred
    EXPECT_GE(executor.execution_count(), 1);
    EXPECT_GE(executor.skip_count(), 1);
}

TEST_F(ExecutorTest, JitterAndProbabilityCombined) {
    std::string cmd = "echo tick >> " + tmp_path;
    Executor executor(cmd, 100, true, 30, "uniform", 0.5);

    std::thread t([&]{ executor.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    executor.stop();
    t.join();

    int total = executor.execution_count() + executor.skip_count();
    EXPECT_GE(total, 3);
    // With 50% probability, expect some skips
    EXPECT_GE(executor.skip_count(), 1);
}
