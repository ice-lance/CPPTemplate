#pragma once
#include <spdlog/spdlog.h>
#include <memory>
#include <mutex>  // 添加互斥锁支持

std::shared_ptr<spdlog::logger> initialize_logging_system();
void shutdown_logging_system();  // 添加关闭日志系统函数