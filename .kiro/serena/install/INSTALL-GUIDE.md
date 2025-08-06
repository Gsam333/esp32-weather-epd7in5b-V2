# Serena安装指南

## 🚀 一键安装（推荐）

```bash
# 运行完整安装脚本
./.kiro/serena/install-serena-complete.sh
```

这个脚本会：
- ✅ 自动检测和解决Python版本兼容性问题
- ✅ 清理旧的安装和配置冲突
- ✅ 尝试多种安装方式确保成功
- ✅ 自动配置环境变量和PATH
- ✅ 生成详细的安装报告

## 📋 安装脚本对比

| 脚本文件 | 用途 | 特点 |
|---------|------|------|
| `install-serena-complete.sh` | **完整安装（推荐）** | 基于实际调试经验，解决所有已知问题 |
| `install-serena-global.sh` | 全局安装 | 详细的错误处理和多种安装方式 |
| `install-serena-simple.sh` | 简化安装 | 基础安装流程，适合快速安装 |
| `setup.sh` | 项目配置 | 为当前项目配置Serena环境 |

## 🔧 手动安装步骤

如果自动安装失败，可以按以下步骤手动安装：

### 1. 检查Python版本
```bash
python3 --version
# 需要Python 3.11或3.12
```

### 2. 安装兼容Python版本（如需要）
```bash
brew install python@3.11
export PATH="/opt/homebrew/opt/python@3.11/bin:$PATH"
```

### 3. 安装pipx
```bash
pip3 install pipx
pipx ensurepath
```

### 4. 安装Serena
```bash
pipx install /Users/sanm/Documents/GitHub/serena
```

### 5. 验证安装
```bash
serena --help
```

## 🐛 常见问题解决

### Python版本不兼容
```bash
# 错误：Package 'serena-agent' requires a different Python
# 解决：安装Python 3.11
brew install python@3.11
echo 'export PATH="/opt/homebrew/opt/python@3.11/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### 命令找不到
```bash
# 错误：command not found: serena
# 解决：检查PATH配置
echo $PATH
# 添加必要路径到~/.zshrc
export PATH="$PATH:$HOME/.local/bin"
export PATH="$PATH:$HOME/Library/Python/3.11/bin"
```

### 依赖冲突
```bash
# 错误：dependency conflicts
# 解决：使用虚拟环境或pipx
pipx install /Users/sanm/Documents/GitHub/serena
```

### 权限问题
```bash
# 错误：Permission denied
# 解决：使用用户安装
pip3 install -e /Users/sanm/Documents/GitHub/serena --user
```

## 📊 安装后验证

### 基本验证
```bash
# 检查命令可用性
which serena
serena --help

# 检查Python环境
python3 --version
pip3 --version
```

### 功能验证
```bash
# 测试项目命令
serena project --help
serena config --help

# 测试MCP服务器
serena start-mcp-server --help
```

## 🔄 卸载和重装

### 完全卸载
```bash
# 卸载pipx安装
pipx uninstall serena-agent

# 卸载pip安装
pip3 uninstall serena-agent

# 删除全局脚本
sudo rm -f /usr/local/bin/serena

# 清理配置（可选）
# 编辑 ~/.zshrc 移除相关PATH配置
```

### 重新安装
```bash
# 运行完整安装脚本
./.kiro/serena/install-serena-complete.sh
```

## 📝 安装报告

安装完成后，会在 `.kiro/serena/` 目录下生成详细的安装报告，包含：
- 安装方式和配置信息
- 环境变量设置
- 验证结果
- 故障排除建议

## 💡 最佳实践

1. **使用完整安装脚本**：推荐使用 `install-serena-complete.sh`
2. **备份配置**：安装前会自动备份 `~/.zshrc`
3. **重启终端**：安装后重启终端或运行 `source ~/.zshrc`
4. **定期更新**：定期更新Serena到最新版本

## 🆘 获取帮助

如果遇到问题：
1. 查看安装报告文件
2. 检查 `~/.zshrc.backup_*` 备份文件
3. 运行手动验证命令
4. 参考常见问题解决方案