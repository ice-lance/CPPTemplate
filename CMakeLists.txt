cmake_minimum_required(VERSION 3.12)
project(@PROJECT_NAME@ VERSION @PROJECT_VERSION@)

# 配置版本头文件
configure_file(
    "${CMAKE_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_BINARY_DIR}/include/version.h"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 头文件目录
include_directories(
    include
    ${CMAKE_BINARY_DIR}/include  # 生成的版本头文件目录
)

# 源文件
file(GLOB SOURCES "src/*.cpp")

# 生成可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 添加版本定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_NAME="${PROJECT_NAME}"
    APP_VERSION="${PROJECT_VERSION}"
)

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION bin)