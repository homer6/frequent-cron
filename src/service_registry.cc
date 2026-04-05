#include "service_registry.h"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <signal.h>
    #include <unistd.h>
    #ifdef __APPLE__
        #include <mach-o/dyld.h>
    #endif
#endif


ServiceRegistry::ServiceRegistry( const std::filesystem::path& data_dir )
    : data_dir_(data_dir)
    , db_(data_dir / "frequent-cron.db")
    , platform_(PlatformService::create())
{
}

std::filesystem::path ServiceRegistry::get_binary_path(){
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    return std::filesystem::path(path);
#elif defined(__APPLE__)
    char path[4096];
    uint32_t size = sizeof(path);
    if( _NSGetExecutablePath(path, &size) == 0 ){
        return std::filesystem::path(path);
    }
    return "frequent-cron";
#else
    char path[4096];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if( len > 0 ){
        path[len] = '\0';
        return std::filesystem::path(path);
    }
    return "frequent-cron";
#endif
}

bool ServiceRegistry::is_process_running( int pid ){
    if( pid <= 0 ) return false;

#ifdef _WIN32
    HANDLE process = OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid );
    if( process ){
        CloseHandle(process);
        return true;
    }
    return false;
#else
    return kill( static_cast<pid_t>(pid), 0 ) == 0;
#endif
}

std::string ServiceRegistry::get_actual_status( const std::string& name ){
    auto state = db_.get_state(name);
    if( !state ) return "unknown";

    if( state->status == "running" && state->pid > 0 ){
        if( !is_process_running(state->pid) ){
            // Process died -- update DB
            ServiceState updated = *state;
            updated.status = "stopped";
            updated.pid = 0;
            db_.update_state(name, updated);
            return "stopped";
        }
        return "running";
    }
    return state->status;
}


int ServiceRegistry::cmd_install( const Config& config ){

    auto existing = db_.get_service(config.service_name);
    if( existing ){
        std::cerr << "Service '" << config.service_name << "' already exists. Use 'remove' first.\n";
        return 1;
    }

    ServiceRecord rec;
    rec.name = config.service_name;
    rec.command = config.command;
    rec.frequency_ms = config.frequency;
    rec.synchronous = config.synchronous;

    if( !db_.insert_service(rec) ){
        std::cerr << "Failed to install service '" << config.service_name << "'.\n";
        return 1;
    }

    // Install platform-native service definition
    if( platform_ ){
        platform_->install(config.service_name, rec, get_binary_path(), data_dir_);
    }

    std::cout << "Service '" << config.service_name << "' installed.\n";
    std::cout << "  command:     " << config.command << "\n";
    std::cout << "  frequency:   " << config.frequency << "ms\n";
    std::cout << "  synchronous: " << (config.synchronous ? "true" : "false") << "\n";
    return 0;
}


int ServiceRegistry::cmd_remove( const std::string& name ){

    auto existing = db_.get_service(name);
    if( !existing ){
        std::cerr << "Service '" << name << "' not found.\n";
        return 1;
    }

    // Stop if running
    auto status = get_actual_status(name);
    if( status == "running" ){
        cmd_stop(name);
    }

    // Remove platform-native service definition
    if( platform_ ){
        platform_->uninstall(name);
    }

    if( !db_.remove_service(name) ){
        std::cerr << "Failed to remove service '" << name << "'.\n";
        return 1;
    }

    // Clean up PID file
    auto pid_path = data_dir_ / "pids" / (name + ".pid");
    std::filesystem::remove(pid_path);

    // Clean up log files
    auto log_path = data_dir_ / "logs" / (name + ".log");
    for( int i = 0; i < 10; i++ ){
        auto rotated = data_dir_ / "logs" / (name + ".log." + std::to_string(i));
        std::filesystem::remove(rotated);
    }
    std::filesystem::remove(log_path);

    std::cout << "Service '" << name << "' removed.\n";
    return 0;
}


int ServiceRegistry::cmd_start( const std::string& name ){

    auto service = db_.get_service(name);
    if( !service ){
        std::cerr << "Service '" << name << "' not found.\n";
        return 1;
    }

    auto status = get_actual_status(name);
    if( status == "running" ){
        std::cerr << "Service '" << name << "' is already running.\n";
        return 1;
    }

    auto pid_path = data_dir_ / "pids" / (name + ".pid");
    auto bin_path = get_binary_path();

    // Build command to launch frequent-cron in run mode
    std::ostringstream cmd;
    cmd << "\"" << bin_path.string() << "\""
        << " run"
        << " --frequency=" << service->frequency_ms
        << " --command=\"" << service->command << "\""
        << " --pid-file=\"" << pid_path.string() << "\"";

    if( !service->synchronous ){
        cmd << " --synchronous=false";
    }

    // Launch in background
#ifdef _WIN32
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    std::string cmd_str = cmd.str();
    if( !CreateProcessA(nullptr, cmd_str.data(), nullptr, nullptr, FALSE,
                        CREATE_NO_WINDOW | DETACHED_PROCESS, nullptr, nullptr, &si, &pi) ){
        std::cerr << "Failed to start service '" << name << "'.\n";
        return 1;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    std::string full_cmd = cmd.str() + " &";
    int rc = system(full_cmd.c_str());
    if( rc != 0 ){
        std::cerr << "Failed to start service '" << name << "'.\n";
        return 1;
    }
#endif

    // Wait briefly for PID file to appear
    for( int i = 0; i < 10; i++ ){
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
        if( std::filesystem::exists(pid_path) ) break;
    }

    // Read PID and update state
    int pid = 0;
    if( std::filesystem::exists(pid_path) ){
        std::ifstream pf(pid_path);
        pf >> pid;
    }

    ServiceState st;
    st.status = "running";
    st.pid = pid;
    st.last_started_at = "now";  // SQLite will use COALESCE
    db_.update_state(name, st);

    std::cout << "Service '" << name << "' started (pid " << pid << ").\n";
    return 0;
}


int ServiceRegistry::cmd_stop( const std::string& name ){

    auto service = db_.get_service(name);
    if( !service ){
        std::cerr << "Service '" << name << "' not found.\n";
        return 1;
    }

    auto state = db_.get_state(name);
    if( !state || state->pid <= 0 ){
        // Try reading PID from file
        auto pid_path = data_dir_ / "pids" / (name + ".pid");
        if( std::filesystem::exists(pid_path) ){
            std::ifstream pf(pid_path);
            int pid = 0;
            pf >> pid;
            if( pid > 0 ){
                ServiceState tmp;
                tmp.status = "running";
                tmp.pid = pid;
                state = tmp;
            }
        }
    }

    if( !state || state->pid <= 0 || !is_process_running(state->pid) ){
        std::cerr << "Service '" << name << "' is not running.\n";
        // Update state to stopped
        ServiceState st;
        st.status = "stopped";
        st.pid = 0;
        db_.update_state(name, st);
        return 1;
    }

#ifdef _WIN32
    HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, state->pid);
    if( process ){
        TerminateProcess(process, 0);
        CloseHandle(process);
    }
#else
    kill(static_cast<pid_t>(state->pid), SIGTERM);
#endif

    // Wait for process to stop
    for( int i = 0; i < 20; i++ ){
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
        if( !is_process_running(state->pid) ) break;
    }

    ServiceState st;
    st.status = "stopped";
    st.pid = 0;
    st.last_stopped_at = "now";
    db_.update_state(name, st);

    // Clean up PID file
    auto pid_path = data_dir_ / "pids" / (name + ".pid");
    std::filesystem::remove(pid_path);

    std::cout << "Service '" << name << "' stopped.\n";
    return 0;
}


int ServiceRegistry::cmd_status( const std::string& name ){

    if( name.empty() ){
        // Show all
        auto services = db_.list_services();
        if( services.empty() ){
            std::cout << "No services registered.\n";
            return 0;
        }
        std::cout << std::left
                  << std::setw(20) << "NAME"
                  << std::setw(10) << "STATUS"
                  << std::setw(8)  << "PID"
                  << std::setw(12) << "FREQ(ms)"
                  << "COMMAND" << "\n";
        std::cout << std::string(80, '-') << "\n";

        for( const auto& svc : services ){
            auto status = get_actual_status(svc.name);
            auto state = db_.get_state(svc.name);
            int pid = state ? state->pid : 0;

            std::cout << std::left
                      << std::setw(20) << svc.name
                      << std::setw(10) << status
                      << std::setw(8)  << (pid > 0 ? std::to_string(pid) : "-")
                      << std::setw(12) << svc.frequency_ms
                      << svc.command << "\n";
        }
        return 0;
    }

    // Show specific service
    auto service = db_.get_service(name);
    if( !service ){
        std::cerr << "Service '" << name << "' not found.\n";
        return 1;
    }

    auto status = get_actual_status(name);
    auto state = db_.get_state(name);

    std::cout << "Service:     " << service->name << "\n";
    std::cout << "Status:      " << status << "\n";
    if( state && state->pid > 0 && status == "running" ){
        std::cout << "PID:         " << state->pid << "\n";
    }
    std::cout << "Command:     " << service->command << "\n";
    std::cout << "Frequency:   " << service->frequency_ms << "ms\n";
    std::cout << "Synchronous: " << (service->synchronous ? "true" : "false") << "\n";
    std::cout << "Created:     " << service->created_at << "\n";
    if( state ){
        if( !state->last_started_at.empty() )
            std::cout << "Last start:  " << state->last_started_at << "\n";
        if( !state->last_stopped_at.empty() )
            std::cout << "Last stop:   " << state->last_stopped_at << "\n";
    }

    return 0;
}


int ServiceRegistry::cmd_list(){

    auto services = db_.list_services();
    if( services.empty() ){
        std::cout << "No services registered.\n";
        return 0;
    }

    std::cout << std::left
              << std::setw(20) << "NAME"
              << std::setw(10) << "STATUS"
              << std::setw(12) << "FREQ(ms)"
              << "COMMAND" << "\n";
    std::cout << std::string(60, '-') << "\n";

    for( const auto& svc : services ){
        auto status = get_actual_status(svc.name);
        std::cout << std::left
                  << std::setw(20) << svc.name
                  << std::setw(10) << status
                  << std::setw(12) << svc.frequency_ms
                  << svc.command << "\n";
    }

    return 0;
}


int ServiceRegistry::cmd_logs( const std::string& name ){

    auto service = db_.get_service(name);
    if( !service ){
        std::cerr << "Service '" << name << "' not found.\n";
        return 1;
    }

    auto log_path = data_dir_ / "logs" / (name + ".log");
    if( !std::filesystem::exists(log_path) ){
        std::cout << "No logs for service '" << name << "'.\n";
        return 0;
    }

    std::ifstream log_file(log_path);
    std::cout << log_file.rdbuf();
    return 0;
}
