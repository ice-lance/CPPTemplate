#include "ConfigManager.h"
#include <fstream>
#include <iostream>

bool ConfigManager::loadFromFile(const std::string &filename)
{
    try
    {
        m_rootNode = YAML::LoadFile(filename);
        return true;
    }
    catch (const YAML::Exception &e)
    {
        std::cerr << "Error loading YAML file: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigManager::saveToFile(const std::string &filename) const
{
    try
    {
        std::ofstream fout(filename);
        fout << m_rootNode;
        fout.close();
        return true;
    }
    catch (const YAML::Exception &e)
    {
        std::cerr << "Error saving YAML file: " << e.what() << std::endl;
        return false;
    }
}

AppConfig ConfigManager::getAsAppConfig() const
{
    AppConfig config;

    try
    {
        if (m_rootNode["name"])
            config.name = m_rootNode["name"].as<std::string>();
        if (m_rootNode["version"])
            config.version = m_rootNode["version"].as<int>();
        if (m_rootNode["id"])
            config.id = m_rootNode["id"].as<std::string>();

        // 注意：这里从 http_api 改为 apiHttp
        if (m_rootNode["http_api"])
        {
            if (m_rootNode["http_api"]["ip"])
                config.apiHttp.ip = m_rootNode["http_api"]["ip"].as<std::string>();
            if (m_rootNode["http_api"]["port"])
                config.apiHttp.port = m_rootNode["http_api"]["port"].as<int>();
        }
    }
    catch (const YAML::Exception &e)
    {
        std::cerr << "Error converting YAML node: " << e.what() << std::endl;
    }

    return config;
}

void ConfigManager::setFromAppConfig(const AppConfig &config)
{
    try
    {
        m_rootNode["name"] = config.name;
        m_rootNode["version"] = config.version;
        m_rootNode["id"] = config.id;

        // 注意：这里从 http_api 改为 apiHttp
        m_rootNode["http_api"]["ip"] = config.apiHttp.ip;
        m_rootNode["http_api"]["port"] = config.apiHttp.port;
    }
    catch (const YAML::Exception &e)
    {
        std::cerr << "Error creating YAML node: " << e.what() << std::endl;
    }
}

// 静态方法实现 - 重命名为 loadConfigFromFile
AppConfig ConfigManager::loadConfigFromFile(const std::string &filename)
{
    ConfigManager manager;
    if (manager.loadFromFile(filename))
    {
        return manager.getAsAppConfig();
    }
    // 返回默认配置
    AppConfig defaultConfig;
    defaultConfig.name = "cppprojectname";
    defaultConfig.version = 2;
    defaultConfig.id = "d24391b2dc714fe6a11e97712390483d";
    defaultConfig.apiHttp.ip = "0.0.0.0";
    defaultConfig.apiHttp.port = 8080;
    return defaultConfig;
}