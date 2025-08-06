#!/bin/bash

# Serena项目配置脚本 - 通用版本
# 自动识别项目类型并生成知识库
# 支持中文和英文知识库生成

set -e

# 设置UTF-8编码环境
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

# 默认语言设置
LANGUAGE="zh"

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        --lang|--language)
            LANGUAGE="$2"
            shift 2
            ;;
        --en|--english)
            LANGUAGE="en"
            shift
            ;;
        --zh|--chinese)
            LANGUAGE="zh"
            shift
            ;;
        -h|--help)
            echo "用法: $0 [选项]"
            echo "选项:"
            echo "  --lang, --language LANG  设置语言 (zh/en)"
            echo "  --zh, --chinese         使用中文"
            echo "  --en, --english         使用英文"
            echo "  -h, --help              显示帮助信息"
            echo ""
            echo "示例:"
            echo "  $0                      # 默认中文"
            echo "  $0 --en                 # 英文版本"
            echo "  $0 --lang zh            # 中文版本"
            exit 0
            ;;
        *)
            echo "未知参数: $1"
            echo "使用 -h 或 --help 查看帮助"
            exit 1
            ;;
    esac
done

# 验证语言参数
if [[ "$LANGUAGE" != "zh" && "$LANGUAGE" != "en" ]]; then
    echo "错误: 不支持的语言 '$LANGUAGE'，请使用 'zh' 或 'en'"
    exit 1
fi

# 根据语言设置显示信息
if [[ "$LANGUAGE" == "zh" ]]; then
    echo "🚀 Serena项目配置 - 通用版本"
    echo "自动识别项目类型并生成知识库 (中文版)"
else
    echo "🚀 Serena Project Setup - Universal Version"
    echo "Auto-detect project type and generate knowledge base (English)"
fi
echo ""

# 获取项目基本信息
PROJECT_PATH=$(pwd)
PROJECT_NAME=$(basename "$PROJECT_PATH")

# 根据语言显示进度信息
if [[ "$LANGUAGE" == "zh" ]]; then
    echo "开始通用项目的Serena配置..."
    echo ""
    echo "🔧 检查Serena可用性..."
    if command -v serena >/dev/null 2>&1; then
        SERENA_PATH=$(which serena)
        echo "✅ Serena可用: $SERENA_PATH"
    else
        echo "❌ Serena未安装或不在PATH中"
        echo "请先安装Serena: https://github.com/serena-ai/serena"
        exit 1
    fi
    echo "🔧 智能检测项目类型..."
else
    echo "Starting universal Serena project setup..."
    echo ""
    echo "🔧 Checking Serena availability..."
    if command -v serena >/dev/null 2>&1; then
        SERENA_PATH=$(which serena)
        echo "✅ Serena available: $SERENA_PATH"
    else
        echo "❌ Serena not installed or not in PATH"
        echo "Please install Serena first: https://github.com/serena-ai/serena"
        exit 1
    fi
    echo "🔧 Smart project type detection..."
fi

# 检测函数
detect_project_type() {
    # 检查是否为嵌入式项目
    if [[ -f "platformio.ini" ]]; then
        echo "embedded"
        return
    fi
    
    # 检查是否为Node.js项目
    if [[ -f "package.json" ]]; then
        if grep -q "react" package.json 2>/dev/null; then
            echo "react"
        elif grep -q "vue" package.json 2>/dev/null; then
            echo "vue"
        elif grep -q "express" package.json 2>/dev/null; then
            echo "nodejs"
        else
            echo "web"
        fi
        return
    fi
    
    # 检查是否为Python项目
    if [[ -f "requirements.txt" ]] || [[ -f "pyproject.toml" ]] || [[ -f "setup.py" ]]; then
        echo "python"
        return
    fi
    
    # 检查是否为Rust项目
    if [[ -f "Cargo.toml" ]]; then
        echo "rust"
        return
    fi
    
    # 检查是否为Go项目
    if [[ -f "go.mod" ]]; then
        echo "go"
        return
    fi
    
    # 检查是否为Java项目
    if [[ -f "pom.xml" ]] || [[ -f "build.gradle" ]]; then
        echo "java"
        return
    fi
    
    # 默认为通用应用
    echo "application"
}

# 检测主要语言
detect_main_language() {
    local project_type=$1
    
    case $project_type in
        "embedded")
            echo "cpp"
            ;;
        "react"|"vue"|"nodejs"|"web")
            echo "javascript"
            ;;
        "python")
            echo "python"
            ;;
        "rust")
            echo "rust"
            ;;
        "go")
            echo "go"
            ;;
        "java")
            echo "java"
            ;;
        *)
            # 通过文件扩展名检测
            if find . -name "*.cpp" -o -name "*.c" | head -1 | grep -q .; then
                echo "cpp"
            elif find . -name "*.py" | head -1 | grep -q .; then
                echo "python"
            elif find . -name "*.js" -o -name "*.ts" | head -1 | grep -q .; then
                echo "javascript"
            else
                echo "unknown"
            fi
            ;;
    esac
}

# 检测开发框架
detect_framework() {
    local project_type=$1
    
    case $project_type in
        "embedded")
            if grep -q "framework = arduino" platformio.ini 2>/dev/null; then
                echo "arduino"
            elif grep -q "framework = espidf" platformio.ini 2>/dev/null; then
                echo "esp-idf"
            else
                echo "platformio"
            fi
            ;;
        "react")
            echo "react"
            ;;
        "vue")
            echo "vue"
            ;;
        "nodejs")
            echo "nodejs"
            ;;
        "python")
            if [[ -f "requirements.txt" ]] && grep -q "django" requirements.txt 2>/dev/null; then
                echo "django"
            elif [[ -f "requirements.txt" ]] && grep -q "flask" requirements.txt 2>/dev/null; then
                echo "flask"
            else
                echo "python"
            fi
            ;;
        *)
            echo "generic"
            ;;
    esac
}

# 检测目标平台
detect_platform() {
    local project_type=$1
    
    case $project_type in
        "embedded")
            if grep -q "board = esp32" platformio.ini 2>/dev/null; then
                echo "esp32"
            elif grep -q "board = esp8266" platformio.ini 2>/dev/null; then
                echo "esp8266"
            elif grep -q "platform = atmelavr" platformio.ini 2>/dev/null; then
                echo "arduino"
            else
                echo "embedded"
            fi
            ;;
        "web"|"react"|"vue"|"nodejs")
            echo "web"
            ;;
        *)
            echo "cross-platform"
            ;;
    esac
}

# 执行检测
PROJECT_TYPE=$(detect_project_type)
MAIN_LANGUAGE=$(detect_main_language $PROJECT_TYPE)
FRAMEWORK=$(detect_framework $PROJECT_TYPE)
PLATFORM=$(detect_platform $PROJECT_TYPE)

# 根据语言显示检测结果
if [[ "$LANGUAGE" == "zh" ]]; then
    echo "✅ 检测到${PROJECT_TYPE}项目: $PLATFORM"
    echo "📊 项目信息:"
    echo "   名称: $PROJECT_NAME"
    echo "   类型: $PROJECT_TYPE"
    echo "   语言: $MAIN_LANGUAGE"
    echo "   平台: $PLATFORM"
    echo "   框架: $FRAMEWORK"
    echo "🔧 创建Serena目录结构..."
    mkdir -p .kiro/serena/{config,knowledge,scripts,analysis,templates}
    echo "✅ 目录结构创建完成"
    echo "🔧 生成项目配置文件..."
else
    echo "✅ Detected ${PROJECT_TYPE} project: $PLATFORM"
    echo "📊 Project Information:"
    echo "   Name: $PROJECT_NAME"
    echo "   Type: $PROJECT_TYPE"
    echo "   Language: $MAIN_LANGUAGE"
    echo "   Platform: $PLATFORM"
    echo "   Framework: $FRAMEWORK"
    echo "🔧 Creating Serena directory structure..."
    mkdir -p .kiro/serena/{config,knowledge,scripts,analysis,templates}
    echo "✅ Directory structure created"
    echo "🔧 Generating project configuration..."
fi

# 生成项目配置文件
cat > .kiro/serena/config/project.yml << EOF
# Serena项目配置文件
project:
  name: "$PROJECT_NAME"
  type: "$PROJECT_TYPE"
  language: "$MAIN_LANGUAGE"
  framework: "$FRAMEWORK"
  platform: "$PLATFORM"
  path: "$PROJECT_PATH"
  created: "$(date)"
  kb_language: "$LANGUAGE"

# 分析配置
analysis:
  include_patterns:
    - "*.cpp"
    - "*.h"
    - "*.c"
    - "*.py"
    - "*.js"
    - "*.ts"
    - "*.rs"
    - "*.go"
  exclude_patterns:
    - "node_modules/**"
    - ".git/**"
    - "build/**"
    - "dist/**"
    - "__pycache__/**"

# 知识库配置
knowledge:
  auto_update: true
  include_code_examples: true
  include_dependencies: true
  language: "$LANGUAGE"
EOF

if [[ "$LANGUAGE" == "zh" ]]; then
    echo "✅ 项目配置文件生成完成"
    echo "🔧 生成项目知识库..."
else
    echo "✅ Project configuration generated"
    echo "🔧 Generating project knowledge base..."
fi

# 统计项目信息
file_count=$(find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.py" -o -name "*.js" -o -name "*.ts" -o -name "*.rs" -o -name "*.go" \) | wc -l | tr -d ' ')
code_lines=$(find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.py" -o -name "*.js" -o -name "*.ts" -o -name "*.rs" -o -name "*.go" \) -exec wc -l {} + 2>/dev/null | tail -1 | awk '{print $1}' || echo "0")

# 生成知识库函数
generate_knowledge_base_zh() {
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
基于${FRAMEWORK}的${PROJECT_TYPE}项目，使用${MAIN_LANGUAGE}开发。

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
- 📱 响应式设计
- 🔄 异步数据处理
- 🔐 用户认证和授权
EOF
            ;;
        "application")
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- 💻 桌面/服务器应用
- 📊 数据处理和分析
- 🔗 API接口开发
- 📦 模块化架构
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
$(find . -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.py" -o -name "*.js" -o -name "*.ts" | head -10 | sed 's|^|- |')

### 配置文件
EOF

    case $FRAMEWORK in
        "arduino")
            echo "- platformio.ini: PlatformIO项目配置" >> .kiro/serena/knowledge/project-knowledge-base.md
            ;;
        "nodejs"|"react"|"vue")
            echo "- package.json: Node.js项目配置" >> .kiro/serena/knowledge/project-knowledge-base.md
            [[ -f "tsconfig.json" ]] && echo "- tsconfig.json: TypeScript配置" >> .kiro/serena/knowledge/project-knowledge-base.md
            ;;
        "python"|"django"|"flask")
            [[ -f "requirements.txt" ]] && echo "- requirements.txt: Python依赖" >> .kiro/serena/knowledge/project-knowledge-base.md
            [[ -f "pyproject.toml" ]] && echo "- pyproject.toml: 现代Python配置" >> .kiro/serena/knowledge/project-knowledge-base.md
            ;;
        *)
            echo "- 项目特定配置文件" >> .kiro/serena/knowledge/project-knowledge-base.md
            ;;
    esac

    cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

## 🔧 技术栈详解

### 开发环境
- **项目类型**: $PROJECT_TYPE
- **编程语言**: $MAIN_LANGUAGE
- **开发框架**: $FRAMEWORK
- **目标平台**: $PLATFORM

### 项目统计
- **代码文件数**: $file_count 个
- **总代码行数**: $code_lines 行

## 🌱 功能模块分析

### 主要模块
EOF

    case $PROJECT_TYPE in
        "embedded")
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
1. **硬件抽象层**: GPIO、SPI、I2C等硬件接口
2. **传感器模块**: 数据采集和处理
3. **通信模块**: WiFi、蓝牙等无线通信
4. **显示模块**: 屏幕显示和用户界面
5. **电源管理**: 低功耗控制和电池管理
EOF
            ;;
        "web")
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
1. **前端界面**: 用户界面和交互逻辑
2. **路由管理**: 页面导航和状态管理
3. **数据层**: API调用和数据处理
4. **组件库**: 可复用的UI组件
5. **工具函数**: 通用工具和辅助函数
EOF
            ;;
        *)
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
1. **核心模块**: 主要功能实现
2. **工具模块**: 辅助工具和函数
3. **配置模块**: 配置管理和参数设置
4. **测试模块**: 单元测试和集成测试
5. **文档模块**: 项目文档和说明
EOF
            ;;
    esac

    cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'

## 🔍 常见问题和解决方案

### 编译/构建问题
- **依赖问题**: 检查项目依赖配置
- **环境问题**: 确保开发环境配置正确

### 运行时问题
- **配置错误**: 检查配置文件格式和参数
- **权限问题**: 确保有足够的文件和网络权限
- **资源不足**: 检查内存和存储空间使用情况

## 📋 开发规范

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
- **功能扩展**: 添加新的核心功能
- **性能优化**: 提升运行效率
- **用户体验**: 改进用户界面和交互
- **平台支持**: 支持更多平台和环境

## 📚 知识库维护

### 更新记录
- **v1.0** ($(date +%Y-%m-%d)): 自动生成初始项目知识库
- 基础项目信息和架构分析
- 核心模块功能说明
- 开发规范和最佳实践

### 待完善内容
- [ ] 详细的API接口文档
- [ ] 架构设计图和流程图
- [ ] 性能优化指南
- [ ] 部署和运维指南
- [ ] 故障排除指南

---
*此知识库由Serena自动生成，随项目发展持续更新*
EOF
}

generate_knowledge_base_en() {
    cat > .kiro/serena/knowledge/project-knowledge-base.md << EOF
# $PROJECT_NAME Project Knowledge Base

## 📊 Project Overview

### Basic Information
- **Project Name**: $PROJECT_NAME
- **Project Type**: $PROJECT_TYPE
- **Main Language**: $MAIN_LANGUAGE
- **Development Framework**: $FRAMEWORK
- **Target Platform**: $PLATFORM
- **Created Time**: $(date)
- **Project Path**: $PROJECT_PATH

### Project Description
A ${PROJECT_TYPE} project based on ${FRAMEWORK}, developed using ${MAIN_LANGUAGE}.

### Core Features
EOF

    # Add features based on project type
    case $PROJECT_TYPE in
        "embedded")
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- 🔌 Embedded system development
- ⚡ Low power design
- 🔧 Hardware interface control
- 📡 Sensor data acquisition
EOF
            ;;
        "web")
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- 🌐 Web application development
- 📱 Responsive design
- 🔄 Asynchronous data processing
- 🔐 User authentication and authorization
EOF
            ;;
        "application")
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- 💻 Desktop/server applications
- 📊 Data processing and analysis
- 🔗 API interface development
- 📦 Modular architecture
EOF
            ;;
        *)
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- 🔧 General software development
- 📚 Modular design
- 🔄 Continuous integration
- 📝 Documentation-driven development
EOF
            ;;
    esac

    cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

## 🏗️ Project Architecture

### Directory Structure
\`\`\`
$(find . -type d -name ".*" -prune -o -type d -print | head -15 | sed 's|^\./||' | sort)
\`\`\`

### Core Files
$(find . -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.py" -o -name "*.js" -o -name "*.ts" | head -10 | sed 's|^|- |')

### Configuration Files
EOF

    case $FRAMEWORK in
        "arduino")
            echo "- platformio.ini: PlatformIO project configuration" >> .kiro/serena/knowledge/project-knowledge-base.md
            ;;
        "nodejs"|"react"|"vue")
            echo "- package.json: Node.js project configuration" >> .kiro/serena/knowledge/project-knowledge-base.md
            [[ -f "tsconfig.json" ]] && echo "- tsconfig.json: TypeScript configuration" >> .kiro/serena/knowledge/project-knowledge-base.md
            ;;
        "python"|"django"|"flask")
            [[ -f "requirements.txt" ]] && echo "- requirements.txt: Python dependencies" >> .kiro/serena/knowledge/project-knowledge-base.md
            [[ -f "pyproject.toml" ]] && echo "- pyproject.toml: Modern Python configuration" >> .kiro/serena/knowledge/project-knowledge-base.md
            ;;
        *)
            echo "- Project-specific configuration files" >> .kiro/serena/knowledge/project-knowledge-base.md
            ;;
    esac

    cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

## 🔧 Technology Stack Details

### Development Environment
- **Project Type**: $PROJECT_TYPE
- **Programming Language**: $MAIN_LANGUAGE
- **Development Framework**: $FRAMEWORK
- **Target Platform**: $PLATFORM

### Project Statistics
- **Code Files**: $file_count files
- **Total Lines of Code**: $code_lines lines

## 🌱 Functional Module Analysis

### Main Modules
EOF

    case $PROJECT_TYPE in
        "embedded")
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
1. **Hardware Abstraction Layer**: GPIO, SPI, I2C and other hardware interfaces
2. **Sensor Module**: Data acquisition and processing
3. **Communication Module**: WiFi, Bluetooth and other wireless communication
4. **Display Module**: Screen display and user interface
5. **Power Management**: Low power control and battery management
EOF
            ;;
        "web")
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
1. **Frontend Interface**: User interface and interaction logic
2. **Route Management**: Page navigation and state management
3. **Data Layer**: API calls and data processing
4. **Component Library**: Reusable UI components
5. **Utility Functions**: Common tools and helper functions
EOF
            ;;
        *)
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
1. **Core Module**: Main functionality implementation
2. **Utility Module**: Helper tools and functions
3. **Configuration Module**: Configuration management and parameter settings
4. **Testing Module**: Unit tests and integration tests
5. **Documentation Module**: Project documentation and descriptions
EOF
            ;;
    esac

    cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'

## 🔍 Common Issues and Solutions

### Build/Compilation Issues
- **Dependency Issues**: Check project dependency configuration
- **Environment Issues**: Ensure development environment is properly configured

### Runtime Issues
- **Configuration Errors**: Check configuration file format and parameters
- **Permission Issues**: Ensure sufficient file and network permissions
- **Resource Shortage**: Check memory and storage space usage

## 📋 Development Standards

### Code Style
- **Naming Conventions**: Use clear, descriptive variable and function names
- **Comment Requirements**: Key functions and complex logic must have comments
- **Error Handling**: Comprehensive error checking and exception handling

### Best Practices
- **Modular Design**: Keep code modular and reusable
- **Version Control**: Use Git for version management
- **Test-Driven**: Write unit tests and integration tests
- **Documentation Updates**: Keep project documentation up to date

## 🎯 Extension Development Guide

### Steps to Add New Features
1. **Requirements Analysis**: Clarify functional requirements and technical solutions
2. **Architecture Design**: Determine module structure and interface design
3. **Code Implementation**: Write core functionality code
4. **Testing and Verification**: Unit testing and integration testing
5. **Documentation Updates**: Update related documentation and comments

### Common Extension Directions
- **Feature Extensions**: Add new core functionality
- **Performance Optimization**: Improve runtime efficiency
- **User Experience**: Improve user interface and interaction
- **Platform Support**: Support more platforms and environments

## 📚 Knowledge Base Maintenance

### Update History
- **v1.0** ($(date +%Y-%m-%d)): Auto-generated initial project knowledge base
- Basic project information and architecture analysis
- Core module functionality descriptions
- Development standards and best practices

### Content to be Improved
- [ ] Detailed API interface documentation
- [ ] Architecture design diagrams and flowcharts
- [ ] Performance optimization guides
- [ ] Deployment and operations guides
- [ ] Troubleshooting guides

---
*This knowledge base is automatically generated by Serena and continuously updated with project development*
EOF
}

# 根据语言生成知识库
if [[ "$LANGUAGE" == "zh" ]]; then
    generate_knowledge_base_zh
else
    generate_knowledge_base_en
fi

if [[ "$LANGUAGE" == "zh" ]]; then
    echo "✅ 项目知识库生成完成"
    echo "🔧 创建辅助脚本..."
else
    echo "✅ Project knowledge base generated"
    echo "🔧 Creating helper scripts..."
fi

# 创建辅助脚本
if [[ "$LANGUAGE" == "zh" ]]; then
    cat > .kiro/serena/scripts/analyze-code.sh << 'EOF'
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
EOF
else
    cat > .kiro/serena/scripts/analyze-code.sh << 'EOF'
#!/bin/bash
# Project code analysis script

echo "🔍 Starting project code analysis..."

# Create analysis directory
mkdir -p .kiro/serena/analysis

# Basic statistics
echo "📊 Project statistical analysis..."
{
    echo "# Project Code Analysis Report"
    echo ""
    echo "Generated: $(date)"
    echo ""
    echo "## File Statistics"
    echo ""
    echo "### Statistics by File Type"
    find . -type f -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.py" -o -name "*.js" -o -name "*.ts" | \
    sed 's/.*\.//' | sort | uniq -c | sort -nr | \
    awk '{printf "- %s: %d files\n", $2, $1}'
    echo ""
    echo "### Lines of Code Statistics"
    find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.py" -o -name "*.js" -o -name "*.ts" \) \
    -exec wc -l {} + 2>/dev/null | tail -1 | awk '{printf "- Total lines of code: %d lines\n", $1}'
    echo ""
} > .kiro/serena/analysis/code-analysis.md

echo "✅ Code analysis completed, results saved to .kiro/serena/analysis/code-analysis.md"
EOF
fi

chmod +x .kiro/serena/scripts/analyze-code.sh

if [[ "$LANGUAGE" == "zh" ]]; then
    echo "✅ 辅助脚本创建完成"
    echo "🔧 生成使用指南..."
else
    echo "✅ Helper scripts created"
    echo "🔧 Generating usage guide..."
fi

# 生成使用指南
if [[ "$LANGUAGE" == "zh" ]]; then
    cat > .kiro/serena/README.md << EOF
# Serena 项目助手使用指南

## 📖 概述
这是为 $PROJECT_NAME 项目自动生成的Serena配置，包含项目知识库、分析工具和辅助脚本。

## 📁 目录结构
\`\`\`
.kiro/serena/
├── config/          # 配置文件
│   └── project.yml  # 项目配置
├── knowledge/       # 知识库
│   └── project-knowledge-base.md
├── scripts/         # 辅助脚本
│   └── analyze-code.sh
├── analysis/        # 分析结果
└── README.md        # 使用指南
\`\`\`

## 🚀 快速开始

### 1. 查看项目知识库
\`\`\`bash
cat .kiro/serena/knowledge/project-knowledge-base.md
\`\`\`

### 2. 运行代码分析
\`\`\`bash
./.kiro/serena/scripts/analyze-code.sh
\`\`\`

### 3. 与Claude协作
在对话中引用知识库文件，让Claude更好地理解项目上下文：
\`\`\`
请参考 #.kiro/serena/knowledge/project-knowledge-base.md 来帮我...
\`\`\`

## 🔧 自定义配置
编辑 \`.kiro/serena/config/project.yml\` 来调整项目配置。

## 🌐 多语言支持
重新运行脚本时可以选择语言：
\`\`\`bash
# 生成英文版知识库
./.kiro/serena/setup/setup-project-multilang.sh --en

# 生成中文版知识库
./.kiro/serena/setup/setup-project-multilang.sh --zh
\`\`\`

## 📚 更多资源
- [Serena官方文档](https://github.com/serena-ai/serena)
- [项目知识库](.kiro/serena/knowledge/project-knowledge-base.md)
EOF
else
    cat > .kiro/serena/README.md << EOF
# Serena Project Assistant Usage Guide

## 📖 Overview
This is an automatically generated Serena configuration for the $PROJECT_NAME project, including project knowledge base, analysis tools, and helper scripts.

## 📁 Directory Structure
\`\`\`
.kiro/serena/
├── config/          # Configuration files
│   └── project.yml  # Project configuration
├── knowledge/       # Knowledge base
│   └── project-knowledge-base.md
├── scripts/         # Helper scripts
│   └── analyze-code.sh
├── analysis/        # Analysis results
└── README.md        # Usage guide
\`\`\`

## 🚀 Quick Start

### 1. View Project Knowledge Base
\`\`\`bash
cat .kiro/serena/knowledge/project-knowledge-base.md
\`\`\`

### 2. Run Code Analysis
\`\`\`bash
./.kiro/serena/scripts/analyze-code.sh
\`\`\`

### 3. Collaborate with Claude
Reference the knowledge base file in conversations to help Claude better understand the project context:
\`\`\`
Please refer to #.kiro/serena/knowledge/project-knowledge-base.md to help me...
\`\`\`

## 🔧 Custom Configuration
Edit \`.kiro/serena/config/project.yml\` to adjust project configuration.

## 🌐 Multi-language Support
You can choose the language when re-running the script:
\`\`\`bash
# Generate English knowledge base
./.kiro/serena/setup/setup-project-multilang.sh --en

# Generate Chinese knowledge base
./.kiro/serena/setup/setup-project-multilang.sh --zh
\`\`\`

## 📚 More Resources
- [Serena Official Documentation](https://github.com/serena-ai/serena)
- [Project Knowledge Base](.kiro/serena/knowledge/project-knowledge-base.md)
EOF
fi

if [[ "$LANGUAGE" == "zh" ]]; then
    echo "✅ 使用指南生成完成"
    echo "🔧 执行初始项目分析..."
else
    echo "✅ Usage guide generated"
    echo "🔧 Running initial project analysis..."
fi

# 执行初始分析
./.kiro/serena/scripts/analyze-code.sh

if [[ "$LANGUAGE" == "zh" ]]; then
    echo "✅ 初始分析完成"
    echo ""
    echo "🎉 $PROJECT_NAME - Serena配置完成！"
    echo ""
    echo "📊 配置摘要:"
    echo "   项目名称: $PROJECT_NAME"
    echo "   项目类型: $PROJECT_TYPE ($PLATFORM)"
    echo "   主要语言: $MAIN_LANGUAGE"
    echo "   开发框架: $FRAMEWORK"
    echo "   知识库语言: $LANGUAGE"
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
    echo "✅ 配置完成！现在可以开始使用Serena辅助开发了。"
else
    echo "✅ Initial analysis completed"
    echo ""
    echo "🎉 $PROJECT_NAME - Serena setup completed!"
    echo ""
    echo "📊 Configuration Summary:"
    echo "   Project Name: $PROJECT_NAME"
    echo "   Project Type: $PROJECT_TYPE ($PLATFORM)"
    echo "   Main Language: $MAIN_LANGUAGE"
    echo "   Development Framework: $FRAMEWORK"
    echo "   Knowledge Base Language: $LANGUAGE"
    echo ""
    echo "📚 Generated Files:"
    echo "   Project Config: .kiro/serena/config/project.yml"
    echo "   Knowledge Base: .kiro/serena/knowledge/project-knowledge-base.md"
    echo "   Usage Guide: .kiro/serena/README.md"
    echo ""
    echo "🔍 Next Steps:"
    echo "   1. View knowledge base: cat .kiro/serena/knowledge/project-knowledge-base.md"
    echo "   2. Run analysis: ./.kiro/serena/scripts/analyze-code.sh"
    echo "   3. View usage guide: cat .kiro/serena/README.md"
    echo ""
    echo "💡 Claude Collaboration Tips:"
    echo "   Reference the knowledge base file in conversations for better project context"
    echo ""
    echo "✅ Setup complete! You can now start using Serena for development assistance."
fi