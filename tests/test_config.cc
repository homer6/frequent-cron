#include <gtest/gtest.h>
#include "config.h"

TEST(Config, HelpReturnsHelp) {
    auto output = parse_args({"--help"});
    EXPECT_EQ(output.result, ParseResult::HELP);
    EXPECT_TRUE(output.message.find("Options") != std::string::npos);
}

TEST(Config, MissingFrequencyReturnsError) {
    auto output = parse_args({"--command=echo hi"});
    EXPECT_EQ(output.result, ParseResult::ERROR);
    EXPECT_TRUE(output.message.find("Frequency") != std::string::npos);
}

TEST(Config, MissingCommandReturnsError) {
    auto output = parse_args({"--frequency=1000"});
    EXPECT_EQ(output.result, ParseResult::ERROR);
    EXPECT_TRUE(output.message.find("Command") != std::string::npos);
}

TEST(Config, ValidArgsReturnsOK) {
    auto output = parse_args({"--frequency=500", "--command=echo hello"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_EQ(output.config.frequency, 500);
    EXPECT_EQ(output.config.command, "echo hello");
    EXPECT_TRUE(output.config.synchronous);
    EXPECT_FALSE(output.config.has_pid_file);
}

TEST(Config, PidFileIsParsed) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--pid-file=/tmp/test.pid"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_TRUE(output.config.has_pid_file);
    EXPECT_EQ(output.config.pid_filename, "/tmp/test.pid");
}

TEST(Config, SynchronousDefaultsToTrue) {
    auto output = parse_args({"--frequency=100", "--command=echo hi"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, SynchronousImplicitValueIsTrue) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, SynchronousTrueExplicit) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=true"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, SynchronousTrueUppercase) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=TRUE"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, SynchronousTrueMixed) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=True"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, SynchronousOne) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=1"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_TRUE(output.config.synchronous);
}

TEST(Config, SynchronousFalse) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=false"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_FALSE(output.config.synchronous);
}

TEST(Config, SynchronousZero) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--synchronous=0"});
    EXPECT_EQ(output.result, ParseResult::OK);
    EXPECT_FALSE(output.config.synchronous);
}

TEST(Config, UnknownOptionReturnsError) {
    auto output = parse_args({"--frequency=100", "--command=echo hi", "--bogus"});
    EXPECT_EQ(output.result, ParseResult::ERROR);
}
