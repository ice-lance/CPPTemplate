#include "error_code.h"

/**
 * @brief 错误码到中文描述的全局映射表
 *
 * 此映射表维护所有预定义错误码的中文描述信息，用于日志输出和用户提示。
 * 当新增错误码时，必须在此表中添加对应的描述，否则会返回"未知错误码"。
 */
static const std::unordered_map<ErrorCode, std::string> errorMessages = {
    // ============= 成功状态 =============
    {ErrorCode::SUCCESS, "操作成功"},

    // ============= 通用错误 =============
    {ErrorCode::UNKNOWN_ERROR, "发生未知错误"},
    {ErrorCode::INVALID_ARGUMENT, "无效参数"},
    {ErrorCode::RESOURCE_NOT_FOUND, "资源未找到"},
    {ErrorCode::PERMISSION_DENIED, "权限不足"},
    {ErrorCode::OPERATION_FAILED, "操作执行失败"},
    {ErrorCode::INVALID_DATA, "数据格式无效"},
    {ErrorCode::RESOURCE_ALREADY_EXISTS, "资源已存在"},

    

    // ============= 网络相关 =============
    {ErrorCode::NETWORK_ERROR, "网络通信错误"},
    {ErrorCode::API_ENDPOINT_NOT_FOUND, "API端点未找到"},

    // ============= 系统错误 =============
    {ErrorCode::SYSTEM_ERROR, "系统错误"},
    {ErrorCode::MEMORY_ALLOC_FAILED, "内存分配失败"},
    {ErrorCode::FILE_IO_ERROR, "文件I/O错误"}};

/**
 * @brief 错误码到HTTP状态码的全局映射表
 *
 * 此映射表遵循RESTful API设计规范，将业务错误映射到标准HTTP状态码。
 * 当新增错误码时，必须在此表中添加对应的HTTP状态码，否则默认返回500。
 *
 * HTTP状态码使用标准：
 *  2xx - 成功
 *  4xx - 客户端错误
 *  5xx - 服务器错误
 */
static const std::unordered_map<ErrorCode, int> httpStatusCodes = {
    {ErrorCode::SUCCESS, 200}, // 请求成功

    // ============= 通用错误 =============
    {ErrorCode::UNKNOWN_ERROR, 500},           // 服务器内部错误
    {ErrorCode::INVALID_ARGUMENT, 400},        // 客户端请求错误
    {ErrorCode::RESOURCE_NOT_FOUND, 404},      // 资源不存在
    {ErrorCode::PERMISSION_DENIED, 403},       // 禁止访问
    {ErrorCode::OPERATION_FAILED, 500},        // 服务器内部错误
    {ErrorCode::INVALID_DATA, 400},            // 客户端请求错误
    {ErrorCode::RESOURCE_ALREADY_EXISTS, 409}, // 资源冲突

    
    // ============= 网络相关 =============
    {ErrorCode::NETWORK_ERROR, 500},          // 服务器内部错误
    {ErrorCode::API_ENDPOINT_NOT_FOUND, 404}, // 资源不存在

    // ============= 系统错误 =============
    {ErrorCode::SYSTEM_ERROR, 500},        // 服务器内部错误
    {ErrorCode::MEMORY_ALLOC_FAILED, 500}, // 服务器内部错误
    {ErrorCode::FILE_IO_ERROR, 500}        // 服务器内部错误
};

/**
 * @brief 获取错误码对应的中文描述
 * @param code 错误码枚举值
 * @return 对应错误的中文描述字符串
 *
 * 如果错误码未在映射表中注册，则返回"未知错误码"
 */
std::string ErrorInfo::getMessage(ErrorCode code)
{
    auto it = errorMessages.find(code);
    return it != errorMessages.end() ? it->second : "未知错误码";
}

/**
 * @brief 获取错误码对应的HTTP状态码
 * @param code 错误码枚举值
 * @return 对应的HTTP状态码
 *
 * 如果错误码未在映射表中注册，则默认返回500（服务器内部错误）
 */
int ErrorInfo::getHttpStatus(ErrorCode code)
{
    auto it = httpStatusCodes.find(code);
    return it != httpStatusCodes.end() ? it->second : 500;
}