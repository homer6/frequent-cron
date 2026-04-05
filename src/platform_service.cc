#include "platform_service.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <winsvc.h>
#else
    #include <unistd.h>
    #include <pwd.h>
#endif


// ============================================================
// Linux: systemd
// ============================================================
#if defined(__linux__)

class SystemdService : public PlatformService {
public:

    bool install( const std::string& name, const ServiceRecord& record,
                  const std::filesystem::path& binary_path,
                  const std::filesystem::path& data_dir ) override {

        auto unit_path = get_unit_path(name);
        auto pid_path = data_dir / "pids" / (name + ".pid");

        std::ostringstream unit;
        unit << "[Unit]\n"
             << "Description=frequent-cron: " << name << "\n"
             << "After=network.target\n\n"
             << "[Service]\n"
             << "Type=forking\n"
             << "ExecStart=" << binary_path.string()
             << " run"
             << " --frequency=" << record.frequency_ms
             << " --command=\"" << record.command << "\""
             << " --pid-file=\"" << pid_path.string() << "\"";

        if( !record.synchronous ){
            unit << " --synchronous=false";
        }

        unit << "\n"
             << "PIDFile=" << pid_path.string() << "\n"
             << "Restart=on-failure\n"
             << "RestartSec=5\n\n"
             << "[Install]\n"
             << "WantedBy=multi-user.target\n";

        std::filesystem::create_directories(unit_path.parent_path());
        std::ofstream file(unit_path);
        if( !file ){
            std::cerr << "Failed to write unit file: " << unit_path << "\n";
            return false;
        }
        file << unit.str();
        file.close();

        // Reload systemd
        system("systemctl daemon-reload 2>/dev/null");
        std::cout << "Systemd unit installed: " << unit_path << "\n";
        return true;
    }

    bool uninstall( const std::string& name ) override {
        auto unit_path = get_unit_path(name);

        // Stop and disable first
        std::string svc = get_service_name(name);
        system(("systemctl stop " + svc + " 2>/dev/null").c_str());
        system(("systemctl disable " + svc + " 2>/dev/null").c_str());

        if( std::filesystem::exists(unit_path) ){
            std::filesystem::remove(unit_path);
            system("systemctl daemon-reload 2>/dev/null");
            std::cout << "Systemd unit removed: " << unit_path << "\n";
        }
        return true;
    }

    bool start( const std::string& name ) override {
        std::string cmd = "systemctl start " + get_service_name(name);
        return system(cmd.c_str()) == 0;
    }

    bool stop( const std::string& name ) override {
        std::string cmd = "systemctl stop " + get_service_name(name);
        return system(cmd.c_str()) == 0;
    }

private:
    std::string get_service_name( const std::string& name ){
        return "frequent-cron-" + name + ".service";
    }

    std::filesystem::path get_unit_path( const std::string& name ){
        if( getuid() == 0 ){
            return std::filesystem::path("/etc/systemd/system") / get_service_name(name);
        }
        const char* home = std::getenv("HOME");
        if( !home ){
            struct passwd* pw = getpwuid(getuid());
            home = pw ? pw->pw_dir : "/tmp";
        }
        return std::filesystem::path(home) / ".config" / "systemd" / "user" / get_service_name(name);
    }
};

#endif // __linux__


// ============================================================
// macOS: launchd
// ============================================================
#if defined(__APPLE__)

class LaunchdService : public PlatformService {
public:

    bool install( const std::string& name, const ServiceRecord& record,
                  const std::filesystem::path& binary_path,
                  const std::filesystem::path& data_dir ) override {

        auto plist_path = get_plist_path(name);
        auto pid_path = data_dir / "pids" / (name + ".pid");

        std::ostringstream plist;
        plist << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
              << "<plist version=\"1.0\">\n"
              << "<dict>\n"
              << "    <key>Label</key>\n"
              << "    <string>" << get_label(name) << "</string>\n"
              << "    <key>ProgramArguments</key>\n"
              << "    <array>\n"
              << "        <string>" << binary_path.string() << "</string>\n"
              << "        <string>run</string>\n"
              << "        <string>--frequency=" << record.frequency_ms << "</string>\n"
              << "        <string>--command=" << record.command << "</string>\n"
              << "        <string>--pid-file=" << pid_path.string() << "</string>\n";

        if( !record.synchronous ){
            plist << "        <string>--synchronous=false</string>\n";
        }

        plist << "    </array>\n"
              << "    <key>RunAtLoad</key>\n"
              << "    <true/>\n"
              << "    <key>KeepAlive</key>\n"
              << "    <true/>\n"
              << "</dict>\n"
              << "</plist>\n";

        std::filesystem::create_directories(plist_path.parent_path());
        std::ofstream file(plist_path);
        if( !file ){
            std::cerr << "Failed to write plist: " << plist_path << "\n";
            return false;
        }
        file << plist.str();
        file.close();

        std::cout << "Launchd plist installed: " << plist_path << "\n";
        return true;
    }

    bool uninstall( const std::string& name ) override {
        // Unload first
        stop(name);

        auto plist_path = get_plist_path(name);
        if( std::filesystem::exists(plist_path) ){
            std::filesystem::remove(plist_path);
            std::cout << "Launchd plist removed: " << plist_path << "\n";
        }
        return true;
    }

    bool start( const std::string& name ) override {
        auto plist_path = get_plist_path(name);
        std::string cmd = "launchctl load \"" + plist_path.string() + "\"";
        return system(cmd.c_str()) == 0;
    }

    bool stop( const std::string& name ) override {
        auto plist_path = get_plist_path(name);
        std::string cmd = "launchctl unload \"" + plist_path.string() + "\" 2>/dev/null";
        system(cmd.c_str());
        return true;
    }

private:
    std::string get_label( const std::string& name ){
        return "com.frequent-cron." + name;
    }

    std::filesystem::path get_plist_path( const std::string& name ){
        if( getuid() == 0 ){
            return std::filesystem::path("/Library/LaunchDaemons") / (get_label(name) + ".plist");
        }
        const char* home = std::getenv("HOME");
        if( !home ){
            struct passwd* pw = getpwuid(getuid());
            home = pw ? pw->pw_dir : "/tmp";
        }
        return std::filesystem::path(home) / "Library" / "LaunchAgents" / (get_label(name) + ".plist");
    }
};

#endif // __APPLE__


// ============================================================
// Windows: SCM
// ============================================================
#if defined(_WIN32)

class ScmService : public PlatformService {
public:

    bool install( const std::string& name, const ServiceRecord& record,
                  const std::filesystem::path& binary_path,
                  const std::filesystem::path& data_dir ) override {

        auto pid_path = data_dir / "pids" / (name + ".pid");

        // Build command line
        std::ostringstream cmd;
        cmd << "\"" << binary_path.string() << "\""
            << " run"
            << " --frequency=" << record.frequency_ms
            << " --command=\"" << record.command << "\""
            << " --pid-file=\"" << pid_path.string() << "\"";

        if( !record.synchronous ){
            cmd << " --synchronous=false";
        }

        std::string svc_name = get_service_name(name);
        std::string display_name = "frequent-cron: " + name;
        std::string cmd_str = cmd.str();

        SC_HANDLE scm = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
        if( !scm ){
            std::cerr << "Failed to open Service Control Manager. Run as Administrator.\n";
            return false;
        }

        SC_HANDLE svc = CreateServiceA(
            scm,
            svc_name.c_str(),
            display_name.c_str(),
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            cmd_str.c_str(),
            nullptr, nullptr, nullptr, nullptr, nullptr
        );

        if( !svc ){
            std::cerr << "Failed to create service. Error: " << GetLastError() << "\n";
            CloseServiceHandle(scm);
            return false;
        }

        CloseServiceHandle(svc);
        CloseServiceHandle(scm);

        std::cout << "Windows service installed: " << svc_name << "\n";
        return true;
    }

    bool uninstall( const std::string& name ) override {
        std::string svc_name = get_service_name(name);

        SC_HANDLE scm = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CONNECT);
        if( !scm ) return false;

        SC_HANDLE svc = OpenServiceA(scm, svc_name.c_str(), DELETE | SERVICE_STOP);
        if( !svc ){
            CloseServiceHandle(scm);
            return false;
        }

        SERVICE_STATUS st{};
        ControlService(svc, SERVICE_CONTROL_STOP, &st);
        bool ok = DeleteService(svc) != 0;
        CloseServiceHandle(svc);
        CloseServiceHandle(scm);

        if( ok ){
            std::cout << "Windows service removed: " << svc_name << "\n";
        }
        return ok;
    }

    bool start( const std::string& name ) override {
        std::string cmd = "sc start " + get_service_name(name);
        return system(cmd.c_str()) == 0;
    }

    bool stop( const std::string& name ) override {
        std::string cmd = "sc stop " + get_service_name(name);
        return system(cmd.c_str()) == 0;
    }

private:
    std::string get_service_name( const std::string& name ){
        return "frequent-cron-" + name;
    }
};

#endif // _WIN32


// ============================================================
// FreeBSD: rc.d
// ============================================================
#if defined(__FreeBSD__)

class RcdService : public PlatformService {
public:

    bool install( const std::string& name, const ServiceRecord& record,
                  const std::filesystem::path& binary_path,
                  const std::filesystem::path& data_dir ) override {

        auto script_path = get_script_path(name);
        auto pid_path = data_dir / "pids" / (name + ".pid");
        auto svc_name = get_service_name(name);

        std::ostringstream script;
        script << "#!/bin/sh\n"
               << "#\n"
               << "# PROVIDE: " << svc_name << "\n"
               << "# REQUIRE: DAEMON\n"
               << "# KEYWORD: shutdown\n"
               << "\n"
               << ". /etc/rc.subr\n"
               << "\n"
               << "name=\"" << svc_name << "\"\n"
               << "rcvar=\"" << svc_name << "_enable\"\n"
               << "pidfile=\"" << pid_path.string() << "\"\n"
               << "command=\"" << binary_path.string() << "\"\n"
               << "command_args=\"run"
               << " --frequency=" << record.frequency_ms
               << " --command='" << record.command << "'"
               << " --pid-file=" << pid_path.string();

        if( !record.synchronous ){
            script << " --synchronous=false";
        }

        script << "\"\n"
               << "\n"
               << "load_rc_config $name\n"
               << "run_rc_command \"$1\"\n";

        std::filesystem::create_directories(script_path.parent_path());
        std::ofstream file(script_path);
        if( !file ){
            std::cerr << "Failed to write rc.d script: " << script_path << "\n";
            return false;
        }
        file << script.str();
        file.close();

        // Make executable
        std::filesystem::permissions(script_path,
            std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
            std::filesystem::perm_options::add);

        // Enable in rc.conf
        std::string enable_cmd = "sysrc " + svc_name + "_enable=YES 2>/dev/null";
        system(enable_cmd.c_str());

        std::cout << "rc.d script installed: " << script_path << "\n";
        return true;
    }

    bool uninstall( const std::string& name ) override {
        stop(name);

        auto script_path = get_script_path(name);
        auto svc_name = get_service_name(name);

        // Disable in rc.conf
        std::string disable_cmd = "sysrc -x " + svc_name + "_enable 2>/dev/null";
        system(disable_cmd.c_str());

        if( std::filesystem::exists(script_path) ){
            std::filesystem::remove(script_path);
            std::cout << "rc.d script removed: " << script_path << "\n";
        }
        return true;
    }

    bool start( const std::string& name ) override {
        std::string cmd = "service " + get_service_name(name) + " start";
        return system(cmd.c_str()) == 0;
    }

    bool stop( const std::string& name ) override {
        std::string cmd = "service " + get_service_name(name) + " stop 2>/dev/null";
        system(cmd.c_str());
        return true;
    }

private:
    std::string get_service_name( const std::string& name ){
        return "frequent_cron_" + name;
    }

    std::filesystem::path get_script_path( const std::string& name ){
        return std::filesystem::path("/usr/local/etc/rc.d") / get_service_name(name);
    }
};

#endif // __FreeBSD__


// ============================================================
// Factory
// ============================================================

std::unique_ptr<PlatformService> PlatformService::create(){
#if defined(__linux__)
    return std::make_unique<SystemdService>();
#elif defined(__APPLE__)
    return std::make_unique<LaunchdService>();
#elif defined(__FreeBSD__)
    return std::make_unique<RcdService>();
#elif defined(_WIN32)
    return std::make_unique<ScmService>();
#else
    return nullptr;
#endif
}
