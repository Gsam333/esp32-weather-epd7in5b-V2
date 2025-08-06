#!/bin/bash
# 通用项目Serena配置脚本
# 适用于各种类型的项目

set -e

echo "🚀 Serena项目配置 - 通用版本"
echo "自动识别项目类型并生成知识库"
echo ""

# 获取项目信息
PROJECT_NAME=$(basename "$(pwd)")
PROJECT_PATH=$(pwd)
CURRENT_DATE=$(date -Iseconds)

# 颜色输出函数
print_step() { echo "🔧 $1"; }
print_success() { echo "✅ $1"; }
print_warning() { echo "⚠️  $1"; }
print_error() { echo "❌ $1"; }
print_info() { echo "💡 $1"; }

# 智能检测项目类型
detect_project_type() {
    print_step "智能检测项目类型..."
    
    PROJECT_TYPE="unknown"
    MAIN_LANGUAGE="unknown"
    PLATFORM="unknown"
    FRAMEWORK="unknown"
    
    # ESP32/Arduino项目
    if [ -f "platformio.ini" ]; then
        PROJECT_TYPE="embedded"
        MAIN_LANGUAGE="cpp"
        FRAMEWORK="arduino"
        if grep -q "esp32" platformio.ini; then
            PLATFORM="esp32"
        elif grep -q "arduino" platformio.ini; then
            PLATFORM="arduino"
        else
            PLATFORM="embedded"
        fi
        print_success "检测到嵌入式项目: $PLATFORM"
        
    # Node.js项目
    elif [ -f "package.json" ]; then
        PROJECT_TYPE="web"
        MAIN_LANGUAGE="javascript"
        PLATFORM="nodejs"
        FRAMEWORK="nodejs"
        if grep -q "react" package.json; then
            FRAMEWORK="react"
        elif grep -q "vue" package.json; then
            FRAMEWORK="vue"
        elif grep -q "express" package.json; then
            FRAMEWORK="express"
        fi
        print_success "检测到Node.js项目: $FRAMEWORK"
        
    # Python项目
    elif [ -f "requirements.txt" ] || [ -f "pyproject.toml" ] || [ -f "setup.py" ]; then
        PROJECT_TYPE="application"
        MAIN_LANGUAGE="python"
        PLATFORM="python"
        if [ -f "pyproject.toml" ]; then
            FRAMEWORK="modern-python"
        elif [ -f "requirements.txt" ]; then
            FRAMEWORK="pip"
        else
            FRAMEWORK="setuptools"
        fi
        print_success "检测到Python项目: $FRAMEWORK"
        
    # Rust项目
    elif [ -f "Cargo.toml" ]; then
        PROJECT_TYPE="system"
        MAIN_LANGUAGE="rust"
        PLATFORM="rust"
        FRAMEWORK="cargo"
        print_success "检测到Rust项目"
        
    # Go项目
    elif [ -f "go.mod" ]; then
        PROJECT_TYPE="system"
        MAIN_LANGUAGE="go"
        PLATFORM="go"
        FRAMEWORK="go-modules"
        print_success "检测到Go项目"
        
    # C/C++项目
    elif [ -f "Makefile" ] || [ -f "CMakeLists.txt" ]; then
        PROJECT_TYPE="system"
        MAIN_LANGUAGE="cpp"
        PLATFORM="native"
        if [ -f "CMakeLists.txt" ]; then
            FRAMEWORK="cmake"
        else
            FRAMEWORK="make"
        fi
        print_success "检测到C/C++项目: $FRAMEWORK"
        
    # Java项目
    elif [ -f "pom.xml" ] || [ -f "build.gradle" ]; then
        PROJECT_TYPE="application"
        MAIN_LANGUAGE="java"
        PLATFORM="jvm"
        if [ -f "pom.xml" ]; then
            FRAMEWORK="maven"
        else
            FRAMEWORK="gradle"
        fi
        print_success "检测到Java项目: $FRAMEWORK"
        
    else
        print_warning "未识别项目类型，使用通用配置"
        PROJECT_TYPE="general"
        MAIN_LANGUAGE="mixed"
        PLATFORM="general"
        FRAMEWORK="unknown"
    fi
    
    echo "📊 项目信息:"
    echo "   名称: $PROJECT_NAME"
    echo "   类型: $PROJECT_TYPE"
    echo "   语言: $MAIN_LANGUAGE"
    echo "   平台: $PLATFORM"
    echo "   框架: $FRAMEWORK"
}

# 检查Serena可用性
check_serena() {
    print_step "检查Serena可用性..."
    
    if command -v serena &> /dev/null; then
        print_success "Serena可用: $(which serena)"
        return 0
    else
        print_error "Serena不可用"
        print_info "请先运行安装脚本: ./.kiro/serena/install/install-serena-complete.sh"
        return 1
    fi
}

# 创建目录结构
create_directories() {
    print_step "创建Serena目录结构..."
    
    mkdir -p .kiro/serena/{config,knowledge,analysis,templates,scripts,reports,cache}
    mkdir -p .kiro/serena/knowledge/{architecture,components,apis,troubleshooting}
    mkdir -p .kiro/serena/analysis/{code,dependencies,performance,security}
    mkdir -p .kiro/serena/templates/{code,docs,tests}
    
    print_success "目录结构创建完成"
}

# 生成项目配置
generate_config() {
    print_step "生成项目配置文件..."
    
    cat > .kiro/serena/config/project.yml << EOF
# $PROJECT_NAME - Serena项目配置
project:
  name: "$PROJECT_NAME"
  type: "$PROJECT_TYPE"
  language: "$MAIN_LANGUAGE"
  platform: "$PLATFORM"
  framework: "$FRAMEWORK"
  created: "$CURRENT_DATE"
  root_path: "$PROJECT_PATH"
  
  description: "基于$FRAMEWORK的$PROJECT_TYPE项目"

# 代码分析配置
analysis:
  focus_files:
    - "src/**/*"
    - "lib/**/*"
    - "include/**/*"
    - "*.py"
    - "*.js"
    - "*.ts"
    - "*.rs"
    - "*.go"
    - "*.java"
    - "*.cpp"
    - "*.h"
  
  exclude_patterns:
    - "build/**"
    - "dist/**"
    - "node_modules/**"
    - "target/**"
    - ".git/**"
    - ".pio/**"
    - ".vscode/**"
    - "*.tmp"
    - "*.bak"

# 知识库配置
knowledge_base:
  path: ".kiro/serena/knowledge/"
  auto_update: true
  version_control: true
  
  categories:
    - "architecture"
    - "components"
    - "apis"
    - "troubleshooting"
    - "best_practices"

# 输出配置
output:
  analysis_dir: ".kiro/serena/analysis/"
  templates_dir: ".kiro/serena/templates/"
  reports_dir: ".kiro/serena/reports/"
  cache_dir: ".kiro/serena/cache/"
EOF

    print_success "项目配置文件生成完成"
}

# 生成知识库
generate_knowledge_base() {
    print_step "生成项目知识库..."
    
    # 获取项目统计信息
    local file_count=0
    local code_lines=0
    
    case $MAIN_LANGUAGE in
        "cpp")
            file_count=$(find . -name "*.cpp" -o -name "*.h" -o -name "*.c" | wc -l)
            code_lines=$(find . -name "*.cpp" -o -name "*.h" -o -name "*.c" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")
            ;;
        "javascript")
            file_count=$(find . -name "*.js" -o -name "*.ts" -o -name "*.jsx" -o -name "*.tsx" | wc -l)
            code_lines=$(find . -name "*.js" -o -name "*.ts" -o -name "*.jsx" -o -name "*.tsx" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")
            ;;
        "python")
            file_count=$(find . -name "*.py" | wc -l)
            code_lines=$(find . -name "*.py" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")
            ;;
        "rust")
            file_count=$(find . -name "*.rs" | wc -l)
            code_lines=$(find . -name "*.rs" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")
            ;;
        "go")
            file_count=$(find . -name "*.go" | wc -l)
            code_lines=$(find . -name "*.go" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")
            ;;
        *)
            file_count=$(find . -type f -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.py" -o -name "*.js" -o -name "*.rs" -o -name "*.go" | wc -l)
            code_lines=$(find . -type f -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.py" -o -name "*.js" -o -name "*.rs" -o -name "*.go" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")
            ;;
    esac
    
    # 主知识库文件
    cat > .kiro/serena/knowledge/project-knowledge-base.md << EOF
# $PROJECT_NAME 项目知识库

## 📊 项目概览

### 基本信息
- **项目名称**: $PROJECT_NAME
- **项目类型**: $PROJECT_TYPE
- **主要语言**: $MAIN_LANGUAGE
- **开发框架**: $FRAMEWORK
- **目标平台**: $PLATFORM
- **创建时间**: $(date)
- **项目路径**: $PROJECT_PATH

### 项目描述
基于$FRAMEWORK的$PROJECT_TYPE项目，使用$MAIN_LANGUAGE开发。

### 核心特性
EOF

# 根据项目类型添加特性
case $PROJECT_TYPE in
    "embedded")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- 🔌 嵌入式系统开发
- ⚡ 低功耗设计
- 🔧 硬件接口控制
- 📡 传感器数据采集
EOF
        ;;
    "web")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- 🌐 Web应用开发
- � 响 应式设计
- � 异步数据处h理
- �  用户认证和授权
EOF
        ;;
    "application")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- 💻 桌面/服务器应用
- � 数据 处理和分析
- �  API接口开发
- 📦 模块化架构
EOF
        ;;
    "system")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- ⚙️ 系统级编程
- 🚀 高性能计算
- 🔒 内存安全
- 🛠️ 底层系统接口
EOF
        ;;
    *)
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- 🔧 通用软件开发
- 📚 模块化设计
- 🔄 持续集成
- 📝 文档驱动开发
EOF
        ;;
esac

cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

## 🏗️ 项目架构

### 目录结构
\`\`\`
$(find . -type d -name ".*" -prune -o -type d -print | head -15 | sed 's|^\./||' | sort)
\`\`\`

### 核心文件
$(case $MAIN_LANGUAGE in
    "cpp")
        find . -name "*.cpp" -o -name "*.h" | head -10 | sed 's|^|- |'
        ;;
    "javascript")
        find . -name "*.js" -o -name "*.ts" | head -10 | sed 's|^|- |'
        ;;
    "python")
        find . -name "*.py" | head -10 | sed 's|^|- |'
        ;;
    *)
        find . -type f -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.py" -o -name "*.js" -o -name "*.rs" -o -name "*.go" | head -10 | sed 's|^|- |'
        ;;
esac)

### 配置文件
$(case $FRAMEWORK in
    "arduino")
        echo "- platformio.ini: PlatformIO项目配置"
        ;;
    "nodejs"|"react"|"vue")
        echo "- package.json: Node.js项目配置"
        [ -f "tsconfig.json" ] && echo "- tsconfig.json: TypeScript配置"
        ;;
    "pip"|"modern-python")
        [ -f "requirements.txt" ] && echo "- requirements.txt: Python依赖"
        [ -f "pyproject.toml" ] && echo "- pyproject.toml: 现代Python配置"
        ;;
    "cargo")
        echo "- Cargo.toml: Rust项目配置"
        ;;
    "go-modules")
        echo "- go.mod: Go模块配置"
        ;;
    "cmake")
        echo "- CMakeLists.txt: CMake构建配置"
        ;;
    "maven")
        echo "- pom.xml: Maven项目配置"
        ;;
    "gradle")
        echo "- build.gradle: Gradle构建配置"
        ;;
esac)

## 🔧 技术栈详解

### 开发环境
- **项目类型**: $PROJECT_TYPE
- **编程语言**: $MAIN_LANGUAGE
- **开发框架**: $FRAMEWORK
- **目标平台**: $PLATFORM

### 项目统计
- **代码文件数**: $file_count 个
- **总代码行数**: $code_lines 行
- **最近修改**: $(find . -name "*.$MAIN_LANGUAGE" -exec stat -f "%Sm %N" -t "%Y-%m-%d" {} \; 2>/dev/null | sort -r | head -1 || echo "无法获取")

## 📱 功能模块分析

### 主要模块
$(case $PROJECT_TYPE in
    "embedded")
        echo "1. **硬件抽象层**: GPIO、SPI、I2C等硬件接口"
        echo "2. **传感器模块**: 数据采集和处理"
        echo "3. **通信模块**: WiFi、蓝牙等无线通信"
        echo "4. **显示模块**: 屏幕显示和用户界面"
        echo "5. **电源管理**: 低功耗控制和电池管理"
        ;;
    "web")
        echo "1. **前端界面**: 用户界面和交互逻辑"
        echo "2. **路由管理**: 页面导航和状态管理"
        echo "3. **数据层**: API调用和数据处理"
        echo "4. **组件库**: 可复用的UI组件"
        echo "5. **工具函数**: 通用工具和辅助函数"
        ;;
    "application")
        echo "1. **核心逻辑**: 主要业务逻辑实现"
        echo "2. **数据处理**: 数据输入输出和转换"
        echo "3. **配置管理**: 应用配置和参数管理"
        echo "4. **错误处理**: 异常捕获和错误恢复"
        echo "5. **日志系统**: 运行日志和调试信息"
        ;;
    *)
        echo "1. **核心模块**: 主要功能实现"
        echo "2. **工具模块**: 辅助工具和函数"
        echo "3. **配置模块**: 配置管理和参数设置"
        echo "4. **测试模块**: 单元测试和集成测试"
        echo "5. **文档模块**: 项目文档和说明"
        ;;
esac)

## 🐛 常见问题和解决方案

### 编译/构建问题
$(case $FRAMEWORK in
    "arduino")
        echo "- **依赖库问题**: 检查platformio.ini中的lib_deps配置"
        echo "- **编译错误**: 确保使用正确的C++标准和编译选项"
        ;;
    "nodejs")
        echo "- **依赖安装**: 运行npm install或yarn install"
        echo "- **版本冲突**: 检查package.json中的依赖版本"
        ;;
    "pip")
        echo "- **依赖安装**: 运行pip install -r requirements.txt"
        echo "- **Python版本**: 确保使用正确的Python版本"
        ;;
    *)
        echo "- **依赖问题**: 检查项目依赖配置"
        echo "- **环境问题**: 确保开发环境配置正确"
        ;;
esac)

### 运行时问题
- **配置错误**: 检查配置文件格式和参数
- **权限问题**: 确保有足够的文件和网络权限
- **资源不足**: 检查内存和存储空间使用情况

## 📝 开发规范

### 代码风格
- **命名规范**: 使用清晰、描述性的变量和函数名
- **注释要求**: 关键函数和复杂逻辑必须有注释
- **错误处理**: 完善的错误检查和异常处理

### 最佳实践
- **模块化设计**: 保持代码模块化和可复用性
- **版本控制**: 使用Git进行版本管理
- **测试驱动**: 编写单元测试和集成测试
- **文档更新**: 及时更新项目文档

## 🎯 扩展开发指导

### 添加新功能的步骤
1. **需求分析**: 明确功能需求和技术方案
2. **架构设计**: 确定模块结构和接口设计
3. **代码实现**: 编写核心功能代码
4. **测试验证**: 单元测试和集成测试
5. **文档更新**: 更新相关文档和注释

### 常见扩展方向
$(case $PROJECT_TYPE in
    "embedded")
        echo "- **新传感器支持**: 添加其他传感器模块"
        echo "- **通信协议**: 支持新的通信方式"
        echo "- **用户界面**: 改进显示和交互体验"
        echo "- **功耗优化**: 进一步降低功耗"
        ;;
    "web")
        echo "- **新页面功能**: 添加新的页面和功能"
        echo "- **性能优化**: 提升加载速度和响应性能"
        echo "- **用户体验**: 改进界面设计和交互"
        echo "- **移动适配**: 优化移动端体验"
        ;;
    *)
        echo "- **功能扩展**: 添加新的核心功能"
        echo "- **性能优化**: 提升运行效率"
        echo "- **用户体验**: 改进用户界面和交互"
        echo "- **平台支持**: 支持更多平台和环境"
        ;;
esac)

## 🔄 知识库维护

### 更新记录
- **v1.0** ($(date +%Y-%m-%d)): 自动生成初始项目知识库
- 基础项目信息和架构分析
- 核心模块功能说明
- 开发规范和最佳实践

### 待完善内容
- [ ] 详细的API接口文档
- [ ] 架构设计图和流程图
- [ ] 性能优化指南
- [ ] 故障排除手册
- [ ] 部署和运维指南

---
*此知识库由Serena自动生成，随项目发展持续更新*
EOF

    print_success "项目知识库生成完成"
}

# 创建辅助脚本
create_scripts() {
    print_step "创建辅助脚本..."
    
    # 代码分析脚本
    cat > .kiro/serena/scripts/analyze-code.sh << 'EOF'
#!/bin/bash
echo "🔍 开始项目代码分析..."

# 获取项目根目录
cd "$(dirname "$0")/../../.."

# 创建分析结果目录
mkdir -p .kiro/serena/analysis

# 基础统计
echo "📊 项目统计分析..."
{
    echo "# 项目代码分析报告"
    echo ""
    echo "## 基础统计"
    echo "- 分析时间: $(date)"
    echo "- 项目路径: $(pwd)"
    echo ""
    
    # 文件统计
    echo "## 文件统计"
    for ext in cpp h py js ts rs go java c; do
        count=$(find . -name "*.$ext" | wc -l)
        if [ $count -gt 0 ]; then
            lines=$(find . -name "*.$ext" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")
            echo "- .$ext 文件: $count 个，$lines 行"
        fi
    done
    
    echo ""
    echo "## 目录结构"
    find . -type d -name ".*" -prune -o -type d -print | head -20 | sed 's|^\./|- |' | sort
    
} > .kiro/serena/analysis/code-analysis.md

echo "✅ 代码分析完成，结果保存到 .kiro/serena/analysis/code-analysis.md"
EOF

    # 知识库更新脚本
    cat > .kiro/serena/scripts/update-knowledge.sh << 'EOF'
#!/bin/bash
echo "🔄 更新项目知识库..."

cd "$(dirname "$0")/../../.."

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

echo "✅ 知识库更新完成"
EOF

    # Serena初始化脚本
    cat > .kiro/serena/scripts/init-serena.sh << 'EOF'
#!/bin/bash
echo "🚀 初始化Serena项目..."

cd "$(dirname "$0")/../../.."

# 检查Serena命令
if command -v serena &> /dev/null; then
    echo "✅ Serena可用: $(which serena)"
    
    # 生成项目配置
    if [ ! -f ".serena/project.yml" ]; then
        echo "📝 生成Serena项目配置..."
        serena project generate-yml
    fi
    
    # 索引项目
    echo "🔍 索引项目代码..."
    serena project index
    
    echo "✅ Serena项目初始化完成"
else
    echo "❌ Serena不可用，请先安装Serena"
    exit 1
fi
EOF

    # 设置执行权限
    chmod +x .kiro/serena/scripts/*.sh
    
    print_success "辅助脚本创建完成"
}

# 生成使用指南
generate_usage_guide() {
    print_step "生成使用指南..."
    
    cat > .kiro/serena/README.md << EOF
# $PROJECT_NAME - Serena集成使用指南

## 🎯 快速开始

### 查看项目知识库
\`\`\`bash
cat .kiro/serena/knowledge/project-knowledge-base.md
\`\`\`

### 执行代码分析
\`\`\`bash
./.kiro/serena/scripts/analyze-code.sh
\`\`\`

### 更新知识库
\`\`\`bash
./.kiro/serena/scripts/update-knowledge.sh
\`\`\`

### 初始化Serena项目
\`\`\`bash
./.kiro/serena/scripts/init-serena.sh
\`\`\`

## 📁 目录结构
- \`config/\`: Serena项目配置
- \`knowledge/\`: 项目知识库
- \`analysis/\`: 代码分析结果
- \`scripts/\`: 辅助脚本工具
- \`templates/\`: 代码模板
- \`reports/\`: 生成的报告

## 🔧 Serena命令使用

### 项目管理
\`\`\`bash
# 生成项目配置文件
serena project generate-yml

# 索引项目代码
serena project index
\`\`\`

## 💡 与Claude协作的最佳实践

### 1. 开始新任务前
\`\`\`bash
# 更新知识库
./.kiro/serena/scripts/update-knowledge.sh

# 查看项目概览
cat .kiro/serena/knowledge/project-knowledge-base.md
\`\`\`

### 2. 向Claude提供上下文
在与Claude对话时，可以引用以下文件：
- 项目知识库: \`.kiro/serena/knowledge/project-knowledge-base.md\`
- 代码分析: \`.kiro/serena/analysis/code-analysis.md\`

### 3. 常用查询模式
- "基于项目知识库，如何实现XXX功能？"
- "参考现有架构，添加XXX模块的最佳方案是什么？"
- "根据项目特点，如何优化XXX部分的代码？"

## 📊 项目信息总览
- **项目类型**: $PROJECT_TYPE
- **开发框架**: $FRAMEWORK
- **主要语言**: $MAIN_LANGUAGE
- **目标平台**: $PLATFORM

## 🆘 故障排除
如果遇到问题：
1. 查看分析日志: \`.kiro/serena/analysis/\`
2. 检查配置文件: \`.kiro/serena/config/\`
3. 运行诊断脚本: \`./.kiro/serena/scripts/analyze-code.sh\`
EOF

    print_success "使用指南生成完成"
}

# 运行初始分析
run_initial_analysis() {
    print_step "执行初始项目分析..."
    
    # 运行代码分析
    ./.kiro/serena/scripts/analyze-code.sh
    
    # 尝试运行Serena项目初始化
    if ./.kiro/serena/scripts/init-serena.sh > .kiro/serena/analysis/serena-init.log 2>&1; then
        print_success "Serena项目初始化完成"
    else
        print_warning "Serena项目初始化失败，请查看日志: .kiro/serena/analysis/serena-init.log"
    fi
    
    print_success "初始分析完成"
}

# 主函数
main() {
    echo "开始通用项目的Serena配置..."
    echo ""
    
    # 检查Serena
    if ! check_serena; then
        print_error "请先安装Serena后再运行此脚本"
        exit 1
    fi
    
    # 执行配置步骤
    detect_project_type
    create_directories
    generate_config
    generate_knowledge_base
    create_scripts
    generate_usage_guide
    run_initial_analysis
    
    echo ""
    echo "🎉 $PROJECT_NAME - Serena配置完成！"
    echo ""
    echo "📊 配置摘要:"
    echo "   项目名称: $PROJECT_NAME"
    echo "   项目类型: $PROJECT_TYPE ($PLATFORM)"
    echo "   主要语言: $MAIN_LANGUAGE"
    echo "   开发框架: $FRAMEWORK"
    echo ""
    echo "📚 生成的文件:"
    echo "   项目配置: .kiro/serena/config/project.yml"
    echo "   知识库: .kiro/serena/knowledge/project-knowledge-base.md"
    echo "   使用指南: .kiro/serena/README.md"
    echo ""
    echo "🔍 下一步建议:"
    echo "   1. 查看知识库: cat .kiro/serena/knowledge/project-knowledge-base.md"
    echo "   2. 运行分析: ./.kiro/serena/scripts/analyze-code.sh"
    echo "   3. 查看使用指南: cat .kiro/serena/README.md"
    echo ""
    echo "💡 与Claude协作提示:"
    echo "   在对话中引用知识库文件，让Claude更好地理解项目上下文"
    echo ""
    print_success "配置完成！现在可以开始使用Serena辅助开发了。"
}

# 执行主函数
main "$@"