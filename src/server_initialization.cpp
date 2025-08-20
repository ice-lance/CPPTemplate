#include "server_initialization.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include "color_utils.h"
#include "utils.hpp"
#include "json.hpp"
#include <api_handler.h>
#include "ConfigManager.h"

extern volatile sig_atomic_t g_running;

// 版本信息常量
const std::string SOFTWARE_NAME = "ProtoFlow_Nexus";
const std::string VERSION = "1.3.0";
const std::string PROTOCOL_VERSION = "2";
const std::string DB_SCHEMA_VERSION = "v5";
const std::string BUILD_DATE = __DATE__ " " __TIME__;
const std::string GIT_COMMIT = "a1b2c3d4"; // 可通过构建脚本自动更新
const std::string BUILD_TYPE =
#ifdef NDEBUG
    "Release"
#else
    "Debug"
#endif
    ;

// 版本信息打印函数
void printVersionInfo()
{
    std::cout << SOFTWARE_NAME << " v" << VERSION << "\n";
    std::cout << "ProtoFlow Nexus for Financial Management\n\n";

    std::cout << "Version Information:\n";
    std::cout << "  Core Version:    " << VERSION << "\n";
    std::cout << "  Protocol Version: " << PROTOCOL_VERSION << "\n";
    std::cout << "  Database Schema:  " << DB_SCHEMA_VERSION << "\n";
    std::cout << "  Build Date:      " << BUILD_DATE << "\n";
    std::cout << "  Git Commit:      " << GIT_COMMIT << "\n";
    std::cout << "  Build Type:      " << BUILD_TYPE << "\n\n";

    std::cout << "System Capabilities:\n";

    std::cout << "Copyright © 2024 ProtoFlow_Nexus Systems Ltd.\n";
    std::cout << "License: MIT Open Source License\n";
    std::cout << "Website: https://hdxx.com\n";
}

// 从主配置创建运行配置
RuntimeConfig RuntimeConfig::fromServerConfig(const AppConfig &config)
{
    RuntimeConfig runtime;
    runtime.ip = config.apiHttp.ip;
    runtime.port = config.apiHttp.port;
    // 可以在这里添加其他字段的映射
    return runtime;
}

std::pair<AppConfig, RuntimeConfig> load_configuration(const std::string &config_path)
{
    RuntimeConfig runtimeConfig;
    AppConfig appConfig;
    try
    {
        // 使用 ConfigManager 的静态方法加载配置 - 更新为新的方法名
        appConfig = ConfigManager::loadConfigFromFile(config_path);
        runtimeConfig = RuntimeConfig::fromServerConfig(appConfig);
        SPDLOG_INFO("成功加载配置文件: {}", config_path);
    }
    catch (const std::exception &e)
    {
        SPDLOG_ERROR("配置文件加载失败: {}", e.what());
        SPDLOG_WARN("使用默认配置");
        // 设置默认配置
        appConfig.name = "cppprojectname";
        appConfig.version = 2;
        appConfig.id = "d24391b2dc714fe6a11e97712390483d";
        appConfig.apiHttp.ip = "0.0.0.0";
        appConfig.apiHttp.port = 8080;
        runtimeConfig = RuntimeConfig::fromServerConfig(appConfig);
    }
    return {appConfig, runtimeConfig};
}

CommandLineOptions parse_command_line(int argc, char *argv[])
{
    CommandLineOptions options;
    options.config_path = "config.yaml"; // 默认配置文件路径

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--version")
        {
            printVersionInfo();
            options.show_version = true;
        }
        else if (arg == "-h" || arg == "--help")
        {
            std::cout << "用法: " << argv[0] << " [选项]" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  --version        显示版本信息" << std::endl;
            std::cout << "  -c, --config <路径>  指定配置文件 (默认: config.yaml)" << std::endl;
            std::cout << "  -h, --help           显示帮助信息" << std::endl;
            options.show_help = true;
        }
        else if ((arg == "-c" || arg == "--config") && i + 1 < argc)
        {
            options.config_path = argv[++i];
        }
    }

    return options;
}

void printBanner()
{
    std::cout << R"(
  _____                   _             ______   _                        _   _
 |  __ \                 | |           |  ____| | |                      | \ | |
 | |__) |  _ __    ___   | |_    ___   | |__    | |   ___   __      __   |  \| |   ___  __  __  _   _   ___
 |  ___/  | '__|  / _ \  | __|  / _ \  |  __|   | |  / _ \  \ \ /\ / /   | . ` |  / _ \ \ \/ / | | | | / __|
 | |      | |    | (_) | | |_  | (_) | | |      | | | (_) |  \ V  V /    | |\  | |  __/  >  <  | |_| | \__ \
 |_|      |_|     \___/   \__|  \___/  |_|      |_|  \___/    \_/\_/     |_| \_|  \___| /_/\_\  \__,_| |___/
                                                                                                            
    )" << std::endl;

    std::cout << " ProtoFlow Nexus Server v1.2" << std::endl;
    std::cout << "==============================" << std::endl;
}

// 使用结构体封装状态
struct TrackingState
{
    std::mutex mutex;
    std::unordered_map<uintptr_t, std::chrono::steady_clock::time_point> request_times;
};

TrackingState tracking_state;

void setup_http_server(httplib::Server &server, const RuntimeConfig &config)
{
    // 设置日志级别
    if (config.log_level == "debug")
    {
        spdlog::set_level(spdlog::level::debug);
    }
    else if (config.log_level == "warn")
    {
        spdlog::set_level(spdlog::level::warn);
    } // 其他级别保持默认info

    // 注册 API 处理器
    registerBlockchainAPI(server);

    // 设置静态文件服务（使用配置的目录）
    server.set_mount_point("/", config.static_dir.c_str());

    // 设置 404 处理器
    server.set_error_handler([](const httplib::Request &req, httplib::Response &res)
                             {
        nlohmann::json response = {
            {"code", 999},
            {"status", "error"},
            {"message", "Endpoint not found"},
            {"path", req.path}
        };
        res.set_content(response.dump(), "application/json"); });

    // 请求时间追踪
    std::mutex request_data_mutex;
    std::unordered_map<uintptr_t, RequestData> request_data_map;

    // 前置中间件：记录请求开始时间
    server.set_pre_routing_handler([&](const httplib::Request &req, httplib::Response &)
                                   {
    try {
        std::lock_guard<std::mutex> lock(tracking_state.mutex);
        tracking_state.request_times[reinterpret_cast<uintptr_t>(&req)] =
            std::chrono::steady_clock::now();
    } catch (const std::system_error& e) {
        std::cerr << "PRE Error ["
                  << e.code().value() << "]: "
                  << e.what() << "\n";
    }
    return httplib::Server::HandlerResponse::Unhandled; });

    // 后置日志记录器
    server.set_logger([&](const httplib::Request &req, const httplib::Response &res)
                      {
        // 获取请求开始时间并清理记录
        std::chrono::steady_clock::time_point start_time;
        {
            std::lock_guard<std::mutex> lock(tracking_state.mutex);
            uintptr_t key = reinterpret_cast<uintptr_t>(&req);
            auto it = tracking_state.request_times.find(key);
            if (it == tracking_state.request_times.end()) {
                // 没有找到记录，可能是前置中间件未执行或已清理
                return httplib::Server::HandlerResponse::Unhandled;
            }
            start_time = it->second;
            tracking_state.request_times.erase(it);
        }
        // 计算持续时间
        auto duration = std::chrono::steady_clock::now() - start_time;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        // 提取客户端IP
        std::string client_ip = req.remote_addr;
        if (client_ip.find(':') != std::string::npos &&
            client_ip[0] == '[' && client_ip.back() == ']') {
            client_ip = client_ip.substr(1, client_ip.size() - 2);
        }
        // 构建完整路径
        std::string full_path = req.path;
        if (!req.params.empty()) {
            full_path += "?";
            bool first_param = true;
            for (const auto& param : req.params) {
                if (!first_param) full_path += "&";
                first_param = false;
                // 敏感参数过滤
                if (param.first == "password" || param.first == "token" ||
                    param.first == "api_key" || param.first == "secret") {
                    full_path += param.first + "=[FILTERED]";
                } else {
                    full_path += param.first + "=" + param.second;
                }
            }
        }
        // 使用自定义颜色
        auto colored_method = colorize_text(req.method, ansi::magenta);
        auto colored_status = colorize_text(std::to_string(res.status),
                                           res.status >= 500 ? ansi::red :
                                           res.status >= 400 ? ansi::yellow :
                                           ansi::green);
        // 根据状态选择日志级别
    if (res.status >= 500) {
        SPDLOG_ERROR("{} - \"{} {}\" {} ({}ms) {}",
                     client_ip, colored_method, full_path,
                     colored_status, ms, req.body.empty() ? "{}" : req.body);
    } else if (res.status >= 400) {
        SPDLOG_WARN("{} - \"{} {}\" {} ({}ms) {}",
                    client_ip, colored_method, full_path,
                    colored_status, ms, req.body.empty() ? "{}" : req.body);
    } else {
        SPDLOG_INFO("{} - \"{} {}\" {} ({}ms) {}",
                    client_ip, colored_method, full_path,
                    colored_status, ms, req.body.empty() ? "{}" : req.body);
    }
    // 返回值
    return httplib::Server::HandlerResponse::Unhandled; });
}

void run_server(httplib::Server &server, const RuntimeConfig &config)
{
    std::cout << std::endl;
    std::cout << "\033[36m[server]\033[0m"
              << "\033[36m在地址 \033[0m"
              << colorize_text(config.ip + ":" + utils::convert::toString(config.port),
                               ansi::white, ansi::bg_blue)
              << "\033[36m 上启动服务器...\033[0m" << std::endl;

    std::thread server_thread([&]()
                              { server.listen(config.ip.c_str(), config.port); });

    std::cout << "\033[33m[server]按Ctrl+C退出...\033[0m" << std::endl;
    server_thread.join();
}

void shutdown_system()
{
    std::cout << "正在停止服务器..." << std::endl;

    SPDLOG_INFO("应用程序正常退出");
    spdlog::shutdown();
    std::cout << "服务器关闭完成." << std::endl;
}