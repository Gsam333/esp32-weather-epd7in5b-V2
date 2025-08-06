# Serena工具包使用指南

## 🎯 概述

Serena工具包是一个可复用的项目配置工具集，帮助你快速在任何项目中集成Serena + Claude协作开发能力。

## 📦 工具包结构

```
serena-toolkit/
├── install/                    # 安装脚本
│   ├── install-serena-complete.sh    # 完整安装脚本
│   ├── install-serena-simple.sh      # 简化安装脚本
│   └── INSTALL-GUIDE.md              # 安装指南
├── setup/                      # 项目配置
│   ├── setup-project.sh             # 项目配置脚本
│   └── project-template.yml         # 配置模板
├── scripts/                    # 辅助脚本
│   ├── analyze-code.sh              # 代码分析
│   ├── update-knowledge.sh          # 知识库更新
│   └── init-serena.sh               # Serena初始化
├── templates/                  # 模板文件
└── docs/                       # 文档
```

## 🚀 使用流程

### 1. 复制工具包到新项目

```bash
# 复制整个工具包
cp -r serena-toolkit /path/to/new-project/.kiro/serena

# 进入新项目目录
cd /path/to/new-project
```

### 2. 安装Serena（如果未安装）

```bash
# 检查Serena是否已安装
serena --help

# 如果未安装，运行完整安装脚本
./.kiro/serena/install/install-serena-complete.sh
```

### 3. 配置项目

```bash
# 运行项目配置脚本
./.kiro/serena/setup/setup-project.sh
```

这个脚本会：
- 🔍 自动识别项目类型（ESP32、Node.js、Python、Rust、Go等）
- 📁 创建完整的目录结构
- ⚙️ 生成项目特定的配置文件
- 📚 创建详细的项目知识库
- 🔧 设置辅助脚本

### 4. 查看生成的知识库

```bash
# 查看主知识库
cat .kiro/serena/knowledge/project-knowledge-base.md

# 查看代码分析结果
cat .kiro/serena/analysis/code-analysis.md
```

## 🔧 日常使用

### 代码分析

```bash
# 运行代码分析
./.kiro/serena/scripts/analyze-code.sh

# 查看分析结果
cat .kiro/serena/analysis/code-analysis.md
```

### 知识库更新

```bash
# 更新知识库（在代码变更后）
./.kiro/serena/scripts/update-knowledge.sh
```

### Serena项目管理

```bash
# 初始化Serena项目
./.kiro/serena/scripts/init-serena.sh

# 重新索引项目
serena project index

# 生成项目配置
serena project generate-yml
```

## 💡 与Claude协作

### 提供项目上下文

在与Claude对话时，这样开始：

> "我正在开发一个项目，以下是项目的详细信息：
> 
> [复制 .kiro/serena/knowledge/project-knowledge-base.md 的内容]
> 
> 基于这些信息，我想要 [具体需求]，请提供详细的实现方案。"

### 最佳实践

1. **始终提供完整上下文**：包含项目知识库信息
2. **具体描述需求**：明确说明要实现的功能
3. **引用现有架构**：让Claude基于现有代码结构提供建议
4. **及时更新知识库**：在重要变更后更新项目信息

### 常用提问模式

```
# 功能开发
"基于项目知识库，如何在现有架构中添加[新功能]？"

# 代码优化
"根据项目特点，如何优化[具体模块]的性能？"

# 问题解决
"项目中遇到[具体问题]，基于现有技术栈如何解决？"

# 架构设计
"要实现[功能需求]，在当前架构下最佳的设计方案是什么？"
```

## 📊 支持的项目类型

### 嵌入式项目
- ✅ ESP32/Arduino (PlatformIO)
- ✅ STM32 (CubeMX)
- ✅ 通用嵌入式C/C++

### Web项目
- ✅ Node.js/Express
- ✅ React/Vue.js
- ✅ TypeScript项目

### 应用开发
- ✅ Python (pip/poetry)
- ✅ Java (Maven/Gradle)
- ✅ C# (.NET)

### 系统编程
- ✅ Rust (Cargo)
- ✅ Go (Go modules)
- ✅ C/C++ (CMake/Make)

## 🔄 维护和更新

### 定期维护

```bash
# 每周或重要变更后
./.kiro/serena/scripts/update-knowledge.sh

# 重新索引项目
serena project index
```

### 团队协作

1. **版本控制**：将`.kiro/serena`目录纳入Git管理
2. **知识共享**：团队成员共享项目知识库
3. **标准化**：使用统一的工具包配置

### 自定义扩展

你可以根据项目需要自定义：

1. **修改模板**：编辑`templates/`下的模板文件
2. **扩展脚本**：在`scripts/`目录添加自定义脚本
3. **调整配置**：修改`setup/project-template.yml`

## 🆘 故障排除

### Serena命令不可用

```bash
# 检查安装
which serena

# 重新安装
./.kiro/serena/install/install-serena-complete.sh
```

### 项目类型识别错误

手动编辑配置文件：
```bash
# 编辑项目配置
vim .kiro/serena/config/project.yml
```

### 知识库内容不完整

```bash
# 重新生成知识库
rm -rf .kiro/serena/knowledge/
./.kiro/serena/setup/setup-project.sh
```

### 代码索引失败

```bash
# 检查项目配置
cat .serena/project.yml

# 重新索引
serena project index
```

## 📈 高级用法

### 自定义分析脚本

创建项目特定的分析脚本：

```bash
# 创建自定义脚本
cat > .kiro/serena/scripts/custom-analysis.sh << 'EOF'
#!/bin/bash
echo "🔍 自定义项目分析..."
# 添加你的分析逻辑
EOF

chmod +x .kiro/serena/scripts/custom-analysis.sh
```

### 集成CI/CD

在CI/CD流水线中使用：

```yaml
# .github/workflows/serena-update.yml
name: Update Serena Knowledge Base
on:
  push:
    branches: [main]
jobs:
  update-knowledge:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Update Knowledge Base
        run: ./.kiro/serena/scripts/update-knowledge.sh
```

---
*通过Serena工具包，让每个项目都拥有智能开发助手！*