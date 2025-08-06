#!/bin/bash
echo "🚀 初始化Serena项目..."

cd "$(dirname "$0")/../../.."

# 检查Serena命令
if command -v serena &> /dev/null; then
    echo "✅ Serena可用: $(which serena)"
    
    # 生成项目配置
    if [ ! -f ".serena/project.yml" ]; then
        echo "📝 生成Serena项目配置..."
        serena project generate-yml
    fi
    
    # 索引项目
    echo "🔍 索引项目代码..."
    serena project index
    
    echo "✅ Serena项目初始化完成"
else
    echo "❌ Serena不可用，请先安装Serena"
    exit 1
fi
