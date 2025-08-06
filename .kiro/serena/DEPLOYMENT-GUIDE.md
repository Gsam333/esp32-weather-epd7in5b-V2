# Serena工具包部署指南

## 🎯 部署概述

这个Serena工具包是一个完整的、可复用的解决方案，让你能够在任何新项目中快速集成Serena + Claude协作开发能力。

## 📦 工具包内容清单

```
serena-toolkit/                    # 工具包根目录
├── README.md                      # 主说明文档
├── QUICK-START.md                 # 5分钟快速上手指南
├── DEPLOYMENT-GUIDE.md            # 本部署指南
│
├── install/                       # 安装脚本目录
│   ├── install-serena-complete.sh    # 完整安装脚本（推荐）
│   ├── install-serena-simple.sh      # 简化安装脚本
│   └── INSTALL-GUIDE.md              # 详细安装指南
│
├── setup/                         # 项目配置目录
│   ├── setup-project.sh             # 通用项目配置脚本
│   └── project-template.yml         # 项目配置模板
│
├── scripts/                       # 辅助脚本目录
│   ├── analyze-code.sh              # 代码分析脚本
│   ├── update-knowledge.sh          # 知识库更新脚本
│   └── init-serena.sh               # Serena初始化脚本
│
├── templates/                     # 模板文件目录
│   └── knowledge-base-template.md   # 知识库模板
│
└── docs/                          # 文档目录
    ├── USAGE-GUIDE.md               # 详细使用指南
    ├── CLAUDE-COLLABORATION.md     # Claude协作指南
    └── TROUBLESHOOTING.md           # 故障排除指南
```

## 🚀 部署步骤

### 1. 准备工具包

```bash
# 确保工具包完整性
ls -la serena-toolkit/

# 检查所有脚本都有执行权限
find serena-toolkit -name "*.sh" -exec ls -l {} \;
```

### 2. 复制到新项目

```bash
# 方法1: 复制整个工具包
cp -r serena-toolkit /path/to/new-project/.kiro/serena

# 方法2: 使用rsync（推荐，可排除不需要的文件）
rsync -av --exclude='.DS_Store' serena-toolkit/ /path/to/new-project/.kiro/serena/
```

### 3. 验证部署

```bash
cd /path/to/new-project

# 检查目录结构
ls -la .kiro/serena/

# 检查脚本权限
ls -l .kiro/serena/scripts/*.sh
ls -l .kiro/serena/setup/*.sh
ls -l .kiro/serena/install/*.sh
```

### 4. 首次配置

```bash
# 如果Serena未安装，先安装
./.kiro/serena/install/install-serena-complete.sh

# 配置项目
./.kiro/serena/setup/setup-project.sh
```

## 🔧 支持的项目类型

### 自动识别的项目类型

| 项目类型 | 识别标志 | 支持程度 |
|---------|---------|----------|
| **ESP32/Arduino** | `platformio.ini` | ✅ 完全支持 |
| **Node.js** | `package.json` | ✅ 完全支持 |
| **Python** | `requirements.txt`, `pyproject.toml` | ✅ 完全支持 |
| **Rust** | `Cargo.toml` | ✅ 完全支持 |
| **Go** | `go.mod` | ✅ 完全支持 |
| **Java** | `pom.xml`, `build.gradle` | ✅ 完全支持 |
| **C/C++** | `CMakeLists.txt`, `Makefile` | ✅ 完全支持 |
| **通用项目** | 其他情况 | ⚠️ 基础支持 |

### 框架特定支持

- **React/Vue.js**: 自动识别前端框架
- **Express**: Node.js后端框架支持
- **Django/Flask**: Python Web框架支持
- **Spring Boot**: Java企业级框架支持

## 📋 部署检查清单

### 安装前检查

- [ ] 确认目标项目目录存在
- [ ] 检查磁盘空间（至少50MB）
- [ ] 确认有写入权限
- [ ] Python 3.11+ 已安装（如果需要安装Serena）

### 部署后验证

```bash
# 1. 目录结构检查
[ -d ".kiro/serena" ] && echo "✅ 目录存在" || echo "❌ 目录缺失"

# 2. 脚本权限检查
find .kiro/serena -name "*.sh" -not -executable && echo "❌ 有脚本无执行权限" || echo "✅ 脚本权限正常"

# 3. 配置文件检查
[ -f ".kiro/serena/setup/setup-project.sh" ] && echo "✅ 配置脚本存在" || echo "❌ 配置脚本缺失"

# 4. 文档完整性检查
[ -f ".kiro/serena/README.md" ] && echo "✅ 主文档存在" || echo "❌ 主文档缺失"
[ -f ".kiro/serena/QUICK-START.md" ] && echo "✅ 快速指南存在" || echo "❌ 快速指南缺失"
```

### 功能测试

```bash
# 1. 运行项目配置
./.kiro/serena/setup/setup-project.sh

# 2. 检查生成的文件
[ -f ".kiro/serena/knowledge/project-knowledge-base.md" ] && echo "✅ 知识库生成成功"

# 3. 测试辅助脚本
./.kiro/serena/scripts/analyze-code.sh
[ -f ".kiro/serena/analysis/code-analysis.md" ] && echo "✅ 代码分析成功"
```

## 🔄 批量部署

### 为多个项目批量部署

```bash
#!/bin/bash
# 批量部署脚本

PROJECTS=(
    "/path/to/project1"
    "/path/to/project2"
    "/path/to/project3"
)

for project in "${PROJECTS[@]}"; do
    echo "🚀 部署到: $project"
    
    # 复制工具包
    rsync -av serena-toolkit/ "$project/.kiro/serena/"
    
    # 进入项目目录配置
    cd "$project"
    
    # 运行配置脚本
    if ./.kiro/serena/setup/setup-project.sh; then
        echo "✅ $project 配置成功"
    else
        echo "❌ $project 配置失败"
    fi
    
    cd - > /dev/null
done
```

### 团队共享部署

```bash
# 1. 将工具包添加到团队共享位置
cp -r serena-toolkit /shared/team/tools/

# 2. 创建团队部署脚本
cat > /shared/team/tools/deploy-serena.sh << 'EOF'
#!/bin/bash
echo "🚀 团队Serena工具包部署"

if [ -z "$1" ]; then
    echo "用法: $0 <项目路径>"
    exit 1
fi

PROJECT_PATH="$1"
TOOLKIT_PATH="/shared/team/tools/serena-toolkit"

# 复制工具包
rsync -av "$TOOLKIT_PATH/" "$PROJECT_PATH/.kiro/serena/"

echo "✅ 工具包已部署到: $PROJECT_PATH"
echo "💡 下一步: cd $PROJECT_PATH && ./.kiro/serena/setup/setup-project.sh"
EOF

chmod +x /shared/team/tools/deploy-serena.sh
```

## 🔧 自定义配置

### 修改默认配置

```bash
# 编辑项目模板
vim serena-toolkit/setup/project-template.yml

# 修改知识库模板
vim serena-toolkit/templates/knowledge-base-template.md

# 自定义分析脚本
vim serena-toolkit/scripts/analyze-code.sh
```

### 添加项目特定配置

```bash
# 为特定项目类型添加配置
cat >> serena-toolkit/setup/setup-project.sh << 'EOF'
# 自定义项目类型检测
elif [ -f "custom.config" ]; then
    PROJECT_TYPE="custom"
    MAIN_LANGUAGE="custom"
    FRAMEWORK="custom-framework"
    print_success "检测到自定义项目类型"
EOF
```

## 📊 部署统计

### 收集部署信息

```bash
# 创建部署统计脚本
cat > collect-deployment-stats.sh << 'EOF'
#!/bin/bash
echo "📊 Serena工具包部署统计"
echo ""

TOTAL=0
SUCCESS=0
FAILED=0

for project in */; do
    if [ -d "$project/.kiro/serena" ]; then
        TOTAL=$((TOTAL + 1))
        if [ -f "$project/.kiro/serena/knowledge/project-knowledge-base.md" ]; then
            SUCCESS=$((SUCCESS + 1))
            echo "✅ $project"
        else
            FAILED=$((FAILED + 1))
            echo "❌ $project"
        fi
    fi
done

echo ""
echo "📈 统计结果:"
echo "- 总部署数: $TOTAL"
echo "- 成功配置: $SUCCESS"
echo "- 配置失败: $FAILED"
echo "- 成功率: $(( SUCCESS * 100 / TOTAL ))%"
EOF

chmod +x collect-deployment-stats.sh
```

## 🚀 持续改进

### 版本管理

```bash
# 为工具包添加版本标识
echo "v1.0.0" > serena-toolkit/VERSION

# 创建更新脚本
cat > update-toolkit.sh << 'EOF'
#!/bin/bash
echo "🔄 更新Serena工具包"

# 备份现有版本
if [ -d "serena-toolkit" ]; then
    mv serena-toolkit "serena-toolkit.backup.$(date +%Y%m%d)"
fi

# 下载新版本
# git clone https://github.com/your-org/serena-toolkit.git

echo "✅ 工具包更新完成"
EOF
```

### 反馈收集

```bash
# 创建反馈收集脚本
cat > serena-toolkit/scripts/collect-feedback.sh << 'EOF'
#!/bin/bash
echo "📝 Serena使用反馈收集"

{
    echo "# 使用反馈 - $(date)"
    echo "- 项目路径: $(pwd)"
    echo "- 项目类型: $(grep 'type:' .kiro/serena/config/project.yml | cut -d'"' -f2)"
    echo "- 使用时间: $(date)"
    echo "- 系统信息: $(uname -a)"
    echo ""
} >> .kiro/serena/feedback.log

echo "✅ 反馈信息已记录"
EOF
```

---

## 🎉 部署完成

恭喜！你现在拥有了一个完整的、可复用的Serena工具包。

### 下一步行动

1. **测试工具包**：在一个示例项目中测试所有功能
2. **团队分享**：将工具包分享给团队成员
3. **持续改进**：根据使用反馈不断优化工具包
4. **文档维护**：保持文档与功能同步更新

### 获得帮助

- 查看 `QUICK-START.md` 快速上手
- 参考 `docs/USAGE-GUIDE.md` 详细使用说明
- 遇到问题查看 `docs/TROUBLESHOOTING.md`
- 学习协作技巧查看 `docs/CLAUDE-COLLABORATION.md`

**让Serena + Claude成为你在每个项目中的智能开发伙伴！** 🚀