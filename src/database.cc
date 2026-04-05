#include "database.h"

#include <stdexcept>

Database::Database( const std::filesystem::path& db_path ){

    int rc = sqlite3_open( db_path.string().c_str(), &db_ );
    if( rc != SQLITE_OK ){
        std::string msg = "Failed to open database: ";
        msg += sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error(msg);
    }

    exec("PRAGMA journal_mode=WAL");
    exec("PRAGMA foreign_keys=ON");
    ensure_schema();
}

Database::~Database(){
    if( db_ ){
        sqlite3_close(db_);
    }
}

void Database::exec( const std::string& sql ){
    char* err = nullptr;
    int rc = sqlite3_exec( db_, sql.c_str(), nullptr, nullptr, &err );
    if( rc != SQLITE_OK ){
        std::string msg = "SQL error: ";
        msg += err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error(msg);
    }
}

void Database::ensure_schema(){

    exec(R"(
        CREATE TABLE IF NOT EXISTS services (
            name TEXT PRIMARY KEY,
            command TEXT NOT NULL,
            frequency_ms INTEGER NOT NULL,
            synchronous INTEGER NOT NULL DEFAULT 1,
            created_at TEXT NOT NULL DEFAULT (datetime('now')),
            updated_at TEXT NOT NULL DEFAULT (datetime('now'))
        )
    )");

    exec(R"(
        CREATE TABLE IF NOT EXISTS service_state (
            name TEXT PRIMARY KEY REFERENCES services(name) ON DELETE CASCADE,
            status TEXT NOT NULL DEFAULT 'stopped',
            pid INTEGER,
            last_started_at TEXT,
            last_stopped_at TEXT,
            execution_count INTEGER NOT NULL DEFAULT 0
        )
    )");
}

bool Database::insert_service( const ServiceRecord& record ){

    const char* sql = "INSERT INTO services (name, command, frequency_ms, synchronous) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;

    if( sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK ){
        return false;
    }

    sqlite3_bind_text( stmt, 1, record.name.c_str(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( stmt, 2, record.command.c_str(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_int( stmt, 3, record.frequency_ms );
    sqlite3_bind_int( stmt, 4, record.synchronous ? 1 : 0 );

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    if( ok ){
        // Insert initial state
        const char* state_sql = "INSERT INTO service_state (name) VALUES (?)";
        sqlite3_stmt* state_stmt = nullptr;
        if( sqlite3_prepare_v2(db_, state_sql, -1, &state_stmt, nullptr) == SQLITE_OK ){
            sqlite3_bind_text( state_stmt, 1, record.name.c_str(), -1, SQLITE_TRANSIENT );
            sqlite3_step(state_stmt);
            sqlite3_finalize(state_stmt);
        }
    }

    return ok;
}

bool Database::remove_service( const std::string& name ){

    const char* sql = "DELETE FROM services WHERE name = ?";
    sqlite3_stmt* stmt = nullptr;

    if( sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK ){
        return false;
    }

    sqlite3_bind_text( stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT );
    bool ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db_) > 0;
    sqlite3_finalize(stmt);
    return ok;
}

std::optional<ServiceRecord> Database::get_service( const std::string& name ){

    const char* sql = "SELECT name, command, frequency_ms, synchronous, created_at, updated_at FROM services WHERE name = ?";
    sqlite3_stmt* stmt = nullptr;

    if( sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK ){
        return std::nullopt;
    }

    sqlite3_bind_text( stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT );

    std::optional<ServiceRecord> result;
    if( sqlite3_step(stmt) == SQLITE_ROW ){
        ServiceRecord rec;
        rec.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        rec.command = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rec.frequency_ms = sqlite3_column_int(stmt, 2);
        rec.synchronous = sqlite3_column_int(stmt, 3) != 0;
        rec.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        rec.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        result = rec;
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<ServiceRecord> Database::list_services(){

    const char* sql = "SELECT name, command, frequency_ms, synchronous, created_at, updated_at FROM services ORDER BY name";
    sqlite3_stmt* stmt = nullptr;
    std::vector<ServiceRecord> results;

    if( sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK ){
        return results;
    }

    while( sqlite3_step(stmt) == SQLITE_ROW ){
        ServiceRecord rec;
        rec.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        rec.command = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rec.frequency_ms = sqlite3_column_int(stmt, 2);
        rec.synchronous = sqlite3_column_int(stmt, 3) != 0;
        rec.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        rec.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        results.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return results;
}

bool Database::update_service( const ServiceRecord& record ){

    const char* sql = "UPDATE services SET command = ?, frequency_ms = ?, synchronous = ?, updated_at = datetime('now') WHERE name = ?";
    sqlite3_stmt* stmt = nullptr;

    if( sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK ){
        return false;
    }

    sqlite3_bind_text( stmt, 1, record.command.c_str(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_int( stmt, 2, record.frequency_ms );
    sqlite3_bind_int( stmt, 3, record.synchronous ? 1 : 0 );
    sqlite3_bind_text( stmt, 4, record.name.c_str(), -1, SQLITE_TRANSIENT );

    bool ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db_) > 0;
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::update_state( const std::string& name, const ServiceState& state ){

    const char* sql = R"(
        UPDATE service_state SET
            status = ?,
            pid = ?,
            last_started_at = COALESCE(?, last_started_at),
            last_stopped_at = COALESCE(?, last_stopped_at),
            execution_count = ?
        WHERE name = ?
    )";
    sqlite3_stmt* stmt = nullptr;

    if( sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK ){
        return false;
    }

    sqlite3_bind_text( stmt, 1, state.status.c_str(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_int( stmt, 2, state.pid );

    if( state.last_started_at.empty() ){
        sqlite3_bind_null( stmt, 3 );
    }else{
        sqlite3_bind_text( stmt, 3, state.last_started_at.c_str(), -1, SQLITE_TRANSIENT );
    }

    if( state.last_stopped_at.empty() ){
        sqlite3_bind_null( stmt, 4 );
    }else{
        sqlite3_bind_text( stmt, 4, state.last_stopped_at.c_str(), -1, SQLITE_TRANSIENT );
    }

    sqlite3_bind_int( stmt, 5, state.execution_count );
    sqlite3_bind_text( stmt, 6, name.c_str(), -1, SQLITE_TRANSIENT );

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

std::optional<ServiceState> Database::get_state( const std::string& name ){

    const char* sql = "SELECT name, status, pid, last_started_at, last_stopped_at, execution_count FROM service_state WHERE name = ?";
    sqlite3_stmt* stmt = nullptr;

    if( sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK ){
        return std::nullopt;
    }

    sqlite3_bind_text( stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT );

    std::optional<ServiceState> result;
    if( sqlite3_step(stmt) == SQLITE_ROW ){
        ServiceState st;
        st.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        st.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        st.pid = sqlite3_column_int(stmt, 2);

        const char* started = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        st.last_started_at = started ? started : "";

        const char* stopped = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        st.last_stopped_at = stopped ? stopped : "";

        st.execution_count = sqlite3_column_int(stmt, 5);
        result = st;
    }

    sqlite3_finalize(stmt);
    return result;
}
