#pragma once
#include <httplib.h>
#include <string>
#include "ServerConfig.h" // 包含新的配置头文件

struct RequestData
{
    std::chrono::steady_clock::time_point start_time;
};

// 服务器运行配置结构体
struct RuntimeConfig {
    std::string ip = "0.0.0.0";
    int port = 8080;
    std::string log_level = "info";
    std::string static_dir = "./public";
    bool show_help = false;
    
    // 从主配置加载
    static RuntimeConfig fromServerConfig(const ServerConfig& config);
};
struct CommandLineOptions {
    std::string config_path = "config.yaml";  // 配置文件路径
    bool show_help = false;                   // 是否显示帮助
    bool show_version = false;                // 是否显示版本信息
};

RuntimeConfig load_configuration(const std::string& config_path);
CommandLineOptions parse_command_line(int argc, char* argv[]);
void printVersionInfo();
void printBanner();
void setup_http_server(httplib::Server& server, const RuntimeConfig& config);
void run_server(httplib::Server& server, const RuntimeConfig& config);
void shutdown_system();