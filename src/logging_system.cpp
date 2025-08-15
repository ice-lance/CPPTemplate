#include "logging_system.h"
#include <filesystem>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <cstdlib>
#include "color_utils.h"

// ANSI 清理格式化器
class ANSICleaningFormatter : public spdlog::formatter
{
public:
    explicit ANSICleaningFormatter(const std::string &pattern = "[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v")
        : base_formatter_(std::make_unique<spdlog::pattern_formatter>(pattern)),
          pattern_(pattern) {}

    // 添加移动构造函数
    ANSICleaningFormatter(ANSICleaningFormatter &&other) noexcept
        : base_formatter_(std::move(other.base_formatter_)),
          pattern_(std::move(other.pattern_)) {}

    // 添加移动赋值运算符
    ANSICleaningFormatter &operator=(ANSICleaningFormatter &&other) noexcept
    {
        if (this != &other)
        {
            base_formatter_ = std::move(other.base_formatter_);
            pattern_ = std::move(other.pattern_);
        }
        return *this;
    }

    // 禁用拷贝
    ANSICleaningFormatter(const ANSICleaningFormatter &) = delete;
    ANSICleaningFormatter &operator=(const ANSICleaningFormatter &) = delete;

    void format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) override
    {
        std::lock_guard<std::mutex> lock(format_mutex_); // 添加互斥锁

        // 如果 base_formatter_ 为空，重新创建
        if (!base_formatter_)
        {
            base_formatter_ = std::make_unique<spdlog::pattern_formatter>(pattern_);
        }

        spdlog::memory_buf_t formatted;
        base_formatter_->format(msg, formatted);
        std::string clean_str = ansi::remove_ansi_escape(
            std::string(formatted.data(), formatted.size()));
        dest.append(clean_str.data(), clean_str.data() + clean_str.size());
    }

    std::unique_ptr<spdlog::formatter> clone() const override
    {
        // 创建新实例时使用保存的模式字符串
        return std::make_unique<ANSICleaningFormatter>(pattern_);
    }

private:
    mutable std::mutex format_mutex_;                           // 互斥锁保护格式化操作
    mutable std::unique_ptr<spdlog::formatter> base_formatter_; // 可变的智能指针
    std::string pattern_;                                       // 保存模式字符串
};

namespace fs = std::filesystem;

std::string log_dir = "logs";
std::string log_file;

// 添加全局互斥锁和运行标志
std::mutex log_rotate_mutex;
std::atomic<bool> log_running(true);

// 全局日志目录
std::string global_log_dir = "logs";

void ensure_log_directory(const std::string &path)
{
    if (!fs::exists(path))
    {
        if (!fs::create_directories(path))
        {
            throw std::runtime_error("无法创建日志目录: " + path);
        }
#if defined(__linux__) || defined(__APPLE__)
        fs::permissions(path,
                        fs::perms::owner_all |
                            fs::perms::group_read | fs::perms::group_exec |
                            fs::perms::others_read | fs::perms::others_exec);
#endif
    }

    auto status = fs::status(path);
    if ((status.permissions() & fs::perms::owner_write) == fs::perms::none)
    {
        throw std::runtime_error("没有日志目录写权限: " + path);
    }
}

std::string generate_daily_filename(const std::string &base_dir)
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << base_dir << "/app_" << std::put_time(&tm, "%Y-%m-%d") << ".log";
    return oss.str();
}

std::chrono::system_clock::time_point get_next_midnight()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    tm.tm_mday += 1;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::shared_ptr<spdlog::logger> initialize_logging_system()
{
    // 从环境变量获取日志目录
    std::string log_dir = global_log_dir;
    if (const char *env_log = std::getenv("APP_LOG_DIR"))
    {
        log_dir = env_log;
    }
    ensure_log_directory(log_dir);

    // 初始化异步日志线程池
    spdlog::init_thread_pool(8192, 1);

    // 创建sink
    std::vector<spdlog::sink_ptr> sinks;

    // 控制台sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%l]%$ %v");

    // 设置颜色
    console_sink->set_color(spdlog::level::trace, ansi::bold + std::string(ansi::cyan));
    console_sink->set_color(spdlog::level::debug, ansi::bold + std::string(ansi::blue));
    console_sink->set_color(spdlog::level::info, ansi::bold + std::string(ansi::green));
    console_sink->set_color(spdlog::level::warn, ansi::bold + std::string(ansi::yellow) + ansi::bg_black);
    console_sink->set_color(spdlog::level::err, ansi::bold + std::string(ansi::red) + ansi::bg_white);
    console_sink->set_color(spdlog::level::critical, ansi::bold + std::string(ansi::white) + ansi::bg_red);

    sinks.push_back(console_sink);

    // 文件sink
    log_file = generate_daily_filename(log_dir);
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, true);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_formatter(std::make_unique<ANSICleaningFormatter>());
    sinks.push_back(file_sink);

    // 创建异步日志器
    auto logger = std::make_shared<spdlog::async_logger>(
        "main_logger",
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    logger->set_level(spdlog::level::trace);
    spdlog::set_default_logger(logger);

    // 使用 weak_ptr 避免悬空指针
    std::weak_ptr<spdlog::async_logger> weak_logger = logger;

    // 每日日志文件切换
    std::thread([weak_logger, log_dir]()
                {
        while (log_running) {
            std::this_thread::sleep_until(get_next_midnight());
            if (!log_running) break;
            auto logger_ptr = weak_logger.lock();
            if (!logger_ptr) {
                // 日志器已被销毁，退出线程
                break;
            }
            {
                std::lock_guard<std::mutex> lock(log_rotate_mutex);
                std::string new_file = generate_daily_filename(log_dir);
                SPDLOG_INFO("切换到新日志文件: {}", new_file);
                // 创建新sink
                auto new_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(new_file, true);
                new_sink->set_formatter(std::make_unique<ANSICleaningFormatter>());
                // 安全替换sink
                auto& logger_sinks = logger_ptr->sinks();
                if (logger_sinks.size() > 1) {
                    logger_sinks[1] = new_sink;
                } else {
                    // 如果sinks不足，添加新sink
                    logger_sinks.push_back(new_sink);
                }
            }
        } })
        .detach();

    // 记录初始化信息
    SPDLOG_INFO("===== 应用程序启动 =====");
    SPDLOG_INFO("日志文件: {}", log_file);
    SPDLOG_INFO("日志目录: {}", log_dir);
    SPDLOG_INFO("当前路径: {}", fs::current_path().string());
    SPDLOG_INFO("日志系统初始化成功");

    return logger;
}

// 添加关闭日志系统函数
void shutdown_logging_system()
{
    log_running = false; // 通知日志切换线程停止
    spdlog::shutdown();  // 安全关闭所有日志资源
}