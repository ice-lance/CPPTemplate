// api_handler.cpp
#include "api_handler.h"
#include "json.hpp"
#include "error_code.h" // 包含错误码头文件
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <spdlog/spdlog.h>
#include "utils.hpp"
using json = nlohmann::json;

// 创建标准错误响应
json createErrorResponse(ErrorCode code, const std::string &additionalInfo = "")
{
    json response;
    response["code"] = static_cast<int>(code);
    response["message"] = ErrorInfo::getMessage(code);

    if (!additionalInfo.empty())
    {
        response["details"] = additionalInfo;
    }

    return response;
}

// 创建成功响应
json createSuccessResponse(const json &data = json::object())
{
    json response;
    response["code"] = static_cast<int>(ErrorCode::SUCCESS);
    response["message"] = ErrorInfo::getMessage(ErrorCode::SUCCESS);

    if (!data.empty())
    {
        response["data"] = data;
    }

    return response;
}
// 获取当前时间的ISO 8601格式字符串
std::string getCurrentTimeISO8601()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    gmtime_r(&in_time_t, &tm); // 线程安全的GMT时间转换

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void registerBlockchainAPI(httplib::Server &server)
{
    // 注册 /info GET 接口
    server.Get("/info", [](const httplib::Request &req, httplib::Response &res)
               {
        SPDLOG_INFO("Received info request from {}", req.remote_addr);
        
        try {
            // 构造状态信息
            json info;
            info["version"] = "1.2.0";  // 服务器版本
            info["status"] = "active";  // 服务器状态
            info["timestamp"] = getCurrentTimeISO8601();  // ISO 8601格式时间戳
            
            // 系统信息 (示例值)
            info["system"]["connections"] = 42;  // 当前连接数
            info["system"]["uptime"] = "36 hours"; // 运行时间
            
            // 返回成功响应
            res.set_content(createSuccessResponse(info).dump(), "application/json");
            SPDLOG_DEBUG("Info response sent successfully");
        } 
        catch (const std::exception& e) {
            SPDLOG_ERROR("Error generating info: {}", e.what());
            json error = createErrorResponse(ErrorCode::INTERNAL_ERROR, "服务器信息不可用");
            res.set_content(error.dump(), "application/json");
            res.status = 500;
        } });
}