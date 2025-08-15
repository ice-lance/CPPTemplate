#!/bin/bash

# ====================
# 项目初始化脚本
# ====================

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

# 5. 创建初始git仓库
echo "初始化新的Git仓库..."
git init
git add .
git commit -m "初始化项目: $project_name $project_version"

# 6. 可选：删除自身
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
echo "1. 开始开发您的项目"
echo "2. 使用 './build-and-package.sh' 构建和打包"