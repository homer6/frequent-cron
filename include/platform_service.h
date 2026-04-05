#pragma once

#include "database.h"

#include <filesystem>
#include <memory>
#include <string>

class PlatformService {
public:
    virtual ~PlatformService() = default;

    // Install a platform-native service definition (systemd unit, launchd plist, SCM entry)
    virtual bool install( const std::string& name, const ServiceRecord& record,
                          const std::filesystem::path& binary_path,
                          const std::filesystem::path& data_dir ) = 0;

    // Remove the platform-native service definition
    virtual bool uninstall( const std::string& name ) = 0;

    // Start the service via the platform service manager
    virtual bool start( const std::string& name ) = 0;

    // Stop the service via the platform service manager
    virtual bool stop( const std::string& name ) = 0;

    // Factory: returns the correct implementation for the current platform
    static std::unique_ptr<PlatformService> create();
};
