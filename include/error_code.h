#ifndef ERROR_CODE_H
#define ERROR_CODE_H

#include <string>
#include <unordered_map>

/**
 * @brief 错误码枚举类，按功能模块分类
 *
 * 错误码范围划分：
 *  0-99   : 成功状态
 *  100-199: 通用错误
 *  200-299: 区块链相关错误
 *  300-399: 钱包相关错误
 *  400-499: 存储相关错误
 *  500-599: 网络相关错误
 *  600-699: 系统级错误
 */
enum class ErrorCode
{
    // ============= 成功状态 (0-99) =============
    SUCCESS = 0, ///< 操作成功执行

    // ============= 通用错误 (100-199) =============
    UNKNOWN_ERROR = 100,           ///< 未知类型的错误
    INVALID_ARGUMENT = 101,        ///< 参数格式错误或缺失必要参数
    RESOURCE_NOT_FOUND = 102,      ///< 请求的资源不存在
    PERMISSION_DENIED = 103,       ///< 权限不足，拒绝访问
    OPERATION_FAILED = 104,        ///< 操作执行过程中发生意外失败
    INVALID_DATA = 105,            ///< 数据格式不符合要求或校验失败
    RESOURCE_ALREADY_EXISTS = 106, ///< 创建资源时发生冲突（资源已存在）


    // ============= 网络相关 (500-599) =============
    NETWORK_ERROR = 500,          ///< 网络通信故障（连接超时/中断）
    API_ENDPOINT_NOT_FOUND = 501, ///< 请求的API端点不存在或已停用

    // ============= 系统错误 (600-699) =============
    SYSTEM_ERROR = 600,        ///< 操作系统级未分类错误
    MEMORY_ALLOC_FAILED = 601, ///< 内存分配失败（资源耗尽）
    FILE_IO_ERROR = 602        ///< 文件读写失败（权限/磁盘损坏）
};

/**
 * @brief 错误信息工具类
 *
 * 提供错误码到可读消息和HTTP状态码的转换功能
 */
class ErrorInfo
{
public:
    /**
     * @brief 获取错误码对应的描述信息
     * @param code 错误码枚举值
     * @return 可读的错误描述（UTF-8字符串）
     */
    static std::string getMessage(ErrorCode code);

    /**
     * @brief 获取错误码对应的HTTP状态码
     * @param code 错误码枚举值
     * @return 符合REST规范的HTTP状态码
     */
    static int getHttpStatus(ErrorCode code);

    /**
     * @brief 检查错误码是否表示成功
     * @param code 待检查的错误码
     * @return true表示成功，false表示失败
     */
    static bool isSuccess(ErrorCode code)
    {
        return code == ErrorCode::SUCCESS;
    }
};

#endif // ERROR_CODE_H