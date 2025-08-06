# Serena工具包 - 快速开始指南

## 🎯 5分钟快速上手

### Step 1: 复制工具包
```bash
# 将工具包复制到新项目
cp -r serena-toolkit /path/to/your-project/.kiro/serena
cd /path/to/your-project
```

### Step 2: 检查Serena安装
```bash
# 检查Serena是否已安装
serena --help

# 如果未安装，运行安装脚本
./.kiro/serena/install/install-serena-complete.sh
```

### Step 3: 配置项目
```bash
# 1. 一键配置当前项目（必须）
./.kiro/serena/setup/setup-project.sh
# 2. 初始化Serena（推荐）
./.kiro/serena/scripts/init-serena.sh
```

### Step 4: 查看生成的知识库
```bash
# 查看项目知识库
cat .kiro/serena/knowledge/project-knowledge-base.md
```

### Step 5: 与Claude协作
在Claude对话中这样提问：
基于 .kiro/serena/knowledge/project-knowledge-base.md 中的项目信息，请帮我分析现有架构并提供开发建议，我想要实现 [具体功能需求]，请提供详细的实现方案。"

## 🔄 日常使用流程

### 开发新功能前
```bash
# 更新知识库
./.kiro/serena/scripts/update-knowledge.sh

# 查看最新项目信息
cat .kiro/serena/knowledge/project-knowledge-base.md
```

### 与Claude协作
1. 复制知识库内容到Claude对话
2. 描述具体需求
3. 获得基于项目上下文的精准建议
4. 实现代码后更新知识库

### 代码变更后
```bash
# 重新分析代码
./.kiro/serena/scripts/analyze-code.sh

# 更新知识库
./.kiro/serena/scripts/update-knowledge.sh
```

## 📝 常用命令

```bash
# 查看项目配置
cat .serena/project.yml

# 如果Serena配置有问题
serena project generate-yml    # 重新生成项目配置文件
serena project index           # 重新索引项目代码（更新符号缓存）
# 或者一键修复
./.kiro/serena/scripts/init-serena.sh

# 工具包脚本
./.kiro/serena/scripts/analyze-code.sh      # 代码分析
./.kiro/serena/scripts/update-knowledge.sh  # 更新知识库
./.kiro/serena/scripts/init-serena.sh       # 初始化Serena

# 查看文档
cat .kiro/serena/knowledge/project-knowledge-base.md    # 项目知识库
cat .kiro/serena/knowledge/architecture/system-architecture.md  # 架构文档

#清理缓存：
# 删除符号缓存（下次索引时会重新生成）
rm -rf .serena/cache/
# 重新索引
serena project index

```

## 🎯 最佳实践

1. **项目开始时**：立即配置Serena，建立知识库基础
2. **定期更新**：每次重要代码变更后更新知识库
3. **团队共享**：将知识库纳入版本控制，团队共享
4. **Claude协作**：始终提供完整项目上下文，获得更好建议

## ⚡ 故障排除

### Serena命令不可用
```bash
# 检查安装
which serena

# 重新安装
./.kiro/serena/install/install-serena-complete.sh
```

### 项目配置失败
```bash
# 检查项目类型
ls -la  # 查看是否有platformio.ini, package.json等

# 手动配置
./.kiro/serena/setup/setup-project.sh
```

### 知识库内容不完整
```bash
# 重新生成
rm -rf .kiro/serena/knowledge/
./.kiro/serena/setup/setup-project.sh
```

---
*现在你已经准备好使用Serena + Claude的强大组合了！*