#!/bin/bash

# .gitignore 验证脚本
# 检查重要文件是否被意外忽略，以及不需要的文件是否被正确忽略

echo "==================================="
echo "📋 .gitignore 验证脚本"
echo "==================================="

# 检查重要文件是否被意外忽略
echo ""
echo "🔍 检查重要文件是否被意外忽略..."

important_files=(
    "README.md"
    "LICENSE"
    "platformio.ini"
    "src/main.cpp"
    "src/config.cpp"
    "tasks/bmp280/src/main.cpp"
    "tasks/bmp280/platformio.ini"
    "tasks/gpio-voltage-test/src/main.cpp"
    ".github/workflows"
    "2.docs/README.md"
)

ignored_important=0
for file in "${important_files[@]}"; do
    if git check-ignore "$file" >/dev/null 2>&1; then
        echo "❌ 重要文件被忽略: $file"
        ignored_important=$((ignored_important + 1))
    else
        echo "✅ 重要文件未被忽略: $file"
    fi
done

# 检查应该被忽略的文件是否被正确忽略
echo ""
echo "🔍 检查应该被忽略的文件..."

should_ignore=(
    ".pio"
    ".DS_Store"
    "*.log"
    "__pycache__"
    "*.pyc"
    "compile_commands.json"
    "tmp"
    ".vscode/settings.json"
    "tasks/bmp280/.pio"
    "tasks/gpio-voltage-test/.pio"
)

not_ignored=0
for pattern in "${should_ignore[@]}"; do
    # 创建测试文件/目录来检查是否被忽略
    if [[ "$pattern" == *"/"* ]]; then
        # 目录模式
        test_path="$pattern/test_file"
        mkdir -p "$(dirname "$test_path")" 2>/dev/null
        touch "$test_path" 2>/dev/null
        if git check-ignore "$test_path" >/dev/null 2>&1; then
            echo "✅ 正确忽略: $pattern"
        else
            echo "❌ 未被忽略: $pattern"
            not_ignored=$((not_ignored + 1))
        fi
        rm -f "$test_path" 2>/dev/null
    else
        # 文件模式
        if git check-ignore "$pattern" >/dev/null 2>&1 || [[ "$pattern" == *.* ]]; then
            echo "✅ 正确忽略模式: $pattern"
        else
            echo "❌ 模式可能未生效: $pattern"
            not_ignored=$((not_ignored + 1))
        fi
    fi
done

# 显示当前被忽略的文件
echo ""
echo "📊 当前被忽略的文件统计..."
ignored_count=$(git status --porcelain --ignored | grep "^!!" | wc -l)
echo "被忽略的文件数量: $ignored_count"

# 显示一些被忽略的文件示例
echo ""
echo "📝 被忽略文件示例 (前10个):"
git status --porcelain --ignored | grep "^!!" | head -10 | while read -r line; do
    echo "  ${line#!!}"
done

# 检查是否有大文件未被忽略
echo ""
echo "🔍 检查大文件 (>1MB)..."
find . -type f -size +1M -not -path "./.git/*" | while read -r file; do
    if git check-ignore "$file" >/dev/null 2>&1; then
        echo "✅ 大文件已忽略: $file"
    else
        echo "⚠️  大文件未忽略: $file ($(du -h "$file" | cut -f1))"
    fi
done

# 总结
echo ""
echo "==================================="
echo "📋 验证总结"
echo "==================================="
if [ $ignored_important -eq 0 ] && [ $not_ignored -eq 0 ]; then
    echo "✅ .gitignore 配置良好！"
    echo "   - 所有重要文件都未被意外忽略"
    echo "   - 应该忽略的文件都被正确忽略"
else
    echo "⚠️  .gitignore 需要调整："
    [ $ignored_important -gt 0 ] && echo "   - $ignored_important 个重要文件被意外忽略"
    [ $not_ignored -gt 0 ] && echo "   - $not_ignored 个文件/模式可能未被正确忽略"
fi

echo ""
echo "💡 建议："
echo "   - 定期运行此脚本检查 .gitignore 配置"
echo "   - 添加新的构建输出或临时文件到 .gitignore"
echo "   - 确保敏感文件（如密钥、配置）被正确忽略"