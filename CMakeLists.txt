cmake_minimum_required(VERSION 3.12)
project(@PROJECT_NAME@ VERSION @PROJECT_VERSION@)

set(CMAKE_BUILD_TYPE Debug)

# 指定C++标准
set(CMAKE_CXX_STANDARD 17) # 设置C++标准为C++17
set(CMAKE_CXX_STANDARD_REQUIRED True) # 使C++标准成为必需的，如果编译器不支持，则会报错

# 添加编译选项，这里添加了一些比较常用的警告选项
add_compile_options(-Wall -Wextra -pedantic)

# 如果是多配置类型的生成器（例如 Visual Studio），可以针对不同配置设置不同的编译选项
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

# 设置项目的版本号
# set(VERSION_MAJOR 1)
# set(VERSION_MINOR 0)
# set(VERSION_PATCH 0)

# 配置版本头文件
configure_file(
    "${CMAKE_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_BINARY_DIR}/include/version.h"
)

# 头文件目录
include_directories(
    include
    ${CMAKE_BINARY_DIR}/include  # 生成的版本头文件目录
)

# 源文件
file(GLOB SOURCES "src/*.cpp" "protocols/*.cpp")


# 生成可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 添加版本定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_NAME="${PROJECT_NAME}"
    APP_VERSION="${PROJECT_VERSION}"
)

# 添加库搜索路径（确保路径正确）
link_directories(${CMAKE_SOURCE_DIR}/libs)

# 将库路径添加到特定目标
target_link_libraries(${PROJECT_NAME} PRIVATE ${CANCMD_LIB})
target_link_libraries(${PROJECT_NAME} PUBLIC
pthread ixwebsocket z ssl crypto yaml-cpp uuid
)


# 4. 确保运行时能找到 .so 文件
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../libs")

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION bin)