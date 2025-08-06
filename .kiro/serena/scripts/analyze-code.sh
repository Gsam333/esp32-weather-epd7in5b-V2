#!/bin/bash
# 项目代码分析脚本

echo "🔍 开始项目代码分析..."

# 创建分析目录
mkdir -p .kiro/serena/analysis

# 基本统计
echo "📊 项目统计分析..."
{
    echo "# 项目代码分析报告"
    echo ""
    echo "生成时间: $(date)"
    echo ""
    echo "## 文件统计"
    echo ""
    echo "### 按文件类型统计"
    find . -type f -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.py" -o -name "*.js" -o -name "*.ts" | \
    sed 's/.*\.//' | sort | uniq -c | sort -nr | \
    awk '{printf "- %s: %d 个文件\n", $2, $1}'
    echo ""
    echo "### 代码行数统计"
    find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.py" -o -name "*.js" -o -name "*.ts" \) \
    -exec wc -l {} + 2>/dev/null | tail -1 | awk '{printf "- 总代码行数: %d 行\n", $1}'
    echo ""
} > .kiro/serena/analysis/code-analysis.md

echo "✅ 代码分析完成，结果保存到 .kiro/serena/analysis/code-analysis.md"
