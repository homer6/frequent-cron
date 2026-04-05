#pragma once

#include <filesystem>
#include <string>

namespace data_dir {

    // Returns the default data directory for the current platform and user.
    std::filesystem::path get_default();

    // Ensures the data directory and its subdirectories exist. Returns the path.
    std::filesystem::path ensure_exists( const std::filesystem::path& dir );

}
