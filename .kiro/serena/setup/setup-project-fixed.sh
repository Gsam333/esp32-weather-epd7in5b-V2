#!/bin/bash

# Serena项目配置脚本 - 通用版本
# 自动识别项目类型并生成知识库
# 支持中文和英文知识库生成

set -e

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
last_modified=$(find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.c" -o -name "*.py" -o -name "*.js" -o -name "*.ts" \) -exec ls -lt {} + 2>/dev/null | head -2 | tail -1 | awk '{print $6"-"$7"-"$8" "$9}' || echo "未知")

# 生成知识库主文件
if [[ "$LANGUAGE" == "zh" ]]; then
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
else
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
A $PROJECT_TYPE project based on $FRAMEWORK, developed using $MAIN_LANGUAGE.

### Core Features
EOF
fi

# 根据项目类型和语言添加特性
if [[ "$LANGUAGE" == "zh" ]]; then
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
else
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
        "system")
            cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- ⚙️ System-level programming
- 🚀 High-performance computing
- 🔒 Memory safety
- 🛠️ Low-level system interfaces
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
fi

if [[ "$LANGUAGE" == "zh" ]]; then
    cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

## 🏗️ 项目架构

### 目录结构
\`\`\`
EOF
else
    cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

## 🏗️ Project Architecture

### Directory Structure
\`\`\`
EOF
fi

# 添加目录结构
find . -type d -name ".*" -prune -o -type d -print | head -15 | sed 's|^\./||' | sort >> .kiro/serena/knowledge/project-knowledge-base.md

if [[ "$LANGUAGE" == "zh" ]]; then
    cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF
\`\`\`

### 核心文件
EOF
else
    cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF
\`\`\`

### Core Files
EOF
fi

# 根据语言类型添加核心文件
case $MAIN_LANGUAGE in
    "cpp")
        find . -name "*.cpp" -o -name "*.h" | head -10 | sed 's|^|- |' >> .kiro/serena/knowledge/project-knowledge-base.md
        ;;
    "javascript")
        find . -name "*.js" -o -name "*.ts" | head -10 | sed 's|^|- |' >> .kiro/serena/knowledge/project-knowledge-base.md
        ;;
    "python")
        find . -name "*.py" | head -10 | sed 's|^|- |' >> .kiro/serena/knowledge/project-knowledge-base.md
        ;;
    *)
        find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.py" -o -name "*.js" -o -name "*.rs" -o -name "*.go" \) | head -10 | sed 's|^|- |' >> .kiro/serena/knowledge/project-knowledge-base.md
        ;;
esac

if [[ "$LANGUAGE" == "zh" ]]; then
    cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

### 配置文件
EOF
else
    cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

### Configuration Files
EOF
fi

# 根据框架添加配置文件
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
- **最近修改**: $last_modified

## 🌱 功能模块分析

### 主要模块
EOF

# 根据项目类型添加模块说明
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
    "application")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
1. **核心逻辑**: 主要业务逻辑实现
2. **数据处理**: 数据输入输出和转换
3. **配置管理**: 应用配置和参数管理
4. **错误处理**: 异常捕获和错误恢复
5. **日志系统**: 运行日志和调试信息
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

cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

## 🔍 常见问题和解决方案

### 编译/构建问题
EOF

# 根据框架添加常见问题
case $FRAMEWORK in
    "arduino")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- **依赖库问题**: 检查platformio.ini中的lib_deps配置
- **编译错误**: 确保使用正确的C++标准和编译选项
EOF
        ;;
    "nodejs")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- **依赖安装**: 运行npm install或yarn install
- **版本冲突**: 检查package.json中的依赖版本
EOF
        ;;
    "python")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- **依赖安装**: 运行pip install -r requirements.txt
- **Python版本**: 确保使用正确的Python版本
EOF
        ;;
    *)
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- **依赖问题**: 检查项目依赖配置
- **环境问题**: 确保开发环境配置正确
EOF
        ;;
esac

cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'

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
EOF

# 根据项目类型添加扩展方向
case $PROJECT_TYPE in
    "embedded")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- **新传感器支持**: 添加其他传感器模块
- **通信协议**: 支持新的通信方式
- **用户界面**: 改进显示和交互体验
- **功耗优化**: 进一步降低功耗
EOF
        ;;
    "web")
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- **新页面功能**: 添加新的页面和功能
- **性能优化**: 提升加载速度和响应性能
- **用户体验**: 改进界面设计和交互
- **移动适配**: 优化移动端体验
EOF
        ;;
    *)
        cat >> .kiro/serena/knowledge/project-knowledge-base.md << 'EOF'
- **功能扩展**: 添加新的核心功能
- **性能优化**: 提升运行效率
- **用户体验**: 改进用户界面和交互
- **平台支持**: 支持更多平台和环境
EOF
        ;;
esac

cat >> .kiro/serena/knowledge/project-knowledge-base.md << EOF

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

echo "✅ 项目知识库生成完成"

# 创建辅助脚本
echo "🔧 创建辅助脚本..."

# 代码分析脚本
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

chmod +x .kiro/serena/scripts/analyze-code.sh
echo "✅ 辅助脚本创建完成"

# 生成使用指南
echo "🔧 生成使用指南..."
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

## 📚 更多资源
- [Serena官方文档](https://github.com/serena-ai/serena)
- [项目知识库](.kiro/serena/knowledge/project-knowledge-base.md)
EOF
echo "✅ 使用指南生成完成"

# 执行初始分析
echo "🔧 执行初始项目分析..."
./.kiro/serena/scripts/analyze-code.sh
echo "✅ 初始分析完成"

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
echo "✅ 配置完成！现在可以开始使用Serena辅助开发了。"