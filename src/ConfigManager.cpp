#include "ServerConfig.h"
#include <fstream>
#include <stdexcept>

using namespace std;

// HTTP API配置转换为JSON
nl::json HttpApiConfig::toJson() const
{
    return nl::json{
        {"ip", ip},
        {"port", port}};
}

// 主配置转换为JSON
nl::json ServerConfig::toJson() const
{
    return nl::json{
        {"name", name},
        {"version", version},
        {"id", id},
        {"http_api", http_api.toJson()}};
}

// 从文件加载配置
ServerConfig ConfigManager::loadFromFile(const string &filepath)
{
    try
    {
        YAML::Node config = YAML::LoadFile(filepath);
        return parseConfig(config);
    }
    catch (const YAML::Exception &e)
    {
        throw runtime_error("YAML解析错误: " + string(e.what()));
    }
    catch (const ifstream::failure &e)
    {
        throw runtime_error("文件错误: " + string(e.what()));
    }
}

// 从字符串加载配置
ServerConfig ConfigManager::loadFromString(const string &content)
{
    try
    {
        YAML::Node config = YAML::Load(content);
        return parseConfig(config);
    }
    catch (const YAML::Exception &e)
    {
        throw runtime_error("YAML解析错误: " + string(e.what()));
    }
}

// 配置转换为JSON
nl::json ConfigManager::toJson(const ServerConfig &config)
{
    return config.toJson();
}

// 解析YAML配置
ServerConfig ConfigManager::parseConfig(const YAML::Node &config)
{
    ServerConfig cfg;

    // 解析顶层字段
    if (config["name"])
        cfg.name = config["name"].as<string>();
    if (config["version"])
        cfg.version = config["version"].as<int>();
    if (config["id"])
        cfg.id = config["id"].as<string>();

    // 解析HTTP API配置
    if (config["http_api"])
    {
        const YAML::Node &http = config["http_api"];
        if (http["ip"])
            cfg.http_api.ip = http["ip"].as<string>();
        if (http["port"])
            cfg.http_api.port = http["port"].as<int>();
    }

    return cfg;
}