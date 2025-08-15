#!/bin/bash
set -e  # 遇到错误立即退出

# ========== 自动配置区域 ==========
# 从CMakeLists.txt提取项目名称
APP_NAME=$(grep 'project(' CMakeLists.txt | sed -E 's/project\s*\(\s*([^ \n]+).*/\1/i' | tr -d ' \n')
if [ -z "$APP_NAME" ]; then
    APP_NAME="DefaultApp"
fi

# 从CMakeLists.txt提取版本
DEFAULT_VERSION=$(grep 'project(' CMakeLists.txt | grep -o 'VERSION [^ )]*' | awk '{print $2}')
if [ -z "$DEFAULT_VERSION" ]; then
    DEFAULT_VERSION="1.0.0"
fi

BUILD_DIR="build"                # 编译目录
RELEASE_DIR="release"            # 发布目录
INSTALL_PREFIX="/usr/local"      # 安装路径前缀
APP_INSTALL_DIR="/opt/$APP_NAME" # 应用安装目录
SERVICE_USER="root"              # 运行服务的用户
DOCKER_REGISTRY=""               # Docker注册中心地址（可选）
# ================================

# 处理命令行参数
if [ $# -ge 1 ]; then
    VERSION="$1"
    echo "使用命令行参数指定版本: ${VERSION}"
else
    VERSION="${DEFAULT_VERSION}"
    echo "使用CMakeLists.txt中的版本: ${VERSION}"
fi

# 确保目录存在
mkdir -p ${BUILD_DIR} ${RELEASE_DIR}

echo "===== 编译项目 ====="
cd ${BUILD_DIR}
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
make -j$(nproc)

echo "===== 收集可执行文件 ====="
# 自动检测生成的可执行文件
EXECUTABLE=$(find . -maxdepth 1 -type f -executable ! -name "*.so" | head -n1)
if [ -z "$EXECUTABLE" ]; then
  echo "错误：未找到可执行文件！"
  exit 1
fi

echo "找到可执行文件: ${EXECUTABLE}"
cp ${EXECUTABLE} ../${RELEASE_DIR}/${APP_NAME}

# 在构建目录中获取版本信息
echo "===== 获取版本信息 ====="
VERSION_INFO=""
# 尝试获取版本信息
if ./${EXECUTABLE} --version > /dev/null 2>&1; then
    VERSION_INFO=$(./${EXECUTABLE} --version 2>&1)
    echo "获取到版本信息: ${VERSION_INFO}"
else
    echo "警告：无法获取可执行文件的版本信息"
    VERSION_INFO="版本信息不可用"
fi


echo "===== 收集动态库依赖 ====="
cd ../${RELEASE_DIR}
mkdir -p libs

ldd ${APP_NAME} | awk '/=> \//{print $3}' | \
grep -Ev "/(libc\.|libm\.|libgcc_s\.|libstdc\+\+\.|ld-linux)" | \
while read lib; do
  cp -v --parents "${lib}" libs/ 2>/dev/null || true
done

# 修复：为每个库创建SONAME符号链接
echo "===== 创建库文件的SONAME符号链接 ====="
cd libs
find . -type f -exec sh -c '
  for file; do
    soname=$(objdump -p "$file" 2>/dev/null | awk "/SONAME/ {print \$2}")
    if [ -n "$soname" ]; then
      dir=$(dirname "$file")
      basefile=$(basename "$file")
      (cd "$dir" && ln -sf "$basefile" "$soname" 2>/dev/null || true)
    fi
  done
' sh {} +
cd ..

echo "===== 创建启动脚本 ====="
cat > run.sh << EOF
#!/bin/bash
SCRIPT_DIR="\$(cd "\$(dirname "\${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="\${SCRIPT_DIR}/libs:\${LD_LIBRARY_PATH}"
exec "\${SCRIPT_DIR}/${APP_NAME}" "\$@"
EOF

chmod +x run.sh

echo "===== 创建systemd服务文件 ====="
cat > ${APP_NAME}.service << EOF
[Unit]
Description=${APP_NAME} Service
After=network.target

[Service]
Type=simple
User=${SERVICE_USER}
WorkingDirectory=${APP_INSTALL_DIR}
ExecStart=${APP_INSTALL_DIR}/run.sh
Restart=on-failure
RestartSec=5s

[Install]
WantedBy=multi-user.target
EOF

echo "===== 创建安装脚本 ====="
cat > install.sh << EOF
#!/bin/bash
set -e

# 检查root权限
if [ "\$(id -u)" -ne 0 ]; then
  echo "请使用sudo运行此安装脚本"
  exit 1
fi

echo "===== [1/6] 创建安装目录: ${APP_INSTALL_DIR} ====="
mkdir -p "${APP_INSTALL_DIR}"

echo "===== [2/6] 复制文件到安装目录 ====="
cp -r ./* "${APP_INSTALL_DIR}/"
echo "✓ 文件复制完成"

echo "===== [3/6] 设置安装目录权限 ====="
chown -R ${SERVICE_USER}:${SERVICE_USER} "${APP_INSTALL_DIR}"
chmod 755 "${APP_INSTALL_DIR}"
chmod 755 "${APP_INSTALL_DIR}/run.sh"
echo "✓ 权限设置完成"

echo "===== [4/6] 安装systemd服务 ====="
cp "${APP_INSTALL_DIR}/${APP_NAME}.service" "/etc/systemd/system/"
echo "✓ 服务文件已复制到/etc/systemd/system/"

echo "===== [5/6] 重新加载systemd配置 ====="
systemctl daemon-reload
echo "✓ 系统守护进程配置已重新加载"

echo "===== [6/6] 启用并启动服务 ====="
systemctl enable "${APP_NAME}.service"
systemctl start "${APP_NAME}.service"
echo "✓ 服务已启用并启动"

echo -e "\n✅ 安装成功！"
echo "========================================"
echo "服务名称: ${APP_NAME}"
echo "安装目录: ${APP_INSTALL_DIR}"
echo "运行用户: ${SERVICE_USER}"
echo "启动命令: sudo systemctl start ${APP_NAME}"
echo "状态检查: sudo systemctl status ${APP_NAME}"
echo "日志查看: journalctl -u ${APP_NAME}.service"
echo "卸载命令: sudo ${APP_INSTALL_DIR}/uninstall.sh"
echo "========================================"
EOF

chmod +x install.sh

echo "===== 创建卸载脚本 ====="
cat > uninstall.sh << EOF
#!/bin/bash
set -e

# 检查root权限
if [ "\$(id -u)" -ne 0 ]; then
  echo "请使用sudo运行此卸载脚本"
  exit 1
fi

echo "===== [1/5] 停止服务 ====="
systemctl stop "${APP_NAME}.service" || true
echo "✓ 服务已停止"

echo "===== [2/5] 禁用服务 ====="
systemctl disable "${APP_NAME}.service" || true
echo "✓ 服务已禁用"

echo "===== [3/5] 删除服务文件 ====="
rm -f "/etc/systemd/system/${APP_NAME}.service"
echo "✓ 服务文件已删除"

echo "===== [4/5] 重新加载systemd配置 ====="
systemctl daemon-reload
echo "✓ 系统守护进程配置已重新加载"

echo "===== [5/5] 删除安装目录 ====="
if [ -d "${APP_INSTALL_DIR}" ]; then
    rm -rf "${APP_INSTALL_DIR}"
    echo "✓ 安装目录已删除: ${APP_INSTALL_DIR}"
else
    echo "ℹ️ 安装目录 ${APP_INSTALL_DIR} 不存在，跳过删除"
fi

echo -e "\n✅ 卸载完成！"
echo "========================================"
echo "应用程序 ${APP_NAME} 已完全移除"
echo "========================================"
EOF

chmod +x uninstall.sh

echo "===== 创建版本信息文件 ====="
echo "Application: ${APP_NAME}" > version.txt
echo "Version: ${VERSION}" >> version.txt
echo "Build date: $(date '+%Y-%m-%d %H:%M:%S')" >> version.txt
echo "System: $(uname -a)" >> version.txt
echo "Executable Version: ${VERSION_INFO}" >> version.txt

echo "===== 创建使用说明文档 ====="
mkdir -p docs

# 创建文本格式使用说明
cat > docs/INSTRUCTIONS.txt << EOF
${APP_NAME} ${VERSION} 使用说明

一、安装步骤：
1. 解压发布包：
   tar xzf ${APP_NAME}-${VERSION}-linux64.tar.gz
   cd ${APP_NAME}-${VERSION}-linux64

2. 运行安装脚本（需要root权限）：
   sudo ./install.sh

3. 服务管理：
   - 启动服务: sudo systemctl start ${APP_NAME}
   - 停止服务: sudo systemctl stop ${APP_NAME}
   - 查看状态: sudo systemctl status ${APP_NAME}
   - 启用自启: sudo systemctl enable ${APP_NAME}
   - 禁用自启: sudo systemctl disable ${APP_NAME}

4. 查看日志：
   journalctl -u ${APP_NAME}.service

二、卸载步骤：
1. 运行卸载脚本（需要root权限）：
   sudo ${APP_INSTALL_DIR}/uninstall.sh

三、Docker镜像使用：
1. 加载镜像：
   docker load -i ${APP_NAME}-${VERSION}-docker.tar

2. 运行容器：
   docker run -d --name ${APP_NAME} \\
     -v /path/to/config:/config \\
     ${DOCKER_REGISTRY}${APP_NAME}:${VERSION}

四、注意事项：
1. 安装和卸载都需要root权限
2. 默认安装目录: ${APP_INSTALL_DIR}
3. 默认运行用户: ${SERVICE_USER}
EOF

# 创建Markdown格式使用说明
cat > docs/README.md << EOF
# ${APP_NAME} ${VERSION} 使用指南

## 安装说明

\`\`\`bash
# 解压发布包
tar xzf ${APP_NAME}-${VERSION}-linux64.tar.gz
cd ${APP_NAME}-${VERSION}-linux64

# 运行安装脚本（需要root权限）
sudo ./install.sh
\`\`\`

## 服务管理

| 命令 | 说明 |
|------|------|
| \`sudo systemctl start ${APP_NAME}\` | 启动服务 |
| \`sudo systemctl stop ${APP_NAME}\` | 停止服务 |
| \`sudo systemctl restart ${APP_NAME}\` | 重启服务 |
| \`sudo systemctl status ${APP_NAME}\` | 查看服务状态 |
| \`sudo systemctl enable ${APP_NAME}\` | 启用开机自启 |
| \`sudo systemctl disable ${APP_NAME}\` | 禁用开机自启 |
| \`journalctl -u ${APP_NAME}.service\` | 查看服务日志 |

## Docker使用

\`\`\`bash
# 加载Docker镜像
docker load -i ${APP_NAME}-${VERSION}-docker.tar

# 运行容器
docker run -d --name ${APP_NAME} \\
  -v /path/to/config:/config \\
  ${DOCKER_REGISTRY}${APP_NAME}:${VERSION}
\`\`\`

## 卸载程序

\`\`\`bash
sudo ${APP_INSTALL_DIR}/uninstall.sh
\`\`\`

## 配置信息
- 应用版本: ${VERSION}
- 安装路径: ${APP_INSTALL_DIR}
- 服务用户: ${SERVICE_USER}
- Docker镜像: ${DOCKER_REGISTRY}${APP_NAME}:${VERSION}
- 构建日期: $(date '+%Y-%m-%d')
- 系统要求: Linux (64位)
EOF

echo "===== 打包发布文件 ====="
tar czf ${APP_NAME}-${VERSION}-linux64.tar.gz \
  ${APP_NAME} \
  run.sh \
  version.txt \
  libs/ \
  ${APP_NAME}.service \
  install.sh \
  uninstall.sh \
  docs/

echo "===== 构建Docker镜像 ====="
# 创建Dockerfile
cat > Dockerfile << EOF
# 使用轻量级基础镜像
FROM alpine:3.14

# 设置元数据
LABEL maintainer="your-email@example.com"
LABEL version="${VERSION}"
LABEL description="${APP_NAME} Docker Image"

# 创建应用目录
WORKDIR /app

# 复制应用程序和依赖库
COPY ${APP_NAME} /app/
COPY libs /app/libs/
COPY run.sh /app/

# 设置环境变量
ENV APP_NAME=${APP_NAME}
ENV APP_VERSION=${VERSION}

# 设置动态库路径
ENV LD_LIBRARY_PATH=/app/libs:\$LD_LIBRARY_PATH

# 设置执行权限
RUN chmod +x /app/${APP_NAME} /app/run.sh

# 设置入口点
ENTRYPOINT ["./run.sh"]

# 暴露必要端口（如果有）
# EXPOSE 8080
EOF

# 构建Docker镜像
docker build -t ${DOCKER_REGISTRY}${APP_NAME}:${VERSION} .

# 保存Docker镜像
docker save -o ${APP_NAME}-${VERSION}-docker.tar ${DOCKER_REGISTRY}${APP_NAME}:${VERSION}

echo "===== 移动Docker镜像到发布目录 ====="
mv ${APP_NAME}-${VERSION}-docker.tar ${RELEASE_DIR}/

# 关键修复：返回上级目录
cd ..

echo -e "\n===== 打包完成 ====="
echo "发布包: ${RELEASE_DIR}/${APP_NAME}-${VERSION}-linux64.tar.gz"
echo "Docker镜像: ${RELEASE_DIR}/${APP_NAME}-${VERSION}-docker.tar"
echo "目录结构:"
if command -v tree &> /dev/null; then
  tree -L 3 ${RELEASE_DIR}
else
  echo "tree 命令未安装，使用 ls 代替:"
  ls -lR ${RELEASE_DIR} | grep -v "^$"
fi

echo -e "\n===== 清理中间文件 ====="
# 保留最终发布的tar包，清理其他中间文件
find "${RELEASE_DIR}" -mindepth 1 ! -name "*.tar.gz" ! -name "*.tar" -exec rm -rf {} +
echo "已清理所有中间文件，只保留最终发布包"