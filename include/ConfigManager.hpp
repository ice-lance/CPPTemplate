#ifndef CONFIG_MANAGER_TPP
#define CONFIG_MANAGER_TPP

#include "ConfigManager.h"
#include <iostream>

template <typename T>
T ConfigManager::getAs() const
{
    try
    {
        return m_rootNode.as<T>();
    }
    catch (const YAML::Exception &e)
    {
        std::cerr << "Error converting YAML node: " << e.what() << std::endl;
        return T();
    }
}

template <typename T>
void ConfigManager::setFrom(const T &config)
{
    try
    {
        m_rootNode = YAML::Node(config);
    }
    catch (const YAML::Exception &e)
    {
        std::cerr << "Error creating YAML node: " << e.what() << std::endl;
    }
}

#endif // CONFIG_MANAGER_TPP