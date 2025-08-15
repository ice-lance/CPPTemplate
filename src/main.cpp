
// main.cpp
#include "server_initialization.h"
#include "logging_system.h"
#include "system_info_display.h"
#include <mutex>

volatile sig_atomic_t g_running = true;
std::mutex log_mutex;

void signalHandler(int signum)
{
    std::lock_guard<std::mutex> lock(log_mutex); // 保护日志输出
    std::cout << "\n收到中断信号。正在关闭..." << signum << std::endl;
    g_running = false;
}

int main(int argc, char *argv[])
{
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try
    {
        // 解析命令行参数
        auto options = parse_command_line(argc, argv);

        // 如果用户请求了帮助或版本信息，直接退出
        if (options.show_help || options.show_version)
        {
            return 0;
        }

        // 打印欢迎信息
        printBanner();

        // 初始化日志系统
        auto logger = initialize_logging_system();

        // 加载配置文件
        RuntimeConfig config = load_configuration(options.config_path);
        
        // 创建 HTTP 服务器
        httplib::Server http_server;
        setup_http_server(http_server, config);

        // 显示系统信息
        display_system_info();

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        // 启动服务器
        run_server(http_server, config);

        // 等待终止信号
        while (g_running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // 停止HTTP服务器
        http_server.stop();

        // 关闭日志系统
        shutdown_logging_system();
    }
    catch (const std::exception &e)
    {
        std::lock_guard<std::mutex> lock(log_mutex); // 保护错误输出
        std::cerr << "严重错误: " << e.what() << std::endl;

        // 确保日志系统被关闭
        shutdown_logging_system();
        return 1;
    }

    return 0;
}