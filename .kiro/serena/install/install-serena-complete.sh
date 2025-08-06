#!/bin/bash
# Serena完整安装脚本 - 基于实际调试经验
# 解决Python版本兼容性、环境管理、依赖冲突等问题

set -e

echo "🚀 Serena完整安装脚本"
echo "基于实际调试经验，解决所有已知问题"
echo ""

# 配置变量
SERENA_PATH="/Users/sanm/Documents/GitHub/serena"
REQUIRED_PYTHON_VERSION="3.11"
BACKUP_SUFFIX=$(date +%Y%m%d_%H%M%S)

# 颜色输出函数
print_step() { echo "🔧 $1"; }
print_success() { echo "✅ $1"; }
print_warning() { echo "⚠️  $1"; }
print_error() { echo "❌ $1"; }
print_info() { echo "💡 $1"; }

# 检查Serena源码
check_serena_source() {
    print_step "检查Serena源码..."
    if [ ! -d "$SERENA_PATH" ]; then
        print_error "未找到Serena源码目录: $SERENA_PATH"
        print_info "请确保Serena源码已下载到正确位置"
        exit 1
    fi
    print_success "找到Serena源码: $SERENA_PATH"
}

# 检查当前安装状态
check_current_installation() {
    print_step "检查当前安装状态..."
    if command -v serena &> /dev/null; then
        print_warning "检测到已安装的Serena"
        echo "当前版本: $(serena --help | head -1 2>/dev/null || echo '无法获取版本信息')"
        echo "安装位置: $(which serena)"
        echo ""
        read -p "是否要重新安装？(y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "取消安装"
            exit 0
        fi
    fi
}

# 备份当前配置
backup_config() {
    print_step "备份当前配置..."
    if [ -f ~/.zshrc ]; then
        cp ~/.zshrc ~/.zshrc.backup_$BACKUP_SUFFIX
        print_success "已备份 ~/.zshrc 到 ~/.zshrc.backup_$BACKUP_SUFFIX"
    fi
}

# 检查和清理Python环境
check_python_environment() {
    print_step "检查Python环境..."
    
    echo "当前Python版本: $(python3 --version)"
    echo "Python位置: $(which python3)"
    
    # 检查Python版本兼容性
    CURRENT_PYTHON_VERSION=$(python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
    
    if [[ "$CURRENT_PYTHON_VERSION" != "3.11" && "$CURRENT_PYTHON_VERSION" != "3.12" ]]; then
        print_warning "当前Python版本 $CURRENT_PYTHON_VERSION 可能不兼容Serena"
        print_info "Serena要求Python 3.11或3.12"
        
        read -p "是否要安装Python 3.11？(y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            install_compatible_python
        else
            print_warning "继续使用当前Python版本，可能会遇到兼容性问题"
        fi
    else
        print_success "Python版本兼容: $CURRENT_PYTHON_VERSION"
    fi
}

# 安装兼容的Python版本
install_compatible_python() {
    print_step "安装Python 3.11..."
    
    # 检查是否已安装
    if brew list python@3.11 &> /dev/null; then
        print_info "Python 3.11已安装，配置PATH优先级..."
    else
        print_step "通过Homebrew安装Python 3.11..."
        brew install python@3.11
    fi
    
    # 配置PATH优先级
    print_step "配置Python 3.11为默认版本..."
    
    # 清理现有Python PATH配置
    if [ -f ~/.zshrc ]; then
        # 移除旧的Python PATH配置
        sed -i '' '/python@/d' ~/.zshrc
        sed -i '' '/Python.*bin/d' ~/.zshrc
    fi
    
    # 添加Python 3.11到PATH最前面
    echo '# Python 3.11 优先配置' >> ~/.zshrc
    echo 'export PATH="/opt/homebrew/opt/python@3.11/bin:$PATH"' >> ~/.zshrc
    echo 'export PATH="/opt/homebrew/bin:$PATH"' >> ~/.zshrc
    
    # 立即生效
    export PATH="/opt/homebrew/opt/python@3.11/bin:$PATH"
    export PATH="/opt/homebrew/bin:$PATH"
    
    # 创建符号链接确保python3指向正确版本
    if [ -f /opt/homebrew/opt/python@3.11/bin/python3.11 ]; then
        ln -sf /opt/homebrew/opt/python@3.11/bin/python3.11 /opt/homebrew/bin/python3
        ln -sf /opt/homebrew/opt/python@3.11/bin/pip3.11 /opt/homebrew/bin/pip3
    fi
    
    print_success "Python 3.11配置完成"
    echo "新的Python版本: $(python3 --version)"
}

# 安装和配置pipx
setup_pipx() {
    print_step "设置pipx..."
    
    if ! command -v pipx &> /dev/null; then
        print_step "安装pipx..."
        pip3 install pipx
        pipx ensurepath
    else
        print_success "pipx已安装"
    fi
    
    # 确保pipx路径在PATH中
    if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
        echo 'export PATH="$PATH:$HOME/.local/bin"' >> ~/.zshrc
        export PATH="$PATH:$HOME/.local/bin"
    fi
    
    print_success "pipx配置完成"
}

# 清理旧的Serena安装
cleanup_old_installation() {
    print_step "清理旧的Serena安装..."
    
    # 卸载pipx安装的serena
    if pipx list | grep -q serena-agent; then
        print_step "卸载pipx安装的serena..."
        pipx uninstall serena-agent || true
    fi
    
    # 卸载pip安装的serena
    if pip3 list | grep -q serena-agent; then
        print_step "卸载pip安装的serena..."
        pip3 uninstall serena-agent -y || true
    fi
    
    # 删除全局脚本
    if [ -f /usr/local/bin/serena ]; then
        print_step "删除全局脚本..."
        sudo rm -f /usr/local/bin/serena || true
    fi
    
    print_success "旧安装清理完成"
}

# 使用pipx安装Serena（推荐方式）
install_with_pipx() {
    print_step "使用pipx安装Serena（推荐方式）..."
    
    if pipx install "$SERENA_PATH"; then
        print_success "pipx安装成功"
        
        # 测试安装
        if command -v serena &> /dev/null; then
            print_success "Serena命令可用"
            serena --help | head -5
            return 0
        else
            print_warning "pipx安装成功但命令不可用，检查PATH..."
            return 1
        fi
    else
        print_warning "pipx安装失败"
        return 1
    fi
}

# 使用pip用户安装
install_with_pip_user() {
    print_step "使用pip用户目录安装..."
    
    if pip3 install -e "$SERENA_PATH" --user; then
        print_success "用户目录安装成功"
        
        # 确保用户Python bin目录在PATH中
        USER_PYTHON_BIN="$HOME/Library/Python/$(python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')/bin"
        if [[ ":$PATH:" != *":$USER_PYTHON_BIN:"* ]]; then
            echo "export PATH=\"\$PATH:$USER_PYTHON_BIN\"" >> ~/.zshrc
            export PATH="$PATH:$USER_PYTHON_BIN"
            print_success "已添加用户Python bin目录到PATH"
        fi
        
        # 测试安装
        if command -v serena &> /dev/null; then
            print_success "Serena命令可用"
            return 0
        else
            print_warning "安装成功但命令不可用"
            return 1
        fi
    else
        print_warning "用户目录安装失败"
        return 1
    fi
}

# 使用系统安装（强制）
install_with_pip_system() {
    print_step "使用系统pip安装（强制模式）..."
    
    if pip3 install -e "$SERENA_PATH" --break-system-packages; then
        print_success "系统安装成功"
        
        # 测试安装
        if command -v serena &> /dev/null; then
            print_success "Serena命令可用"
            return 0
        else
            print_warning "安装成功但命令不可用"
            return 1
        fi
    else
        print_warning "系统安装失败"
        return 1
    fi
}

# 创建全局脚本（最后手段）
create_global_script() {
    print_step "创建全局脚本..."
    
    cat > /tmp/serena_wrapper << EOF
#!/bin/bash
# Serena全局包装脚本
export PYTHONPATH="$SERENA_PATH/src:\$PYTHONPATH"
exec python3 -m serena.cli "\$@"
EOF
    
    if sudo cp /tmp/serena_wrapper /usr/local/bin/serena && sudo chmod +x /usr/local/bin/serena; then
        print_success "全局脚本创建成功"
        
        # 测试脚本
        if serena --help &> /dev/null; then
            print_success "全局脚本工作正常"
            return 0
        else
            print_warning "全局脚本创建成功但无法正常工作"
            return 1
        fi
    else
        print_error "全局脚本创建失败"
        return 1
    fi
}

# 验证安装
verify_installation() {
    print_step "验证安装..."
    
    if command -v serena &> /dev/null; then
        print_success "Serena安装成功！"
        echo ""
        echo "📍 安装位置: $(which serena)"
        echo "🐍 Python版本: $(python3 --version)"
        echo "📋 Serena帮助:"
        serena --help | head -10
        echo ""
        print_success "安装验证通过"
        return 0
    else
        print_error "安装验证失败"
        return 1
    fi
}

# 生成安装报告
generate_report() {
    print_step "生成安装报告..."
    
    REPORT_FILE=".kiro/serena/install-report-$BACKUP_SUFFIX.md"
    mkdir -p .kiro/serena
    
    cat > "$REPORT_FILE" << EOF
# Serena安装报告

## 安装信息
- **安装时间**: $(date)
- **Python版本**: $(python3 --version)
- **Python位置**: $(which python3)
- **Serena位置**: $(which serena 2>/dev/null || echo "未找到")
- **安装方式**: $INSTALL_METHOD

## 环境配置
- **PATH配置**: 已更新
- **配置备份**: ~/.zshrc.backup_$BACKUP_SUFFIX

## 验证结果
$(if command -v serena &> /dev/null; then
    echo "✅ 安装成功"
    echo ""
    echo "### Serena命令测试"
    echo "\`\`\`"
    serena --help | head -10
    echo "\`\`\`"
else
    echo "❌ 安装失败"
fi)

## 使用说明
\`\`\`bash
# 查看帮助
serena --help

# 项目相关命令
serena project --help
serena config --help
\`\`\`

## 故障排除
如果遇到问题，请检查：
1. Python版本是否为3.11或3.12
2. PATH配置是否正确
3. 依赖包是否完整安装

## 配置文件位置
- 主配置: ~/.zshrc
- 备份配置: ~/.zshrc.backup_$BACKUP_SUFFIX
EOF
    
    print_success "安装报告已生成: $REPORT_FILE"
}

# 主安装流程
main() {
    echo "开始Serena完整安装流程..."
    echo ""
    
    # 检查前置条件
    check_serena_source
    check_current_installation
    backup_config
    
    # 环境准备
    check_python_environment
    setup_pipx
    cleanup_old_installation
    
    # 尝试不同安装方式
    INSTALL_SUCCESS=false
    INSTALL_METHOD=""
    
    # 方式1: pipx安装（推荐）
    if install_with_pipx; then
        INSTALL_SUCCESS=true
        INSTALL_METHOD="pipx"
    # 方式2: pip用户安装
    elif install_with_pip_user; then
        INSTALL_SUCCESS=true
        INSTALL_METHOD="pip --user"
    # 方式3: pip系统安装
    elif install_with_pip_system; then
        INSTALL_SUCCESS=true
        INSTALL_METHOD="pip --break-system-packages"
    # 方式4: 全局脚本
    elif create_global_script; then
        INSTALL_SUCCESS=true
        INSTALL_METHOD="global script"
    fi
    
    # 验证和报告
    if [ "$INSTALL_SUCCESS" = true ]; then
        if verify_installation; then
            generate_report
            
            echo ""
            echo "🎉 Serena安装完成！"
            echo ""
            echo "📋 安装摘要:"
            echo "   方式: $INSTALL_METHOD"
            echo "   位置: $(which serena)"
            echo "   Python: $(python3 --version)"
            echo ""
            echo "🔄 重新加载shell配置:"
            echo "   source ~/.zshrc"
            echo ""
            echo "🚀 开始使用:"
            echo "   serena --help"
            echo "   serena project --help"
            echo ""
            print_success "安装流程完成！"
        else
            print_error "安装验证失败"
            exit 1
        fi
    else
        print_error "所有安装方式都失败了"
        echo ""
        echo "🔧 手动解决方案："
        echo "1. 检查Python环境: python3 --version"
        echo "2. 检查pip: pip3 --version"
        echo "3. 手动测试: PYTHONPATH=$SERENA_PATH/src python3 -m serena.cli --help"
        echo "4. 检查依赖: cd $SERENA_PATH && python3 -c 'import serena'"
        echo ""
        echo "💡 如果手动运行成功，可以创建alias:"
        echo "echo 'alias serena=\"PYTHONPATH=$SERENA_PATH/src python3 -m serena.cli\"' >> ~/.zshrc"
        exit 1
    fi
}

# 执行主流程
main "$@"