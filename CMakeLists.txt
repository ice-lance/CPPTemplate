
# 指定CMake的最小版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称及版本
project(CPPTemplate VERSION 1.0 LANGUAGES CXX)

# 指定C++标准
set(CMAKE_CXX_STANDARD 17) # 设置C++标准为C++17
set(CMAKE_CXX_STANDARD_REQUIRED True) # 使C++标准成为必需的，如果编译器不支持，则会报错

# 添加编译选项，这里添加了一些比较常用的警告选项
add_compile_options(-Wall -Wextra -pedantic)

# 如果是多配置类型的生成器（例如 Visual Studio），可以针对不同配置设置不同的编译选项
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")

# 设置项目的版本号
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)


# 配置一个头文件，通过它来传递一些CMake设置到源代码中
# configure_file(
#   "${PROJECT_SOURCE_DIR}/Config.h.in"
#   "${PROJECT_BINARY_DIR}/Config.h"
# )



# 让CMake能够在指定的目录下查找头文件
# 这里我们把构建目录添加到包含目录中，这样我们的Config.h文件就能被包含进来
# include_directories("${PROJECT_BINARY_DIR}")

# 添加子目录，这通常用于包含多个项目或库
# add_subdirectory(subdir)

# 可以指定头文件搜索路径
include_directories(
    include    
    
)

# 添加生成目标，创建一个可执行文件
# 根据指定的源文件生成项目的可执行文件
add_executable(${PROJECT_NAME} main.cpp 

)



# 将库路径添加到特定目标
target_link_libraries(${PROJECT_NAME} 
pthread
)

# 添加链接库，如果项目中有用到外部库，需要链接到我们的项目中
# target_link_libraries(${PROJECT_NAME} name_of_library)

# 安装规则，指定安装路径和目标文件
# install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 启用测试
# enable_testing()

# 定义测试案例
# add_test(NAME Runs COMMAND ${PROJECT_NAME})