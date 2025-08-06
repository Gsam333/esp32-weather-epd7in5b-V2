# Serena 项目助手使用指南

## 📖 概述
这是为 esp32-weather-epd7in5b-V2 项目自动生成的Serena配置，包含项目知识库、分析工具和辅助脚本。

## 📁 目录结构
```
.kiro/serena/
├── config/          # 配置文件
│   └── project.yml  # 项目配置
├── knowledge/       # 知识库
│   └── project-knowledge-base.md
├── scripts/         # 辅助脚本
│   └── analyze-code.sh
├── analysis/        # 分析结果
└── README.md        # 使用指南
```

## 🚀 快速开始

### 1. 查看项目知识库
```bash
cat .kiro/serena/knowledge/project-knowledge-base.md
```

### 2. 运行代码分析
```bash
./.kiro/serena/scripts/analyze-code.sh
```

### 3. 与Claude协作
在对话中引用知识库文件，让Claude更好地理解项目上下文：
```
请参考 #.kiro/serena/knowledge/project-knowledge-base.md 来帮我...
```

## 🔧 自定义配置
编辑 `.kiro/serena/config/project.yml` 来调整项目配置。

## 🌐 多语言支持
重新运行脚本时可以选择语言：
```bash
# 生成英文版知识库
./.kiro/serena/setup/setup-project-multilang.sh --en

# 生成中文版知识库
./.kiro/serena/setup/setup-project-multilang.sh --zh
```

## 📚 更多资源
- [Serena官方文档](https://github.com/serena-ai/serena)
- [项目知识库](.kiro/serena/knowledge/project-knowledge-base.md)
