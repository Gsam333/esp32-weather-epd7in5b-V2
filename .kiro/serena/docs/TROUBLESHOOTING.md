# Serena工具包故障排除指南

## 🔧 常见问题及解决方案

### 1. Serena安装问题

#### 问题：`serena: command not found`

**原因**：Serena未正确安装或不在PATH中

**解决方案**：
```bash
# 检查Serena是否安装
which serena

# 如果未安装，运行安装脚本
./.kiro/serena/install/install-serena-complete.sh

# 检查PATH配置
echo $PATH | grep -o '[^:]*' | grep -E '(local|serena)'
```

#### 问题：Python版本不兼容

**错误信息**：`Package 'serena-agent' requires a different Python`

**解决方案**：
```bash
# 检查Python版本
python3 --version

# 安装兼容的Python版本（3.11或3.12）
brew install python@3.11

# 更新PATH
echo 'export PATH="/opt/homebrew/opt/python@3.11/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

#### 问题：pipx安装失败

**解决方案**：
```bash
# 安装pipx
pip3 install pipx

# 确保pipx路径
pipx ensurepath

# 重新安装Serena
pipx install /path/to/serena/source
```

### 2. 项目配置问题

#### 问题：项目类型识别错误

**症状**：setup脚本识别的项目类型不正确

**解决方案**：
```bash
# 手动编辑项目配置
vim .kiro/serena/config/project.yml

# 或重新运行配置脚本
./.kiro/serena/setup/setup-project.sh
```

#### 问题：知识库生成不完整

**症状**：生成的知识库缺少重要信息

**解决方案**：
```bash
# 删除现有知识库
rm -rf .kiro/serena/knowledge/

# 重新生成
./.kiro/serena/setup/setup-project.sh

# 手动补充信息
vim .kiro/serena/knowledge/project-knowledge-base.md
```

#### 问题：目录权限问题

**错误信息**：`Permission denied`

**解决方案**：
```bash
# 检查目录权限
ls -la .kiro/

# 修复权限
chmod -R 755 .kiro/serena/
chmod +x .kiro/serena/scripts/*.sh
```

### 3. Serena命令问题

#### 问题：`serena project index` 失败

**错误信息**：编码错误或文件读取失败

**解决方案**：
```bash
# 检查项目配置
cat .serena/project.yml

# 清理缓存
rm -rf .serena/cache/

# 重新索引
serena project index

# 如果仍然失败，跳过有问题的文件
# 编辑 .gitignore 排除二进制文件
echo "*.bin" >> .gitignore
echo "*.o" >> .gitignore
```

#### 问题：`serena project generate-yml` 报错文件已存在

**解决方案**：
```bash
# 删除现有配置
rm .serena/project.yml

# 重新生成
serena project generate-yml

# 或者强制覆盖（如果支持）
serena project generate-yml --force
```

### 4. 脚本执行问题

#### 问题：脚本没有执行权限

**错误信息**：`Permission denied`

**解决方案**：
```bash
# 添加执行权限
chmod +x .kiro/serena/scripts/*.sh
chmod +x .kiro/serena/setup/*.sh
chmod +x .kiro/serena/install/*.sh
```

#### 问题：脚本找不到命令

**错误信息**：`command not found: jq` 或类似

**解决方案**：
```bash
# 安装缺失的工具
brew install jq  # JSON处理工具
brew install ripgrep  # 文本搜索工具

# 或者修改脚本，添加工具检查
if ! command -v jq &> /dev/null; then
    echo "jq not found, skipping JSON parsing"
fi
```

### 5. 知识库问题

#### 问题：知识库内容过时

**症状**：知识库信息与当前代码不匹配

**解决方案**：
```bash
# 更新知识库
./.kiro/serena/scripts/update-knowledge.sh

# 重新分析代码
./.kiro/serena/scripts/analyze-code.sh

# 手动更新重要信息
vim .kiro/serena/knowledge/project-knowledge-base.md
```

#### 问题：代码统计不准确

**解决方案**：
```bash
# 检查统计脚本
cat .kiro/serena/scripts/analyze-code.sh

# 手动验证统计
find . -name "*.cpp" | wc -l
find . -name "*.py" | wc -l

# 修改脚本以适应项目特点
vim .kiro/serena/scripts/analyze-code.sh
```

### 6. Claude协作问题

#### 问题：Claude给出的建议不符合项目特点

**原因**：提供的上下文信息不够完整或准确

**解决方案**：
```bash
# 确保知识库是最新的
./.kiro/serena/scripts/update-knowledge.sh

# 检查知识库内容
cat .kiro/serena/knowledge/project-knowledge-base.md

# 在与Claude对话时提供更多具体信息
# 包括：项目约束、技术选型理由、现有代码风格等
```

#### 问题：生成的代码风格不一致

**解决方案**：
1. 在知识库中明确记录代码规范
2. 向Claude提供现有代码示例
3. 明确说明项目的编码标准

### 7. 性能问题

#### 问题：代码分析速度慢

**解决方案**：
```bash
# 优化分析脚本，排除不必要的文件
vim .kiro/serena/scripts/analyze-code.sh

# 添加更多排除模式
echo "node_modules/" >> .gitignore
echo "build/" >> .gitignore
echo ".pio/" >> .gitignore
```

#### 问题：Serena索引占用空间大

**解决方案**：
```bash
# 清理缓存
rm -rf .serena/cache/

# 检查缓存大小
du -sh .serena/

# 定期清理（可添加到脚本中）
find .serena/cache -mtime +30 -delete
```

### 8. 环境兼容性问题

#### 问题：在不同操作系统上脚本失败

**解决方案**：
```bash
# 检查系统类型
uname -s

# 修改脚本以支持多系统
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS specific commands
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux specific commands
fi
```

#### 问题：Shell兼容性问题

**解决方案**：
```bash
# 确保使用bash
#!/bin/bash

# 或者使用更兼容的语法
# 避免使用bash特有的功能
```

## 🔍 诊断工具

### 环境检查脚本

创建一个诊断脚本：

```bash
cat > .kiro/serena/scripts/diagnose.sh << 'EOF'
#!/bin/bash
echo "🔍 Serena环境诊断"
echo ""

echo "📋 基础环境:"
echo "- 操作系统: $(uname -s)"
echo "- Shell: $SHELL"
echo "- Python: $(python3 --version 2>/dev/null || echo '未安装')"
echo "- Serena: $(serena --help >/dev/null 2>&1 && echo '已安装' || echo '未安装')"

echo ""
echo "📁 目录结构:"
ls -la .kiro/serena/ 2>/dev/null || echo "Serena目录不存在"

echo ""
echo "⚙️ 配置文件:"
[ -f ".kiro/serena/config/project.yml" ] && echo "✅ 项目配置存在" || echo "❌ 项目配置缺失"
[ -f ".serena/project.yml" ] && echo "✅ Serena配置存在" || echo "❌ Serena配置缺失"

echo ""
echo "📚 知识库:"
[ -f ".kiro/serena/knowledge/project-knowledge-base.md" ] && echo "✅ 知识库存在" || echo "❌ 知识库缺失"

echo ""
echo "🔧 脚本权限:"
for script in .kiro/serena/scripts/*.sh; do
    if [ -x "$script" ]; then
        echo "✅ $(basename $script)"
    else
        echo "❌ $(basename $script) (无执行权限)"
    fi
done
EOF

chmod +x .kiro/serena/scripts/diagnose.sh
```

### 使用诊断脚本

```bash
# 运行诊断
./.kiro/serena/scripts/diagnose.sh

# 根据诊断结果修复问题
```

## 📞 获取帮助

### 自助解决

1. **查看日志**：检查 `.kiro/serena/analysis/` 目录下的日志文件
2. **重新配置**：删除配置后重新运行 `setup-project.sh`
3. **清理重建**：删除整个 `.kiro/serena` 目录后重新配置

### 常用修复命令

```bash
# 完全重置Serena配置
rm -rf .kiro/serena .serena
cp -r /path/to/serena-toolkit .kiro/serena
./.kiro/serena/setup/setup-project.sh

# 修复权限问题
find .kiro/serena -name "*.sh" -exec chmod +x {} \;

# 重新生成知识库
rm .kiro/serena/knowledge/project-knowledge-base.md
./.kiro/serena/scripts/update-knowledge.sh
```

---
*遇到问题不要慌，按照这个指南一步步排查，大部分问题都能解决！*