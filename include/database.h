#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <sqlite3.h>

struct ServiceRecord {
    std::string name;
    std::string command;
    int frequency_ms = 0;
    bool synchronous = true;
    int jitter_ms = 0;
    std::string jitter_distribution = "uniform";  // "uniform" or "normal"
    double fire_probability = 1.0;
    std::string created_at;
    std::string updated_at;
};

struct ServiceState {
    std::string name;
    std::string status;     // "running", "stopped", "failed"
    int pid = 0;
    std::string last_started_at;
    std::string last_stopped_at;
    int execution_count = 0;
};

class Database {
public:
    explicit Database( const std::filesystem::path& db_path );
    ~Database();

    Database( const Database& ) = delete;
    Database& operator=( const Database& ) = delete;

    // Service CRUD
    bool insert_service( const ServiceRecord& record );
    bool remove_service( const std::string& name );
    std::optional<ServiceRecord> get_service( const std::string& name );
    std::vector<ServiceRecord> list_services();
    bool update_service( const ServiceRecord& record );

    // State management
    bool update_state( const std::string& name, const ServiceState& state );
    std::optional<ServiceState> get_state( const std::string& name );

private:
    sqlite3* db_ = nullptr;
    void ensure_schema();
    void exec( const std::string& sql );
};
