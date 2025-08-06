#!/bin/bash
# Serena全局安装脚本 - 简化版

set -e

echo "🚀 Serena全局安装"
echo ""

SERENA_PATH="/Users/sanm/Documents/GitHub/serena"

# 检查源码
if [ ! -d "$SERENA_PATH" ]; then
    echo "❌ 未找到Serena源码目录: $SERENA_PATH"
    exit 1
fi

echo "✅ 找到Serena源码: $SERENA_PATH"

# 检查是否已安装
if command -v serena &> /dev/null; then
    echo "⚠️  检测到已安装的Serena"
    echo "当前版本: $(serena --version 2>/dev/null || echo '无法获取版本')"
    read -p "是否要重新安装？(y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "取消安装"
        exit 0
    fi
fi

echo ""
echo "🔍 开始全局安装..."

# 方法1: 优先使用pipx（推荐）
PIPX_PATH=""
if command -v pipx &> /dev/null; then
    PIPX_PATH="pipx"
elif [ -f "/opt/homebrew/bin/pipx" ]; then
    PIPX_PATH="/opt/homebrew/bin/pipx"
elif [ -f "/opt/homebrew/Cellar/pipx/1.7.1_1/bin/pipx" ]; then
    PIPX_PATH="/opt/homebrew/Cellar/pipx/1.7.1_1/bin/pipx"
fi

if [ -n "$PIPX_PATH" ]; then
    echo "🔧 方法1: 使用pipx全局安装..."
    if $PIPX_PATH install "$SERENA_PATH"; then
        echo "✅ pipx安装成功"
        # 确保pipx bin目录在PATH中
        if [ ! -d "$HOME/.local/bin" ]; then
            mkdir -p "$HOME/.local/bin"
        fi
        if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
            echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc
            export PATH="$HOME/.local/bin:$PATH"
        fi
        if serena --version 2>/dev/null; then
            echo "🎉 pipx全局安装完成！"
            exit 0
        fi
    else
        echo "❌ pipx安装失败，尝试其他方式..."
    fi
else
    echo "🔧 pipx未找到，跳过pipx方式..."
fi

# 方法2: 用户目录全局安装
echo ""
echo "🔧 方法2: 用户目录全局安装..."
if pip3 install -e "$SERENA_PATH" --user; then
    echo "✅ 用户目录安装成功"
    
    # 确保用户bin在PATH中
    USER_BIN="$HOME/.local/bin"
    if [[ ":$PATH:" != *":$USER_BIN:"* ]]; then
        echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc
        export PATH="$HOME/.local/bin:$PATH"
        echo "✅ PATH已更新"
    fi
    
    if command -v serena &> /dev/null && serena --version; then
        echo "🎉 用户目录全局安装完成！"
        echo "📝 请运行: source ~/.zshrc"
        exit 0
    fi
fi

# 方法3: 系统全局安装（强制）
echo ""
echo "🔧 方法3: 系统全局安装..."
if pip3 install -e "$SERENA_PATH" --break-system-packages; then
    echo "✅ 系统全局安装成功"
    if serena --version; then
        echo "🎉 系统全局安装完成！"
        exit 0
    fi
fi

# 方法4: 创建全局脚本
echo ""
echo "🔧 方法4: 创建全局脚本..."
cat > /tmp/serena_global << EOF
#!/bin/bash
export PYTHONPATH="$SERENA_PATH/src:\$PYTHONPATH"
exec python3 -m serena.cli "\$@"
EOF

if sudo cp /tmp/serena_global /usr/local/bin/serena && sudo chmod +x /usr/local/bin/serena; then
    echo "✅ 全局脚本创建成功"
    if serena --version; then
        echo "🎉 全局脚本安装完成！"
        exit 0
    fi
fi

echo ""
echo "❌ 所有安装方式都失败了"
echo "💡 手动解决方案："
echo "1. 检查Python环境: python3 --version"
echo "2. 手动测试: PYTHONPATH=$SERENA_PATH/src python3 -m serena.cli --version"
echo "3. 创建alias: echo 'alias serena=\"PYTHONPATH=$SERENA_PATH/src python3 -m serena.cli\"' >> ~/.zshrc"

exit 1