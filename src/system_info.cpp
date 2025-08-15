#include "system_info.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <unordered_map>
#include <cmath>

// CPU负载计算函数
double get_cpu_load()
{
    // 读取CPU统计数据的lambda函数
    auto read_cpu_stats = []() -> std::vector<unsigned long>
    {
        std::ifstream file("/proc/stat");
        std::string line;
        std::getline(file, line); // 读取第一行(cpu总体统计)
        std::istringstream iss(line);

        std::vector<unsigned long> values;
        std::string cpu_label;
        iss >> cpu_label; // 跳过"cpu"标签

        unsigned long val;
        while (iss >> val)
        {
            values.push_back(val);
        }
        return values;
    };

    // 第一次读取CPU状态
    auto stats1 = read_cpu_stats();
    if (stats1.size() < 4)
        return -1.0; // 确保有足够数据

    // 等待100ms后再次读取
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto stats2 = read_cpu_stats();
    if (stats2.size() < 4)
        return -1.0;

    // 计算两次统计的差值
    unsigned long prev_idle = stats1[3];
    unsigned long idle = stats2[3];

    unsigned long prev_total = 0;
    unsigned long total = 0;

    for (size_t i = 0; i < stats1.size(); ++i)
    {
        prev_total += stats1[i];
        total += stats2[i];
    }

    // 计算CPU利用率
    double idle_diff = static_cast<double>(idle - prev_idle);
    double total_diff = static_cast<double>(total - prev_total);

    if (total_diff == 0)
        return 0.0;

    return 100.0 * (1.0 - idle_diff / total_diff);
}

// 内存信息获取函数
MemoryInfo get_memory_info()
{
    std::ifstream file("/proc/meminfo");
    std::unordered_map<std::string, long> mem_data;
    std::string key;
    long value;
    std::string unit;

    // 解析/proc/meminfo文件
    while (file >> key >> value >> unit)
    {
        key.pop_back(); // 移除键名后的冒号
        mem_data[key] = value;
    }

    // 返回内存信息结构体
    return {
        mem_data["MemTotal"],
        mem_data["MemFree"],
        mem_data["MemAvailable"],
        mem_data["Buffers"],
        mem_data["Cached"]};
}

// 带时区的时间获取函数
std::string get_current_time_with_timezone()
{
    // 获取当前系统时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // 转换为本地时间（线程安全）
    struct tm local_time;
    localtime_r(&now_time, &local_time);

    // 格式化为字符串
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");

    // 计算时区偏移
    long offset = local_time.tm_gmtoff;
    char sign = (offset >= 0) ? '+' : '-';
    int hours = static_cast<int>(std::abs(offset)) / 3600;
    int minutes = (static_cast<int>(std::abs(offset)) % 3600) / 60;

    // 添加时区信息
    oss << " UTC" << sign
        << std::setfill('0') << std::setw(2) << hours
        << ":" << std::setw(2) << minutes;

    return oss.str();
}