#pragma once

#include "config.h"
#include "database.h"
#include "data_dir.h"
#include "platform_service.h"

#include <filesystem>
#include <iostream>
#include <memory>

class ServiceRegistry {
public:
    explicit ServiceRegistry( const std::filesystem::path& data_dir );

    int cmd_install( const Config& config );
    int cmd_remove( const std::string& name );
    int cmd_start( const std::string& name );
    int cmd_stop( const std::string& name );
    int cmd_status( const std::string& name );
    int cmd_list();
    int cmd_logs( const std::string& name );

private:
    std::filesystem::path data_dir_;
    Database db_;
    std::unique_ptr<PlatformService> platform_;

    bool is_process_running( int pid );
    std::string get_actual_status( const std::string& name );
    std::filesystem::path get_binary_path();
};
