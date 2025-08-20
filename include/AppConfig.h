#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <string>

// HTTP API 服务器配置结构体
struct HttpApiConfig {
    std::string ip;
    int port;
};

// 应用程序配置结构体
struct AppConfig {
    std::string name;
    int version;
    std::string id;
    HttpApiConfig apiHttp;
};

#endif // APP_CONFIG_H