#include "data_dir.h"

#include <cstdlib>

#ifdef _WIN32
    #include <shlobj.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <pwd.h>
#endif

namespace data_dir {

std::filesystem::path get_default(){

#ifdef _WIN32
    // %LOCALAPPDATA%\frequent-cron
    char* local_app_data = std::getenv("LOCALAPPDATA");
    if( local_app_data ){
        return std::filesystem::path(local_app_data) / "frequent-cron";
    }
    return std::filesystem::path("C:\\ProgramData\\frequent-cron");

#elif defined(__APPLE__)
    // ~/Library/Application Support/frequent-cron
    const char* home = std::getenv("HOME");
    if( !home ){
        struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/tmp";
    }
    return std::filesystem::path(home) / "Library" / "Application Support" / "frequent-cron";

#else
    // Linux: $XDG_DATA_HOME/frequent-cron or ~/.local/share/frequent-cron
    // Root: /var/lib/frequent-cron
    if( getuid() == 0 ){
        return std::filesystem::path("/var/lib/frequent-cron");
    }
    const char* xdg = std::getenv("XDG_DATA_HOME");
    if( xdg && xdg[0] != '\0' ){
        return std::filesystem::path(xdg) / "frequent-cron";
    }
    const char* home = std::getenv("HOME");
    if( !home ){
        struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/tmp";
    }
    return std::filesystem::path(home) / ".local" / "share" / "frequent-cron";
#endif

}

std::filesystem::path ensure_exists( const std::filesystem::path& dir ){

    std::filesystem::create_directories( dir );
    std::filesystem::create_directories( dir / "logs" );
    std::filesystem::create_directories( dir / "pids" );
    return dir;

}

} // namespace data_dir
