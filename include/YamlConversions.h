#ifndef YAML_CONVERSIONS_H
#define YAML_CONVERSIONS_H

#include <yaml-cpp/yaml.h>
#include "AppConfig.h"

namespace YAML {
// HttpApiConfig 的 YAML 转换
template<>
struct convert<HttpApiConfig> {
    static Node encode(const HttpApiConfig& rhs) {
        Node node;
        node["ip"] = rhs.ip;
        node["port"] = rhs.port;
        return node;
    }

    static bool decode(const Node& node, HttpApiConfig& rhs) {
        if (!node.IsMap()) {
            return false;
        }
        rhs.ip = node["ip"].as<std::string>();
        rhs.port = node["port"].as<int>();
        return true;
    }
};

// AppConfig 的 YAML 转换
template<>
struct convert<AppConfig> {
    static Node encode(const AppConfig& rhs) {
        Node node;
        node["name"] = rhs.name;
        node["version"] = rhs.version;
        node["id"] = rhs.id;
        node["http_api"] = rhs.apiHttp; // 注意：字段名仍然是 http_api，但映射到 apiHttp
        return node;
    }

    static bool decode(const Node& node, AppConfig& rhs) {
        if (!node.IsMap()) {
            return false;
        }
        rhs.name = node["name"].as<std::string>();
        rhs.version = node["version"].as<int>();
        rhs.id = node["id"].as<std::string>();
        rhs.apiHttp = node["http_api"].as<HttpApiConfig>(); // 注意：字段名仍然是 http_api
        return true;
    }
};
}

#endif // YAML_CONVERSIONS_H