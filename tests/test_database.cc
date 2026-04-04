#include <gtest/gtest.h>
#include "database.h"

#include <filesystem>

class DatabaseTest : public ::testing::Test {
protected:
    std::filesystem::path db_path;

    void SetUp() override {
        db_path = std::filesystem::temp_directory_path() / "frequent-cron-test.db";
        std::filesystem::remove(db_path);
    }

    void TearDown() override {
        std::filesystem::remove(db_path);
    }
};

TEST_F(DatabaseTest, OpensAndCreatesSchema) {
    EXPECT_NO_THROW(Database db(db_path));
    EXPECT_TRUE(std::filesystem::exists(db_path));
}

TEST_F(DatabaseTest, InsertAndGetService) {
    Database db(db_path);
    ServiceRecord rec;
    rec.name = "test-service";
    rec.command = "echo hello";
    rec.frequency_ms = 1000;
    rec.synchronous = true;

    EXPECT_TRUE(db.insert_service(rec));

    auto result = db.get_service("test-service");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "test-service");
    EXPECT_EQ(result->command, "echo hello");
    EXPECT_EQ(result->frequency_ms, 1000);
    EXPECT_TRUE(result->synchronous);
    EXPECT_FALSE(result->created_at.empty());
}

TEST_F(DatabaseTest, InsertDuplicateFails) {
    Database db(db_path);
    ServiceRecord rec;
    rec.name = "dup";
    rec.command = "echo hi";
    rec.frequency_ms = 500;

    EXPECT_TRUE(db.insert_service(rec));
    EXPECT_FALSE(db.insert_service(rec));
}

TEST_F(DatabaseTest, RemoveService) {
    Database db(db_path);
    ServiceRecord rec;
    rec.name = "to-remove";
    rec.command = "echo bye";
    rec.frequency_ms = 100;

    db.insert_service(rec);
    EXPECT_TRUE(db.remove_service("to-remove"));
    EXPECT_FALSE(db.get_service("to-remove").has_value());
}

TEST_F(DatabaseTest, RemoveNonexistentReturnsFalse) {
    Database db(db_path);
    EXPECT_FALSE(db.remove_service("nonexistent"));
}

TEST_F(DatabaseTest, ListServices) {
    Database db(db_path);

    ServiceRecord a;
    a.name = "alpha";
    a.command = "echo a";
    a.frequency_ms = 100;
    db.insert_service(a);

    ServiceRecord b;
    b.name = "beta";
    b.command = "echo b";
    b.frequency_ms = 200;
    db.insert_service(b);

    auto list = db.list_services();
    EXPECT_EQ(list.size(), 2u);
    EXPECT_EQ(list[0].name, "alpha");
    EXPECT_EQ(list[1].name, "beta");
}

TEST_F(DatabaseTest, ListServicesEmpty) {
    Database db(db_path);
    auto list = db.list_services();
    EXPECT_TRUE(list.empty());
}

TEST_F(DatabaseTest, UpdateService) {
    Database db(db_path);
    ServiceRecord rec;
    rec.name = "updatable";
    rec.command = "echo old";
    rec.frequency_ms = 100;
    db.insert_service(rec);

    rec.command = "echo new";
    rec.frequency_ms = 200;
    EXPECT_TRUE(db.update_service(rec));

    auto result = db.get_service("updatable");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->command, "echo new");
    EXPECT_EQ(result->frequency_ms, 200);
}

TEST_F(DatabaseTest, GetServiceNotFound) {
    Database db(db_path);
    EXPECT_FALSE(db.get_service("nonexistent").has_value());
}

TEST_F(DatabaseTest, StateCreatedOnInsert) {
    Database db(db_path);
    ServiceRecord rec;
    rec.name = "with-state";
    rec.command = "echo hi";
    rec.frequency_ms = 100;
    db.insert_service(rec);

    auto state = db.get_state("with-state");
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->status, "stopped");
    EXPECT_EQ(state->pid, 0);
    EXPECT_EQ(state->execution_count, 0);
}

TEST_F(DatabaseTest, UpdateState) {
    Database db(db_path);
    ServiceRecord rec;
    rec.name = "stateful";
    rec.command = "echo hi";
    rec.frequency_ms = 100;
    db.insert_service(rec);

    ServiceState st;
    st.status = "running";
    st.pid = 12345;
    st.last_started_at = "2026-04-04 12:00:00";
    st.execution_count = 42;

    EXPECT_TRUE(db.update_state("stateful", st));

    auto result = db.get_state("stateful");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, "running");
    EXPECT_EQ(result->pid, 12345);
    EXPECT_EQ(result->last_started_at, "2026-04-04 12:00:00");
    EXPECT_EQ(result->execution_count, 42);
}

TEST_F(DatabaseTest, CascadeDeleteRemovesState) {
    Database db(db_path);
    ServiceRecord rec;
    rec.name = "cascade";
    rec.command = "echo hi";
    rec.frequency_ms = 100;
    db.insert_service(rec);

    EXPECT_TRUE(db.get_state("cascade").has_value());
    db.remove_service("cascade");
    EXPECT_FALSE(db.get_state("cascade").has_value());
}
