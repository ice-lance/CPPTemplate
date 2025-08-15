#include "system_info_display.h"
#include "system_info.h"
#include "color_utils.h"
#include <iostream>

void display_system_info()
{

    // 显示CPU负载
    double cpu_load = get_cpu_load();
    std::cout << colorize_text("CPU Load: ", ansi::cyan)
              << colorize_text(std::to_string(cpu_load) + "%",
                               cpu_load > 80 ? ansi::red : cpu_load > 50 ? ansi::yellow
                                                                         : ansi::green)
              << "\n";

    // 显示内存信息
    MemoryInfo mem = get_memory_info();
    std::cout << colorize_text("Memory Information:\n", ansi::cyan);
    std::cout << colorize_text("  Total:     ", ansi::blue)
              << mem.total / 1024.0 << " MB\n";
    std::cout << colorize_text("  Free:      ", ansi::green)
              << mem.free / 1024.0 << " MB\n";
    std::cout << colorize_text("  Available: ", ansi::yellow)
              << mem.available / 1024.0 << " MB\n";

    // 显示当前时间
    std::string current_time = get_current_time_with_timezone();
    std::cout << colorize_text("Current Time: ", ansi::magenta)
              << colorize_text(current_time, ansi::white, ansi::bg_blue) << std::endl;
    std::cout << std::endl;
}