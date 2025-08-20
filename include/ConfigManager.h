#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <yaml-cpp/yaml.h>
#include "AppConfig.h"

class ConfigManager
{
public:
    ConfigManager() = default;
    ~ConfigManager() = default;

    // 从文件加载YAML配置
    bool loadFromFile(const std::string &filename);

    // 保存YAML配置到文件
    bool saveToFile(const std::string &filename) const;

    // 从YAML节点转换为自定义结构体
    AppConfig getAsAppConfig() const;

    // 从自定义结构体设置YAML节点
    void setFromAppConfig(const AppConfig &config);

    // 模板方法：从YAML节点转换为任意类型
    template <typename T>
    T getAs() const;

    // 模板方法：从任意类型设置YAML节点
    template <typename T>
    void setFrom(const T &config);

    // 静态方法：直接从文件加载配置
    static AppConfig loadConfigFromFile(const std::string &filename);

    // 获取原始YAML节点（用于高级操作）
    YAML::Node getRootNode() const { return m_rootNode; }

    // 设置原始YAML节点（用于高级操作）
    void setRootNode(const YAML::Node &node) { m_rootNode = node; }

private:
    YAML::Node m_rootNode;
};

// 包含模板实现
#include "ConfigManager.hpp"

#endif // CONFIG_MANAGER_H