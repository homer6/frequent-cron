#include <gtest/gtest.h>
#include "log_manager.h"

#include <filesystem>
#include <fstream>

class LogManagerTest : public ::testing::Test {
protected:
    std::filesystem::path log_dir;

    void SetUp() override {
        log_dir = std::filesystem::temp_directory_path() / "frequent-cron-test-logs";
        std::filesystem::remove_all(log_dir);
    }

    void TearDown() override {
        std::filesystem::remove_all(log_dir);
    }
};

TEST_F(LogManagerTest, WritesLogFile) {
    LogManager lm(log_dir);
    lm.write("test-svc", "hello world");

    auto path = log_dir / "test-svc.log";
    EXPECT_TRUE(std::filesystem::exists(path));

    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("hello world"), std::string::npos);
}

TEST_F(LogManagerTest, AppendsToExistingLog) {
    LogManager lm(log_dir);
    lm.write("test-svc", "line1");
    lm.write("test-svc", "line2");

    auto content = lm.read_log("test-svc");
    EXPECT_NE(content.find("line1"), std::string::npos);
    EXPECT_NE(content.find("line2"), std::string::npos);
}

TEST_F(LogManagerTest, ReadLogReturnsEmpty) {
    LogManager lm(log_dir);
    auto content = lm.read_log("nonexistent");
    EXPECT_TRUE(content.empty());
}

TEST_F(LogManagerTest, ReadLogLastNLines) {
    LogManager lm(log_dir);
    for( int i = 0; i < 20; i++ ){
        lm.write("test-svc", "line " + std::to_string(i));
    }

    auto content = lm.read_log("test-svc", 5);
    // Should contain the last 5 lines
    EXPECT_NE(content.find("line 15"), std::string::npos);
    EXPECT_NE(content.find("line 19"), std::string::npos);
    EXPECT_EQ(content.find("line 0]"), std::string::npos);
}

TEST_F(LogManagerTest, RotatesWhenSizeExceeded) {
    // Use tiny max size to trigger rotation
    LogManager lm(log_dir, 100, 3);

    // Write enough to exceed 100 bytes
    for( int i = 0; i < 10; i++ ){
        lm.write("test-svc", "this is a log line that should trigger rotation when enough are written " + std::to_string(i));
    }

    auto rotated = log_dir / "test-svc.log.1";
    EXPECT_TRUE(std::filesystem::exists(rotated));
}

TEST_F(LogManagerTest, IncludesTimestamp) {
    LogManager lm(log_dir);
    lm.write("test-svc", "timestamped");

    auto content = lm.read_log("test-svc");
    // Should contain a timestamp like [2026-04-05 ...]
    EXPECT_NE(content.find("[20"), std::string::npos);
}

TEST_F(LogManagerTest, EmptyOutputNotWritten) {
    LogManager lm(log_dir);
    lm.write("test-svc", "");

    auto path = log_dir / "test-svc.log";
    EXPECT_FALSE(std::filesystem::exists(path));
}
