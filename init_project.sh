#!/bin/bash

# ====================
# 项目初始化脚本（含文件夹重命名）
# ====================

# 保存当前目录的绝对路径
original_dir=$(pwd)

# 验证执行位置
if [ ! -f "CMakeLists.txt" ] || [ ! -d "src" ]; then
    echo "错误：必须在项目根目录执行此脚本"
    exit 1
fi

# 获取新项目名
read -p "输入新项目名称: " project_name
if [ -z "$project_name" ]; then
    echo "错误：项目名称不能为空"
    exit 1
fi

# 获取初始版本号
read -p "输入初始版本号(默认1.0.0): " project_version
project_version=${project_version:-"1.0.0"}

echo "正在初始化项目: $project_name (版本 $project_version)"

# 1. 替换CMakeLists.txt中的占位符
echo "更新CMake配置..."
sed -i.bak "s/@PROJECT_NAME@/$project_name/g" CMakeLists.txt
sed -i.bak "s/@PROJECT_VERSION@/$project_version/g" CMakeLists.txt
rm CMakeLists.txt.bak

# 2. 更新版本头文件模板
echo "配置版本头文件..."
sed -i.bak "s/@PROJECT_NAME@/$project_name/g" include/version.h.in
sed -i.bak "s/@PROJECT_VERSION@/$project_version/g" include/version.h.in
rm include/version.h.in.bak

# 3. 清理git历史
echo "清除原有Git历史..."
rm -rf .git

# 4. 使构建脚本可执行
chmod +x build-and-package.sh

# --- 新增关键步骤：重命名项目文件夹 ---
current_dir_name=$(basename "$original_dir")
if [ "$current_dir_name" != "$project_name" ]; then
    echo "重命名项目文件夹: '$current_dir_name' -> '$project_name'"
    
    # 移动到父目录执行重命名
    cd ..
    mv -v "$current_dir_name" "$project_name"
    cd "$project_name" || exit 1
    
    echo "✅ 文件夹已重命名"
    echo "注意：当前工作目录已变更到新路径: $(pwd)"
fi

# 5. 创建初始git仓库
echo "初始化新的Git仓库..."
git init
git add .
git commit -m "初始化项目: $project_name $project_version"

# 6. VSCode专项提示
echo -e "\n\033[1;34m=== VSCode 用户指引 ===\033[0m"
if [ "$current_dir_name" != "$project_name" ]; then
    echo -e "请重新打开项目：\n  1. 关闭当前VSCode窗口"
    echo -e "  2. 选择 File > Open Folder"
    echo -e "  3. 导航到新路径: \033[1;33m$(pwd)\033[0m"
else
    echo -e "请重新加载窗口使更改生效：\n  \033[1;33mCtrl+Shift+P > Reload Window\033[0m"
fi

# 7. 可选：删除自身
read -p "初始化完成！是否删除初始化脚本? [y/N] " yn
case $yn in
    [Yy]* ) 
        echo "删除初始化脚本..."
        rm -- "$0"
        ;;
    * )
        echo "保留初始化脚本"
        ;;
esac

echo -e "\n✅ 项目 '$project_name' 初始化成功！"
echo "下一步:"
echo "1. 在VSCode中重新打开项目"
echo "2. 开始开发您的项目"
echo "3. 使用 './build-and-package.sh' 构建和打包"