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

void registerBlockchainAPI(httplib::Server &server)
{

    
}