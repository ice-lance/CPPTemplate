# C++项目模板使用指南

## 项目概述

这是一个现代化的C++项目模板，提供一键初始化、自动化构建、打包和部署功能，特别适合需要部署为系统服务或Docker容器的应用程序。

## 快速开始

### 1. 获取模板

```bash
git clone https://github.com/ice-lance/CPPTemplate.git
cd cpp_project_template
```

### 2. 初始化项目

```bash
./init_project.sh
```

脚本会提示：

- **项目名称**：不能为空（如 `MyAccountingApp`）
- **初始版本号**：默认为 `1.0.0`

### 3. 开始开发

- 将您的源代码放入 `src/` 目录
- 头文件放入 `include/` 目录
- 更新 `version.h.in` 中的元数据（如果需要）

### 4. 构建和打包

```bash
./build-and-package.sh [版本号]
```

- 若不指定版本号，则使用CMakeLists.txt中的版本
- 若指定版本号（如 `1.2.3`），则覆盖默认版本

## 项目结构

```bash
my_project/
├── CMakeLists.txt                # CMake配置文件
├── build-and-package.sh          # 构建打包脚本
├── .gitignore                    # Git忽略规则
├── src/                          # 源代码目录
│   └── main.cpp                  # 主程序文件
├── include/                      # 头文件目录
│   └── version.h.in              # 版本头文件模板
└── build/                        # 构建目录（自动创建）
```

## 核心功能

### 1. 初始化脚本 (`init_project.sh`)

- 设置项目名称和版本
- 自动更新CMake配置
- 创建新的Git仓库
- 可选择是否删除自身

### 2. 构建脚本 (`build-and-package.sh`)

自动完成以下流程：

1. **编译项目**：在 `build/`目录执行Release构建
2. **收集依赖**：自动提取动态库依赖
3. **生成服务文件**：创建systemd服务单元
4. **创建安装脚本**：`install.sh`（需sudo运行）
5. **创建卸载脚本**：`uninstall.sh`
6. **打包发布文件**：包含完整部署所需文件
7. **构建Docker镜像**：基于Alpine的轻量级镜像

### 3. 生成产物

在 `release/`目录生成：

1. `<项目名>-<版本号>-linux64.tar.gz` - Linux可执行程序包
2. `<项目名>-<版本号>-docker.tar` - Docker镜像

## 高级配置

### 版本管理

在CMakeLists.txt中修改版本：

```cmake
project(MyAccountingApp VERSION 2.0.0)  # 修改此处版本号
```

### 服务配置

在 `build-and-package.sh`中可修改：

```bash
# ========== 配置区域 ==========
APP_NAME="MyAccountingApp"       # 应用程序名称
SERVICE_USER="appuser"           # 运行服务的用户
APP_INSTALL_DIR="/opt/accounting"# 应用安装目录
# =============================
```

### Docker定制

在生成的Dockerfile中修改：

```dockerfile
FROM alpine:3.14                 # 基础镜像
EXPOSE 8080                      # 暴露端口
ENV CONFIG_PATH="/config"        # 环境变量
```

## 最佳实践

### 开发工作流

```bash
# 1. 初始化项目
./init_project.sh

# 2. 开发迭代
git add src/new_feature.cpp
git commit -m "添加新功能"

# 3. 构建测试
./build-and-package.sh

# 4. 部署验证
tar xzf release/*.tar.gz
cd MyAccountingApp-1.0.0
sudo ./install.sh
sudo systemctl status MyAccountingApp

# 5. 发布版本
./build-and-package.sh 1.1.0
```

### 版本管理建议

1. 使用语义化版本 (Semantic Versioning)
2. 每次功能更新后递增次版本号
3. 重大变更时递增主版本号
4. 修复BUG时递增修订号

## 注意事项

1. **权限要求**：

   - 安装/卸载脚本需root权限
   - Docker构建需安装Docker引擎
2. **系统要求**：

   - Linux环境（已在Ubuntu/CentOS测试）
   - CMake 3.12+
   - Git 2.0+
   - Docker（可选）
3. **安全建议**：

   - 生产环境避免使用root运行服务
   - Docker容器使用非root用户
   - 定期更新依赖库

## 技术支持

遇到问题请检查：

1. `build/`目录下的CMake日志
2. 系统日志：`journalctl -xe`
3. Docker日志：`docker logs <容器名>`

如需进一步帮助，请提交issue至项目仓库。
