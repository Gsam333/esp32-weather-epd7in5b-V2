#!/bin/bash
echo "🔄 更新项目知识库..."

cd "$(dirname "$0")/../../.."

# 确保架构目录存在
mkdir -p .kiro/serena/knowledge/architecture

# 更新主知识库的架构链接部分
update_main_knowledge_base() {
    # 检查是否已有架构链接部分
    if ! grep -q "## 🏗️ 详细架构文档" .kiro/serena/knowledge/project-knowledge-base.md; then
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'

## 🏗️ 详细架构文档

### 核心架构
- [墨水屏显示架构](architecture/epd-display-architecture.md) - ESP32墨水屏技术架构详解
- [移植工具包设计](architecture/epd-display-architecture.md#移植工具包结构) - 代码移植和复用方案

### 技术栈分析
- **显示驱动**: GxEPD2库 + 多屏幕支持
- **渲染引擎**: 自定义布局系统 + 图标管理
- **数据处理**: OpenWeatherMap API + JSON解析
- **硬件抽象**: SPI接口 + GPIO控制

### 移植能力
- ✅ 支持多种墨水屏尺寸 (640x384, 800x480)
- ✅ 支持多种颜色模式 (黑白, 三色, 七色)
- ✅ 模块化架构设计，易于移植
- ✅ 完整的工具链支持

EOF
    fi
}

# 更新项目统计
echo "## 知识库更新 - $(date)" >> .kiro/serena/knowledge/project-knowledge-base.md
echo "" >> .kiro/serena/knowledge/project-knowledge-base.md
echo "### 最新统计" >> .kiro/serena/knowledge/project-knowledge-base.md

# 统计各种文件类型
for ext in cpp h py js ts rs go java c; do
    count=$(find . -name "*.$ext" | wc -l)
    if [ $count -gt 0 ]; then
        lines=$(find . -name "*.$ext" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")
        echo "- .$ext 文件: $count 个，$lines 行" >> .kiro/serena/knowledge/project-knowledge-base.md
    fi
done

echo "- 更新时间: $(date)" >> .kiro/serena/knowledge/project-knowledge-base.md
echo "" >> .kiro/serena/knowledge/project-knowledge-base.md

# 更新架构文档中的时间戳
if [ -f ".kiro/serena/knowledge/architecture/epd-display-architecture.md" ]; then
    sed -i.bak "s/\*文档更新时间: .*\*/\*文档更新时间: $(date)\*/" .kiro/serena/knowledge/architecture/epd-display-architecture.md
    rm -f .kiro/serena/knowledge/architecture/epd-display-architecture.md.bak
fi

# 更新主知识库的架构链接
update_main_knowledge_base

echo "✅ 知识库更新完成"
echo "📋 更新内容:"
echo "   - 项目统计信息已更新"
echo "   - 架构文档已同步"
echo "   - 主知识库已添加架构链接"
