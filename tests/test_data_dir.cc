#include <gtest/gtest.h>
#include "data_dir.h"

#include <filesystem>

TEST(DataDir, DefaultPathIsNotEmpty) {
    auto path = data_dir::get_default();
    EXPECT_FALSE(path.empty());
}

TEST(DataDir, DefaultPathContainsFrequentCron) {
    auto path = data_dir::get_default();
    EXPECT_NE(path.string().find("frequent-cron"), std::string::npos);
}

TEST(DataDir, EnsureExistsCreatesDirectories) {
    auto tmp = std::filesystem::temp_directory_path() / "frequent-cron-test-datadir";
    std::filesystem::remove_all(tmp);

    auto result = data_dir::ensure_exists(tmp);
    EXPECT_EQ(result, tmp);
    EXPECT_TRUE(std::filesystem::exists(tmp));
    EXPECT_TRUE(std::filesystem::exists(tmp / "logs"));
    EXPECT_TRUE(std::filesystem::exists(tmp / "pids"));

    std::filesystem::remove_all(tmp);
}

TEST(DataDir, EnsureExistsIsIdempotent) {
    auto tmp = std::filesystem::temp_directory_path() / "frequent-cron-test-datadir2";
    std::filesystem::remove_all(tmp);

    data_dir::ensure_exists(tmp);
    data_dir::ensure_exists(tmp);  // should not throw

    EXPECT_TRUE(std::filesystem::exists(tmp));
    std::filesystem::remove_all(tmp);
}
