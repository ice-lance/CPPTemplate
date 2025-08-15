#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <string>

// 内存信息结构体
struct MemoryInfo
{
    long total;     // 总内存 (KB)
    long free;      // 空闲内存 (KB)
    long available; // 可用内存 (KB)
    long buffers;   // 缓冲区内存 (KB)
    long cached;    // 缓存内存 (KB)
};

// 获取CPU负载百分比
double get_cpu_load();

// 获取内存信息
MemoryInfo get_memory_info();

// 获取带时区的当前时间字符串 (格式: YYYY-MM-DD HH:MM:SS UTC±HH:MM)
std::string get_current_time_with_timezone();

#endif // SYSTEM_INFO_H