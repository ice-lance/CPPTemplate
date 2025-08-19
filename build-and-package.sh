#!/bin/bash
set -e  # 遇到错误立即退出

# ========== 自动配置区域 ==========
# 从CMakeLists.txt提取项目名称
APP_NAME=$(grep 'project(' CMakeLists.txt | sed -E 's/project\s*\(\s*([^ \n]+).*/\1/i' | tr -d ' \n')
if [ -z "$APP_NAME" ]; then
    APP_NAME="DefaultApp"
fi

# 清理应用名称：移除特殊字符
CLEAN_APP_NAME=$(echo "$APP_NAME" | tr -dc '[:alnum:]-_')  # 只保留字母数字、连字符和下划线
if [ -z "$CLEAN_APP_NAME" ]; then
    CLEAN_APP_NAME="App"
fi

# 从CMakeLists.txt提取版本
DEFAULT_VERSION=$(grep 'project(' CMakeLists.txt | grep -o 'VERSION [^ )]*' | awk '{print $2}')
if [ -z "$DEFAULT_VERSION" ]; then
    DEFAULT_VERSION="1.0.0"
fi

BUILD_DIR="build"                # 编译目录
RELEASE_DIR="release"            # 发布目录
INSTALL_PREFIX="/usr/local"      # 安装路径前缀
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
cp ${EXECUTABLE} ../${RELEASE_DIR}/${CLEAN_APP_NAME}  # 使用清理后的名称

# 在构建目录中获取版本信息
echo "===== 获取版本信息 ====="
VERSION_INFO=""
# 尝试获取版本信息
if ./${EXECUTABLE} --version > /dev/null 2>&1; then
    VERSION_INFO=$(./${EXECUTABLE} --version 2>&1 | head -n1)
    echo "获取到版本信息: ${VERSION_INFO}"
else
    echo "警告：无法获取可执行文件的版本信息"
    VERSION_INFO="版本信息不可用"
fi

echo "===== 收集动态库依赖（完整递归方案） ====="
cd ../${RELEASE_DIR}
mkdir -p libs

# 递归收集所有依赖库的函数
collect_libs() {
    local bin="$1"
    local bin_path=$(realpath "$bin")
    
    # 获取所有依赖库
    ldd "$bin_path" | awk '/=> \// {print $3}' | while read lib; do
        # 跳过已存在的库
        lib_basename=$(basename "$lib")
        if [ -f "libs/$lib_basename" ]; then
            continue
        fi
        
        # 复制库文件
        if [ -f "$lib" ]; then
            echo "收集: $lib"
            cp -v "$lib" libs/
            
            # 递归收集这个库的依赖
            collect_libs "$lib"
        fi
    done
}

# 收集主程序依赖
collect_libs ${CLEAN_APP_NAME}

echo "===== 创建库文件的SONAME符号链接 ====="
cd libs
for lib in *; do
    # 如果是文件（不是符号链接）并且是共享库
    if [ -f "$lib" ] && [[ "$lib" == *.so* ]]; then
        # 获取SONAME
        soname=$(objdump -p "$lib" 2>/dev/null | awk '/SONAME/ {print $2}')
        if [ -n "$soname" ] && [ ! -e "$soname" ]; then
            echo "创建SONAME链接: $soname -> $lib"
            ln -s "$lib" "$soname"
        fi
        
        # 获取NEEDED库并创建链接
        objdump -p "$lib" 2>/dev/null | awk '/NEEDED/ {print $2}' | while read needed; do
            if [ -f "$needed" ] && [ ! -e "$needed" ]; then
                echo "创建NEEDED链接: $needed -> $lib"
                ln -s "$lib" "$needed"
            fi
        done
    fi
done
cd ..

echo "===== 创建增强型启动脚本 ====="
cat > run.sh << EOF
#!/bin/bash
# 增强目录处理 (关键修复)
SCRIPT_DIR="\$(cd "\$(dirname "\${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
if [ ! -d "\${SCRIPT_DIR}" ]; then
    echo "错误：脚本目录不存在！" >&2
    exit 1
fi
cd "\${SCRIPT_DIR}" || {
    echo "错误：无法切换到脚本目录！" >&2
    exit 1
}

# 设置库路径
LIB_DIR="\${SCRIPT_DIR}/libs"
export LD_LIBRARY_PATH="\${LIB_DIR}:\${LD_LIBRARY_PATH}"

# 调试信息
if [ "\${DEBUG}" = "1" ]; then
    echo "===== 环境调试信息 ====="
    echo "脚本目录: \${SCRIPT_DIR}"
    echo "工作目录: \$(pwd)"
    echo "LD_LIBRARY_PATH: \${LD_LIBRARY_PATH}"
    echo "目录内容:"
    ls -l
    echo "库目录内容:"
    ls -l libs/
    echo "依赖检查:"
    ldd \${SCRIPT_DIR}/${CLEAN_APP_NAME}
    echo "===== 开始执行 ====="
fi

# 执行程序
exec "\${SCRIPT_DIR}/${CLEAN_APP_NAME}" "\$@"
EOF
chmod +x run.sh

echo "===== 创建systemd用户服务文件 ====="
cat > ${CLEAN_APP_NAME}.service << EOF
[Unit]
Description=${APP_NAME} Service
After=network.target

[Service]
Type=simple
# 使用动态路径占位符 (关键修复)
WorkingDirectory=@INSTALL_DIR@

# 设置库路径
Environment="LD_LIBRARY_PATH=@INSTALL_DIR@/libs"
Environment="DEBUG=0"  # 设置为1可启用调试

# 使用绝对路径
ExecStart=@INSTALL_DIR@/run.sh
Restart=on-failure
RestartSec=5s

# 安全增强
NoNewPrivileges=yes
PrivateTmp=yes

[Install]
WantedBy=default.target
EOF

echo "===== 创建增强型安装脚本 ====="
cat > install.sh << EOF
#!/bin/bash
set -e

# 获取用户主目录（确保正确性）
USER_HOME=\$(getent passwd \$USER | cut -d: -f6)
if [ -z "\${USER_HOME}" ] || [ ! -d "\${USER_HOME}" ]; then
    echo "错误：无法获取有效的用户主目录！"
    exit 1
fi

APP_INSTALL_DIR="\${USER_HOME}/apps/${CLEAN_APP_NAME}"

echo "===== [1/7] 验证用户信息 ====="
echo "用户: \$USER"
echo "主目录: \${USER_HOME}"
echo "安装目录: \${APP_INSTALL_DIR}"

echo "===== [2/7] 创建安装目录 ====="
mkdir -p "\${APP_INSTALL_DIR}" || {
    echo "错误：无法创建安装目录！"
    exit 1
}
echo "✓ 目录创建成功: \${APP_INSTALL_DIR}"

echo "===== [3/7] 复制文件到安装目录 ====="
cp -r ./* "\${APP_INSTALL_DIR}/" || {
    echo "错误：文件复制失败！"
    exit 1
}
echo "✓ 文件复制完成"

echo "===== [4/7] 设置安装目录权限 ====="
chmod 755 "\${APP_INSTALL_DIR}"
chmod 755 "\${APP_INSTALL_DIR}/run.sh"
echo "✓ 权限设置完成"

echo "===== [5/7] 安装并配置systemd用户服务 ====="
USER_SERVICE_DIR="\${USER_HOME}/.config/systemd/user"
mkdir -p "\${USER_SERVICE_DIR}"

# 关键修复：正确引用服务文件路径
SERVICE_SOURCE="\${APP_INSTALL_DIR}/${CLEAN_APP_NAME}.service"
if [ ! -f "\${SERVICE_SOURCE}" ]; then
    echo "错误：源服务文件不存在: \${SERVICE_SOURCE}"
    exit 1
fi

# 替换服务文件中的占位符
sed "s|@INSTALL_DIR@|\${APP_INSTALL_DIR}|g" \\
    "\${SERVICE_SOURCE}" \\
    > "\${USER_SERVICE_DIR}/${CLEAN_APP_NAME}.service"

echo "✓ 服务文件已配置并复制到用户目录"

echo "===== [6/7] 验证服务文件配置 ====="
SERVICE_FILE="\${USER_SERVICE_DIR}/${CLEAN_APP_NAME}.service"

# 检查服务文件是否存在
if [ ! -f "\${SERVICE_FILE}" ]; then
    echo "错误：服务文件未创建！"
    exit 1
fi

# 检查工作目录是否存在
WORKING_DIR=\$(grep 'WorkingDirectory=' "\${SERVICE_FILE}" | cut -d= -f2)
if [ ! -d "\${WORKING_DIR}" ]; then
    echo "错误：工作目录不存在: \${WORKING_DIR}"
    exit 1
fi
echo "✓ 工作目录验证通过: \${WORKING_DIR}"

# 检查可执行文件是否存在
EXEC_START=\$(grep 'ExecStart=' "\${SERVICE_FILE}" | cut -d= -f2)
if [ ! -f "\${EXEC_START}" ]; then
    echo "错误：可执行文件不存在: \${EXEC_START}"
    exit 1
fi
echo "✓ 可执行文件验证通过: \${EXEC_START}"

echo "===== [7/7] 启用并启动用户服务 ====="
systemctl --user daemon-reload

# 添加服务启动前的最终检查
echo "执行最终启动前检查:"
echo "安装目录内容:"
ls -l "\${APP_INSTALL_DIR}"
echo "库目录内容:"
ls -l "\${APP_INSTALL_DIR}/libs"

systemctl --user enable "${CLEAN_APP_NAME}.service"
systemctl --user start "${CLEAN_APP_NAME}.service"

echo "等待服务启动..."
sleep 2  # 给服务启动时间

SERVICE_STATUS=\$(systemctl --user is-active "${CLEAN_APP_NAME}.service" 2>&1)
if [ "\${SERVICE_STATUS}" = "active" ]; then
    echo "✓ 用户服务已成功启动"
else
    echo "错误：服务启动失败！状态: \${SERVICE_STATUS}"
    echo "尝试手动启动以获取更多信息:"
    echo "systemctl --user start '${CLEAN_APP_NAME}.service'"
    echo "查看详细日志:"
    echo "journalctl --user-unit='${CLEAN_APP_NAME}.service' -e --since '1 min ago'"
    echo "您可能需要启用linger以在用户注销后保持服务运行："
    echo "loginctl enable-linger \$USER"
    exit 1
fi

echo -e "\n✅ 安装成功！"
echo "========================================"
echo "服务名称: ${CLEAN_APP_NAME}"
echo "安装目录: \${APP_INSTALL_DIR}"
echo "服务文件: \${SERVICE_FILE}"
echo "启动命令: systemctl --user start ${CLEAN_APP_NAME}"
echo "状态检查: systemctl --user status ${CLEAN_APP_NAME}"
echo "日志查看: journalctl --user-unit=${CLEAN_APP_NAME}.service -e"
echo "调试模式: 编辑服务文件设置 DEBUG=1"
echo "卸载命令: \${APP_INSTALL_DIR}/uninstall.sh"
echo "========================================"
echo "注意：用户服务在注销后会自动停止"
echo "如需长期运行，请执行:"
echo "  loginctl enable-linger \$USER"
echo "========================================"
EOF
chmod +x install.sh

echo "===== 创建增强型卸载脚本 ====="
cat > uninstall.sh << EOF
#!/bin/bash
set -e

# 获取用户主目录（确保正确性）
USER_HOME=\$(getent passwd \$USER | cut -d: -f6)
if [ -z "\${USER_HOME}" ] || [ ! -d "\${USER_HOME}" ]; then
    echo "错误：无法获取有效的用户主目录！"
    exit 1
fi

APP_INSTALL_DIR="\${USER_HOME}/apps/${CLEAN_APP_NAME}"

echo "===== [1/6] 验证用户信息 ====="
echo "用户: \$USER"
echo "主目录: \${USER_HOME}"
echo "安装目录: \${APP_INSTALL_DIR}"

echo "===== [2/6] 停止用户服务 ====="
systemctl --user stop "${CLEAN_APP_NAME}.service" || {
    echo "警告：停止服务失败，可能服务未运行"
}

echo "===== [3/6] 禁用用户服务 ====="
systemctl --user disable "${CLEAN_APP_NAME}.service" || {
    echo "警告：禁用服务失败"
}

echo "===== [4/6] 删除用户服务文件 ====="
USER_SERVICE_DIR="\${USER_HOME}/.config/systemd/user"
SERVICE_FILE="\${USER_SERVICE_DIR}/${CLEAN_APP_NAME}.service"

if [ -f "\${SERVICE_FILE}" ]; then
    rm -f "\${SERVICE_FILE}"
    echo "✓ 服务文件已删除"
    systemctl --user daemon-reload
else
    echo "ℹ️ 服务文件不存在，跳过删除"
fi

echo "===== [5/6] 删除安装目录 ====="
if [ -d "\${APP_INSTALL_DIR}" ]; then
    rm -rf "\${APP_INSTALL_DIR}"
    echo "✓ 安装目录已删除: \${APP_INSTALL_DIR}"
else
    echo "ℹ️ 安装目录不存在，跳过删除"
fi

echo "===== [6/6] 清理残留配置 ====="
# 清理可能存在的临时文件
rm -f "\${USER_HOME}/.${CLEAN_APP_NAME}_backup" 2>/dev/null || true

echo -e "\n✅ 卸载完成！"
echo "========================================"
echo "应用程序 ${CLEAN_APP_NAME} 已完全移除"
echo "========================================"
EOF
chmod +x uninstall.sh

echo "===== 创建版本信息文件 ====="
echo "Application: ${APP_NAME}" > version.txt
echo "Version: ${VERSION}" >> version.txt
echo "Build date: $(date '+%Y-%m-%d %H:%M:%S')" >> version.txt
echo "System: $(uname -a)" >> version.txt
echo "Executable Version: ${VERSION_INFO}" >> version.txt
echo "Included Libraries:" >> version.txt
find libs -maxdepth 1 -type f -exec basename {} \; >> version.txt

echo "===== 创建使用说明文档 ====="
mkdir -p docs
cat > docs/INSTRUCTIONS.txt << EOF
${APP_NAME} ${VERSION} 使用说明

一、安装步骤：
1. 解压发布包：
   tar xzf ${CLEAN_APP_NAME}-${VERSION}-linux64.tar.gz
   cd ${CLEAN_APP_NAME}-${VERSION}-linux64

2. 运行安装脚本：
   ./install.sh   # 不需要root权限！

3. 服务管理：
   - 启动服务: systemctl --user start ${CLEAN_APP_NAME}
   - 停止服务: systemctl --user stop ${CLEAN_APP_NAME}
   - 查看状态: systemctl --user status ${CLEAN_APP_NAME}
   - 启用自启: systemctl --user enable ${CLEAN_APP_NAME}

4. 查看日志：
   journalctl --user-unit=${CLEAN_APP_NAME}.service -e

二、常见问题解决：
1. 如果启动失败，尝试启用调试模式：
   nano ~/.config/systemd/user/${CLEAN_APP_NAME}.service
   修改以下行：
     Environment="DEBUG=1"
   然后重启服务：systemctl --user restart ${CLEAN_APP_NAME}
   查看日志获取详细信息

2. 如需长期后台运行（即使注销后）：
   loginctl enable-linger \$USER

3. 如果遇到路径错误：
   - 检查服务文件中的路径：~/.config/systemd/user/${CLEAN_APP_NAME}.service
   - 确保路径正确指向：${USER_HOME}/apps/${CLEAN_APP_NAME}

三、卸载步骤：
1. 运行卸载脚本：
   ~/apps/${CLEAN_APP_NAME}/uninstall.sh

四、注意事项：
1. 所有操作都在用户权限下完成
2. 默认安装目录: ~/apps/${CLEAN_APP_NAME}
3. 服务文件位置: ~/.config/systemd/user/${CLEAN_APP_NAME}.service

五、配置文件说明：
1. 默认配置文件位置: ~/apps/${CLEAN_APP_NAME}/config.yaml
2. 修改配置后需要重启服务:
   systemctl --user restart ${CLEAN_APP_NAME}
3. 配置模板内容:
   $(cat config.yaml 2>/dev/null || echo "未找到配置文件")
EOF

echo "===== 复制配置文件 ====="
# 从项目根目录复制配置文件
if [ -f "../config.yaml" ]; then
    cp -v ../config.yaml .
    echo "✓ 配置文件已复制"
else
    echo "警告：未找到项目根目录的 config.yaml 文件！"
    # 不退出，但创建空文件防止打包失败
    touch config.yaml
    echo "已创建空 config.yaml 占位"
fi

echo "===== 打包发布文件 ====="
tar czf ${CLEAN_APP_NAME}-${VERSION}-linux64.tar.gz \
  ${CLEAN_APP_NAME} \
  run.sh \
  version.txt \
  libs/ \
  ${CLEAN_APP_NAME}.service \
  install.sh \
  uninstall.sh \
  docs/ \
  config.yaml

echo "===== 清理中间文件 ====="
# 保留最终发布的tar包，清理其他中间文件
find . -mindepth 1 ! -name "*.tar.gz" ! -name "*.tar" -exec rm -rf {} +
echo "已清理所有中间文件，只保留最终发布包"

cd ..

echo -e "\n===== 打包完成 ====="
echo "发布包: ${RELEASE_DIR}/${CLEAN_APP_NAME}-${VERSION}-linux64.tar.gz"
echo -e "\n✅ 增强型用户级安装方案完成！"
echo "========================================"
echo "关键修复："
echo "1. 修复了服务文件路径处理中的引号问题"
echo "2. 添加了服务文件存在性检查"
echo "3. 改进了路径拼接逻辑"
echo "4. 增强了错误处理机制"
echo "========================================"
echo "在目标设备上部署后，如果仍有问题："
echo "1. 在安装时启用调试模式: DEBUG=1 ./install.sh"
echo "2. 检查服务文件: ~/.config/systemd/user/${CLEAN_APP_NAME}.service"
echo "3. 确保路径正确: ~/apps/${CLEAN_APP_NAME}"
echo "========================================"

echo -e "\n✅ 所有打包操作已完成！"