#include <gtest/gtest.h>
#include "config.h"

// === Legacy mode (backward compat) ===

TEST(Config, LegacyHelpReturnsHelp) {
    auto output = parse_args({"--help"});
    EXPECT_EQ(output.result, ParseResult::HELP);
}

TEST(Config, LegacyMissingFrequencyReturnsError) {
    auto output = parse_args({"--command=echo hi"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

TEST(Config, LegacyMissingCommandReturnsError) {
    auto output = parse_args({"--frequency=1000"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

TEST(Config, LegacyValidArgs) {
    auto output = parse_args({"--frequency=500", "--command=echo hello"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::LEGACY);
    EXPECT_EQ(output.config.frequency, 500);
    EXPECT_EQ(output.config.command, "echo hello");
    EXPECT_TRUE(output.config.synchronous);
    EXPECT_FALSE(output.config.has_pid_file);
}

TEST(Config, LegacyPidFile) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--pid-file=/tmp/test.pid"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_TRUE(output.config.has_pid_file);
    EXPECT_EQ(output.config.pid_filename, "/tmp/test.pid");
}

TEST(Config, LegacySynchronousDefaultsToTrue) {
    auto output = parse_args({"--frequency=100", "--command=echo hi"});
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, LegacySynchronousFalse) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=false"});
    EXPECT_FALSE(output.config.synchronous);
}

TEST(Config, LegacySynchronousTrue) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=true"});
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, LegacySynchronousTRUE) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=TRUE"});
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, LegacySynchronousOne) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=1"});
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, LegacySynchronousZero) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=0"});
    EXPECT_FALSE(output.config.synchronous);
}

TEST(Config, LegacyUnknownOption) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--bogus"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

// === run subcommand ===

TEST(Config, RunSubcommand) {
    auto output = parse_args({"run", "--frequency=200", "--command=echo run"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::RUN);
    EXPECT_EQ(output.config.frequency, 200);
    EXPECT_EQ(output.config.command, "echo run");
}

TEST(Config, RunHelp) {
    auto output = parse_args({"run", "--help"});
    EXPECT_EQ(output.result, ParseResult::HELP);
}

TEST(Config, RunMissingFrequency) {
    auto output = parse_args({"run", "--command=echo hi"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

// === install subcommand ===

TEST(Config, InstallSubcommand) {
    auto output = parse_args({"install", "myservice", "--frequency=500", "--command=echo hi"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::INSTALL);
    EXPECT_EQ(output.config.service_name, "myservice");
    EXPECT_EQ(output.config.frequency, 500);
    EXPECT_EQ(output.config.command, "echo hi");
}

TEST(Config, InstallMissingName) {
    auto output = parse_args({"install", "--frequency=500", "--command=echo hi"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

TEST(Config, InstallMissingFrequency) {
    auto output = parse_args({"install", "myservice", "--command=echo hi"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

TEST(Config, InstallMissingCommand) {
    auto output = parse_args({"install", "myservice", "--frequency=500"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

// === remove subcommand ===

TEST(Config, RemoveSubcommand) {
    auto output = parse_args({"remove", "myservice"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::REMOVE);
    EXPECT_EQ(output.config.service_name, "myservice");
}

TEST(Config, RemoveMissingName) {
    auto output = parse_args({"remove"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

// === start subcommand ===

TEST(Config, StartSubcommand) {
    auto output = parse_args({"start", "myservice"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::START);
    EXPECT_EQ(output.config.service_name, "myservice");
}

TEST(Config, StartMissingName) {
    auto output = parse_args({"start"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

// === stop subcommand ===

TEST(Config, StopSubcommand) {
    auto output = parse_args({"stop", "myservice"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::STOP);
    EXPECT_EQ(output.config.service_name, "myservice");
}

// === status subcommand ===

TEST(Config, StatusWithName) {
    auto output = parse_args({"status", "myservice"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::STATUS);
    EXPECT_EQ(output.config.service_name, "myservice");
}

TEST(Config, StatusWithoutName) {
    auto output = parse_args({"status"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::STATUS);
    EXPECT_TRUE(output.config.service_name.empty());
}

// === list subcommand ===

TEST(Config, ListSubcommand) {
    auto output = parse_args({"list"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::LIST);
}

// === logs subcommand ===

TEST(Config, LogsSubcommand) {
    auto output = parse_args({"logs", "myservice"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.subcommand, Subcommand::LOGS);
    EXPECT_EQ(output.config.service_name, "myservice");
}

TEST(Config, LogsMissingName) {
    auto output = parse_args({"logs"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

// === data-dir option ===

TEST(Config, DataDirOption) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--data-dir=/tmp/mydata"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.data_dir, "/tmp/mydata");
}

// === jitter and probability options ===

TEST(Config, JitterDefaultsToZero) {
    auto output = parse_args({"--frequency=100", "--command=echo hi"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.jitter_ms, 0);
    EXPECT_EQ(output.config.jitter_distribution, "uniform");
    EXPECT_DOUBLE_EQ(output.config.fire_probability, 1.0);
}

TEST(Config, JitterParsedForRun) {
    auto output = parse_args({"run", "--frequency=5000", "--command=echo hi", "--jitter=2000"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.jitter_ms, 2000);
}

TEST(Config, JitterDistributionNormal) {
    auto output = parse_args({"run", "--frequency=5000", "--command=echo hi", "--jitter=1000", "--jitter-distribution=normal"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.jitter_distribution, "normal");
}

TEST(Config, FireProbabilityParsedForRun) {
    auto output = parse_args({"run", "--frequency=1000", "--command=echo hi", "--fire-probability=0.4"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_DOUBLE_EQ(output.config.fire_probability, 0.4);
}

TEST(Config, JitterParsedForInstall) {
    auto output = parse_args({"install", "myservice", "--frequency=5000", "--command=echo hi", "--jitter=2000", "--jitter-distribution=normal"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.jitter_ms, 2000);
    EXPECT_EQ(output.config.jitter_distribution, "normal");
}

TEST(Config, FireProbabilityParsedForInstall) {
    auto output = parse_args({"install", "myservice", "--frequency=1000", "--command=echo hi", "--fire-probability=0.5"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_DOUBLE_EQ(output.config.fire_probability, 0.5);
}

TEST(Config, AllVarianceOptionsCombined) {
    auto output = parse_args({"run", "--frequency=300000", "--command=echo hi", "--jitter=120000", "--jitter-distribution=uniform", "--fire-probability=0.8"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.jitter_ms, 120000);
    EXPECT_EQ(output.config.jitter_distribution, "uniform");
    EXPECT_DOUBLE_EQ(output.config.fire_probability, 0.8);
}

// === unknown subcommand ===

TEST(Config, UnknownSubcommand) {
    auto output = parse_args({"bogus"});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}

// === no args ===

TEST(Config, NoArgs) {
    auto output = parse_args({});
    EXPECT_EQ(output.result, ParseResult::PARSE_ERROR);
}
