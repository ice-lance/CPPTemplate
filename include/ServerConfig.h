#pragma once

#include <string>
#include <json.hpp>
#include <yaml-cpp/yaml.h>

namespace nl = nlohmann;

// HTTP API 配置结构体
struct HttpApiConfig
{
    std::string ip = "0.0.0.0";
    int port = 8080;

    // 转换为JSON
    nl::json toJson() const;
};

// 主配置结构体
struct ServerConfig
{
    std::string name;
    int version = 0;
    std::string id;
    HttpApiConfig http_api;

    // 转换为JSON
    nl::json toJson() const;
};

// 配置管理器类
class ConfigManager
{
public:
    // 从文件加载配置
    static ServerConfig loadFromFile(const std::string &filepath);

    // 从字符串内容加载配置
    static ServerConfig loadFromString(const std::string &content);

    // 配置转换为JSON
    static nl::json toJson(const ServerConfig &config);

private:
    // 解析YAML节点到配置对象
    static ServerConfig parseConfig(const YAML::Node &config);
};