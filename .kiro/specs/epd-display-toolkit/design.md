# ESP32墨水屏显示工具包 - 设计文档
📊 项目技术架构分析
🏗️ 核心架构组件
1. 硬件抽象层 (HAL)
// 墨水屏驱动配置 (config.h)
- 支持多种7.5寸墨水屏：
  * DISP_BW_V2: 800x480px 黑白屏
  * DISP_3C_B:  800x480px 三色屏 (红/黑/白)
  * DISP_7C_F:  800x480px 七色屏
  * DISP_BW_V1: 640x384px 黑白屏(旧版)

// 驱动板支持
- DESPI-C02 (官方推荐)
- Waveshare rev2.2/2.3 (已弃用)
2. 显示渲染系统
// 核心显示类 (renderer.cpp)
GxEPD2_3C<GxEPD2_750c_Z08> display  // 三色屏示例
- 字符串渲染: drawString() 支持对齐
- 图标渲染: 多尺寸位图 (16x16 到 196x196)
- 布局管理: 分区域渲染天气信息
3. 数据处理层
// API响应处理 (api_response.h)
- OpenWeatherMap API集成
- 天气数据结构化存储
- 空气质量数据处理

// 显示工具 (display_utils.cpp)
- 天气图标选择算法
- 电池状态显示
- WiFi信号强度显示
🔧 墨水屏接口技术详解
SPI接口配置
// 引脚定义 (基于DESPI-C02)
#define PIN_EPD_CS    15  // 片选
#define PIN_EPD_DC    27  // 数据/命令选择
#define PIN_EPD_RST   26  // 复位
#define PIN_EPD_BUSY  25  // 忙状态
显示驱动核心
// GxEPD2库封装
template<typename GxEPD2_Type>
class DisplayDriver {
  GxEPD2_Type display;
  
  void init() {
    display.init();
    display.setRotation(1);  // 横屏显示
  }
  
  void refresh() {
    display.display();       // 全屏刷新
  }
  
  void partialRefresh() {
    display.displayWindow(); // 局部刷新
  }
};
🎯 移植实现方案
Phase 1: 核心显示模块提取
1.1 创建独立的显示抽象层
// EPDDisplay.h - 墨水屏抽象接口
class EPDDisplay {
public:
    virtual void init() = 0;
    virtual void clear() = 0;
    virtual void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, 
                           int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawString(int16_t x, int16_t y, const String& text, 
                           uint16_t color) = 0;
    virtual void refresh() = 0;
    virtual void sleep() = 0;
};

// GxEPD2Adapter.h - GxEPD2库适配器
class GxEPD2Adapter : public EPDDisplay {
private:
    GxEPD2_3C<GxEPD2_750c_Z08> display;
public:
    void init() override;
    void drawBitmap(...) override;
    // 实现其他接口...
};
1.2 布局配置系统
// LayoutConfig.h - 布局配置
struct DisplayLayout {
    struct {
        int16_t x, y, w, h;
    } currentWeather;
    
    struct {
        int16_t x, y, w, h;
    } forecast[8];  // 8小时预报
    
    struct {
        int16_t x, y, w, h;
    } dailyForecast[7];  // 7天预报
    
    struct {
        int16_t x, y, w, h;
    } statusBar;
};

// 预定义布局
extern const DisplayLayout LAYOUT_800x480_3COLOR;
extern const DisplayLayout LAYOUT_640x384_BW;
Phase 2: 渲染引擎模块化
2.1 渲染器重构
// WeatherRenderer.h
class WeatherRenderer {
private:
    EPDDisplay* display;
    DisplayLayout layout;
    
public:
    WeatherRenderer(EPDDisplay* disp, const DisplayLayout& layout);
    
    void renderCurrentWeather(const WeatherData& data);
    void renderHourlyForecast(const HourlyData& data);
    void renderDailyForecast(const DailyData& data);
    void renderStatusBar(const SystemStatus& status);
    void renderComplete();
};
2.2 图标资源管理
// IconManager.h
class IconManager {
private:
    std::map<String, const uint8_t*> iconMap;
    
public:
    void loadIcons();
    const uint8_t* getWeatherIcon(int weatherId, int size);
    const uint8_t* getBatteryIcon(int percentage);
    const uint8_t* getWiFiIcon(int rssi);
};
Phase 3: 配置系统重构
3.1 硬件配置抽象
// HardwareConfig.h
struct EPDConfig {
    enum DisplayType {
        BW_V1_640x384,
        BW_V2_800x480,
        COLOR_3C_800x480,
        COLOR_7C_800x480
    };
    
    enum DriverBoard {
        DESPI_C02,
        WAVESHARE_REV22,
        WAVESHARE_REV23
    };
    
    DisplayType displayType;
    DriverBoard driverBoard;
    
    struct {
        int cs, dc, rst, busy;
    } pins;
};
3.2 运行时配置
// DisplayConfig.h
struct DisplayConfig {
    String locale = "en_US";
    bool use24HourFormat = true;
    int refreshInterval = 30;  // 分钟
    bool showBattery = true;
    bool showWiFi = true;
    uint16_t accentColor = GxEPD_RED;
};
Phase 4: 移植工具包
4.1 项目生成器
# create-epd-project.sh
#!/bin/bash
PROJECT_NAME=$1
DISPLAY_TYPE=$2  # bw_v2, 3c_b, 7c_f, bw_v1

mkdir -p $PROJECT_NAME/{src,include,lib,config}

# 复制核心文件
cp -r epd-core/src/* $PROJECT_NAME/src/
cp -r epd-core/include/* $PROJECT_NAME/include/
cp -r epd-core/config/$DISPLAY_TYPE.h $PROJECT_NAME/config/display_config.h

# 生成platformio.ini
generate_platformio_ini $PROJECT_NAME $DISPLAY_TYPE
4.2 配置向导
// ConfigWizard.h
class ConfigWizard {
public:
    EPDConfig detectHardware();
    DisplayConfig setupDisplay();
    void generateConfig(const String& projectPath);
    void validateConfig(const EPDConfig& config);
};
📦 移植包结构
epd-display-toolkit/
├── core/                    # 核心显示引擎
│   ├── EPDDisplay.h        # 显示抽象接口
│   ├── GxEPD2Adapter.cpp   # GxEPD2适配器
│   ├── WeatherRenderer.cpp # 天气渲染器
│   └── IconManager.cpp     # 图标管理
├── layouts/                # 布局配置
│   ├── layout_800x480.h    # 800x480布局
│   └── layout_640x384.h    # 640x384布局
├── icons/                  # 图标资源
│   ├── weather/           # 天气图标
│   ├── system/            # 系统图标
│   └── fonts/             # 字体文件
├── examples/              # 示例项目
│   ├── basic_weather/     # 基础天气显示
│   ├── advanced_layout/   # 高级布局
│   └── custom_icons/      # 自定义图标
├── tools/                 # 工具脚本
│   ├── create_project.sh  # 项目生成器
│   ├── icon_converter.py  # 图标转换工具
│   └── config_wizard.cpp  # 配置向导
└── docs/                  # 文档
    ├── API.md            # API文档
    ├── PORTING.md        # 移植指南
    └── LAYOUTS.md        # 布局设计指南
🚀 快速移植步骤
Step 1: 提取核心代码
# 提取显示相关代码
mkdir epd-toolkit
cp src/display_utils.* epd-toolkit/
cp src/renderer.* epd-toolkit/
cp -r icons/ epd-toolkit/
cp -r fonts/ epd-toolkit/
Step 2: 创建抽象层
# 创建新项目
./tools/create_project.sh my-weather-display 3c_b

# 配置硬件
./tools/config_wizard my-weather-display
Step 3: 集成数据源
// 在新项目中集成你的数据源
class MyDataProvider : public WeatherDataProvider {
public:
    WeatherData getCurrentWeather() override {
        // 你的数据获取逻辑
    }
};
💡 移植优势
模块化设计: 核心显示逻辑与硬件解耦
多屏支持: 轻松适配不同尺寸和类型的墨水屏
布局灵活: 可视化布局配置，支持自定义
资源复用: 图标、字体等资源可跨项目使用
快速部署: 工具链支持一键生成新项目
这个移植方案将你的墨水屏显示技术提炼成可复用的工具包，既保持了原有的功能完整性，又提供了良好的扩展性和移植性。

## 系统架构设计 - "像搭积木一样简单"

### 🏗️ 整体架构图 - 5层结构
```
┌─────────────────────────────────────────────────────────────┐
│  🎯 应用层: 你的天气App                                        │
│     "我要显示今天的天气"                                        │
├─────────────────────────────────────────────────────────────┤
│  🎨 渲染引擎: 专业画家团队                                      │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │  天气画家    │ │  图标管家    │ │  布局设计师  │           │
│  │WeatherRender│ │ IconManager │ │LayoutConfig │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
├─────────────────────────────────────────────────────────────┤
│  🔌 显示抽象层: 万能遥控器                                      │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  EPDDisplay接口: "开机、清屏、画图、写字、刷新、休眠"        │ │
│  └─────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│  🔧 硬件适配层: 翻译官                                          │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │  屏幕翻译官  │ │  通信专家    │ │  引脚管家    │           │
│  │GxEPD2Adapter│ │ SPIInterface│ │ GPIOControl │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
├─────────────────────────────────────────────────────────────┤
│  ⚡ 硬件层: 真实的电子元件                                       │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │   ESP32     │ │   墨水屏     │ │   传感器     │           │
│  │   主控芯片   │ │   显示屏     │ │   温湿度等   │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
└─────────────────────────────────────────────────────────────┘
```

### 🤔 为什么要这样设计？

**就像盖房子一样**:
- **地基(硬件层)**: ESP32芯片和墨水屏，这是基础
- **水电(适配层)**: 把复杂的电路连接变成简单的开关
- **装修(抽象层)**: 提供统一的"开关面板"，不管什么品牌都能用
- **家具(渲染层)**: 专业的"室内设计师"，知道东西放哪里好看
- **生活(应用层)**: 你的实际需求，比如"我要看天气"

**好处是什么？**
- 🔄 **可替换**: 换个屏幕？只需要换"翻译官"
- 🧩 **可复用**: "室内设计师"可以用在任何房子里  
- 🛠️ **易维护**: 每层职责清楚，出问题容易找到原因
- 📈 **可扩展**: 要新功能？在对应层加就行

## 核心组件设计

### 1. 显示抽象层 - "万能遥控器"设计

#### 🎯 设计理念
**就像电视遥控器一样**: 不管是什么品牌的电视，遥控器上都有"开机、换台、调音量"这些基本按钮。

#### 📱 EPDDisplay接口设计
```cpp
class EPDDisplay {
public:
    // 🔌 基础操作 - 就像遥控器的基本按钮
    virtual bool 开机() = 0;                    // init() - 初始化屏幕
    virtual void 清屏(颜色 = 白色) = 0;          // clear() - 清空屏幕
    virtual void 刷新显示() = 0;                 // refresh() - 让屏幕显示内容
    virtual void 休眠() = 0;                    // sleep() - 省电模式
    
    // 🎨 绘制操作 - 就像画笔工具
    virtual void 画图标(位置x, 位置y,           // drawBitmap() - 画图标
                       图标数据, 宽度, 高度, 
                       颜色) = 0;
    virtual void 写文字(位置x, 位置y,           // drawString() - 写文字
                       文字内容, 颜色) = 0;
    
    // 📏 属性查询 - 了解屏幕基本信息
    virtual int 屏幕宽度() const = 0;           // width() - 屏幕有多宽
    virtual int 屏幕高度() const = 0;           // height() - 屏幕有多高  
    virtual bool 支持彩色() const = 0;          // supportsColor() - 能显示彩色吗
};
```

#### 💡 为什么这样设计？
- **简单易懂**: 只有6个基本操作，新手也能快速上手
- **功能完整**: 涵盖了墨水屏的所有基本需求
- **易于扩展**: 需要新功能时可以继续添加
- **跨平台**: 不管什么屏幕，都用这套接口###
 2. GxEPD2适配器设计
```cpp
class GxEPD2Adapter : public EPDDisplay {
private:
    std::unique_ptr<GxEPD2_GFX> display_;
    EPDConfig config_;
    
public:
    GxEPD2Adapter(const EPDConfig& config);
    
    bool init() override;
    void clear(uint16_t color) override;
    void refresh() override;
    void sleep() override;
    
    void drawBitmap(int16_t x, int16_t y, 
                   const uint8_t* bitmap, 
                   int16_t w, int16_t h, 
                   uint16_t color) override;
    void drawString(int16_t x, int16_t y, 
                   const String& text, 
                   uint16_t color) override;
    
    int16_t width() const override { return display_->width(); }
    int16_t height() const override { return display_->height(); }
    bool supportsColor() const override;
};
```

### 3. 布局配置系统
```cpp
struct DisplayLayout {
    struct Region {
        int16_t x, y, w, h;
        
        bool contains(int16_t px, int16_t py) const {
            return px >= x && px < x + w && py >= y && py < y + h;
        }
    };
    
    Region currentWeather;
    Region hourlyForecast[8];
    Region dailyForecast[7];
    Region statusBar;
    Region alerts;
    
    bool validate() const;
    void scale(float factor);
};

// 预定义布局
namespace Layouts {
    extern const DisplayLayout LAYOUT_800x480_3COLOR;
    extern const DisplayLayout LAYOUT_640x384_BW;
    extern const DisplayLayout LAYOUT_800x480_7COLOR;
}
```

### 4. 渲染引擎设计
```cpp
class WeatherRenderer {
private:
    EPDDisplay* display_;
    DisplayLayout layout_;
    IconManager* iconManager_;
    
public:
    WeatherRenderer(EPDDisplay* display, 
                   const DisplayLayout& layout,
                   IconManager* iconManager);
    
    void renderCurrentWeather(const WeatherData& data);
    void renderHourlyForecast(const std::vector<HourlyData>& data);
    void renderDailyForecast(const std::vector<DailyData>& data);
    void renderStatusBar(const SystemStatus& status);
    void renderAlerts(const std::vector<AlertData>& alerts);
    
    void renderComplete();
    void renderError(const String& message);
    
private:
    void drawWeatherIcon(int16_t x, int16_t y, int weatherId, int size);
    void drawTemperature(int16_t x, int16_t y, float temp, const String& unit);
    void drawText(int16_t x, int16_t y, const String& text, 
                  TextAlign align = LEFT);
};
```### 5. 图标
管理系统
```cpp
class IconManager {
private:
    std::map<String, std::map<int, const uint8_t*>> weatherIcons_;
    std::map<String, const uint8_t*> systemIcons_;
    
public:
    bool loadIcons();
    
    const uint8_t* getWeatherIcon(int weatherId, int size);
    const uint8_t* getBatteryIcon(int percentage, int size = 24);
    const uint8_t* getWiFiIcon(int rssi, int size = 16);
    const uint8_t* getAlertIcon(const String& alertType, int size = 32);
    
    bool registerCustomIcon(const String& name, int size, const uint8_t* data);
    std::vector<int> getAvailableSizes(const String& iconName);
};
```

### 6. 配置管理系统
```cpp
struct EPDConfig {
    enum DisplayType {
        BW_V1_640x384,
        BW_V2_800x480,
        COLOR_3C_800x480,
        COLOR_7C_800x480
    };
    
    enum DriverBoard {
        DESPI_C02,
        WAVESHARE_REV22,
        WAVESHARE_REV23
    };
    
    DisplayType displayType;
    DriverBoard driverBoard;
    
    struct {
        int cs = 15;
        int dc = 27;
        int rst = 26;
        int busy = 25;
    } pins;
    
    struct {
        int frequency = 4000000;  // 4MHz
        int mode = SPI_MODE0;
    } spi;
    
    bool validate() const;
    String toString() const;
};

struct DisplayConfig {
    String locale = "en_US";
    bool use24HourFormat = true;
    int refreshInterval = 30;  // minutes
    bool showBattery = true;
    bool showWiFi = true;
    uint16_t accentColor = 0x0000;  // Black
    
    struct {
        bool enabled = true;
        int threshold = 20;  // percentage
    } lowBatteryWarning;
    
    bool validate() const;
    void loadFromNVS();
    void saveToNVS() const;
};
```

## 数据流设计

### 数据流图
```
[Weather API] → [Data Parser] → [Weather Data] → [Renderer] → [Display]
      ↓              ↓              ↓              ↓           ↓
  [HTTP Client] → [JSON Parser] → [Validation] → [Layout] → [E-Paper]
```

### 数据结构设计
```cpp
struct WeatherData {
    float temperature;
    float humidity;
    float pressure;
    int weatherId;
    String description;
    float windSpeed;
    int windDirection;
    int visibility;
    float uvIndex;
    
    struct {
        time_t sunrise;
        time_t sunset;
    } sun;
    
    bool isValid() const;
    String toString() const;
};

struct SystemStatus {
    int batteryPercentage;
    int wifiRSSI;
    time_t lastUpdate;
    String errorMessage;
    
    enum Status {
        OK,
        WARNING,
        ERROR
    } status;
};
```## 错误
处理设计

### 错误分类
```cpp
enum class EPDError {
    NONE = 0,
    INIT_FAILED,
    SPI_ERROR,
    DISPLAY_TIMEOUT,
    MEMORY_ERROR,
    CONFIG_INVALID,
    ICON_NOT_FOUND,
    LAYOUT_INVALID
};

class EPDException : public std::exception {
private:
    EPDError error_;
    String message_;
    
public:
    EPDException(EPDError error, const String& message);
    const char* what() const noexcept override;
    EPDError getError() const { return error_; }
};
```

### 错误恢复策略
1. **显示初始化失败**: 重试3次，失败后进入安全模式
2. **SPI通信错误**: 重置SPI接口，重新初始化
3. **内存不足**: 清理缓存，使用简化显示模式
4. **配置错误**: 使用默认配置，记录警告日志

## 性能优化设计

### 内存管理
```cpp
class MemoryManager {
private:
    static constexpr size_t BUFFER_SIZE = 32768;  // 32KB
    uint8_t displayBuffer_[BUFFER_SIZE];
    size_t bufferUsed_ = 0;
    
public:
    void* allocate(size_t size);
    void deallocate(void* ptr);
    void clear();
    size_t getUsage() const { return bufferUsed_; }
    size_t getAvailable() const { return BUFFER_SIZE - bufferUsed_; }
};
```

### 显示优化
1. **局部刷新**: 仅更新变化的区域
2. **缓存机制**: 缓存常用图标和字体
3. **压缩存储**: 使用RLE压缩存储大图标
4. **异步渲染**: 后台准备下一帧数据

## 测试策略设计

### 单元测试
```cpp
class EPDDisplayTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    std::unique_ptr<EPDDisplay> display_;
    EPDConfig config_;
};

TEST_F(EPDDisplayTest, InitializationTest) {
    EXPECT_TRUE(display_->init());
    EXPECT_GT(display_->width(), 0);
    EXPECT_GT(display_->height(), 0);
}

TEST_F(EPDDisplayTest, DrawOperationsTest) {
    display_->clear();
    display_->drawString(0, 0, "Test", 0x0000);
    // 验证绘制结果
}
```

### 集成测试
1. **硬件兼容性测试**: 测试不同墨水屏和驱动板组合
2. **性能测试**: 测试刷新时间和内存使用
3. **稳定性测试**: 长时间运行测试
4. **功耗测试**: 测试深度睡眠功耗

## 部署架构设计

### 工具包结构
```
epd-display-toolkit/
├── src/                     # 源代码
│   ├── core/               # 核心抽象层
│   ├── adapters/           # 硬件适配器
│   ├── renderers/          # 渲染引擎
│   └── utils/              # 工具函数
├── include/                # 头文件
├── examples/               # 示例项目
├── tools/                  # 开发工具
├── tests/                  # 测试代码
├── docs/                   # 文档
└── resources/              # 资源文件
    ├── icons/              # 图标资源
    ├── fonts/              # 字体文件
    └── layouts/            # 布局配置
```

### 构建系统
```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(epd-display-toolkit)

set(CMAKE_CXX_STANDARD 17)

# 核心库
add_library(epd-core STATIC
    src/core/EPDDisplay.cpp
    src/core/WeatherRenderer.cpp
    src/core/IconManager.cpp
)

# 适配器库
add_library(epd-adapters STATIC
    src/adapters/GxEPD2Adapter.cpp
)

# 示例程序
add_executable(weather-display
    examples/weather-display/main.cpp
)

target_link_libraries(weather-display epd-core epd-adapters)
```

---
*设计文档版本: v1.0*
*创建时间: $(date)*
*最后更新: $(date)*## 🎯 移植实
现方案 - "三步走战略"

### 第一步: 提取核心技术 - "搬家整理"

#### 🏠 现状分析 - "看看家里有什么"
```
现有项目就像一个装修好的房子:
├── 🖼️ 客厅(renderer.cpp) - 负责显示天气界面
├── 🔧 工具间(display_utils.cpp) - 各种显示工具
├── 🎨 装饰品(icons/) - 各种天气图标
├── ⚙️ 配电箱(config.h) - 硬件配置
└── 📚 说明书(README.md) - 使用文档
```

#### 📦 提取策略 - "打包搬家"
```cpp
// 把分散的功能整理成工具包
epd-toolkit/
├── 🔌 EPDDisplay.h          // 万能遥控器
├── 🎨 WeatherRenderer.cpp   // 专业画家
├── 📚 IconManager.cpp       // 图标管家  
├── ⚙️ HardwareConfig.h      // 硬件配置师
└── 🛠️ ProjectGenerator.sh   // 项目生成器
```

### 第二步: 简化使用方式 - "傻瓜式操作"

#### 🎯 设计目标
- **30秒**: 创建新项目
- **3分钟**: 配置硬件
- **30分钟**: 看到完整效果

#### 🚀 使用流程设计
```bash
# 1. 一键创建项目
./create_project.sh my-weather

# 2. 自动检测硬件  
./setup_hardware.sh --auto

# 3. 一键编译上传
./build_and_flash.sh

# 4. 完成！看到天气显示
```

#### 🧩 模块化设计
```cpp
// 像搭积木一样简单
int main() {
    // 1. 选择屏幕类型
    auto display = EPDFactory::create("7.5inch_3color");
    
    // 2. 选择布局样式
    auto layout = LayoutFactory::create("weather_standard");
    
    // 3. 选择数据源
    auto dataSource = DataFactory::create("openweathermap");
    
    // 4. 组装完成
    WeatherStation station(display, layout, dataSource);
    station.run();
    
    return 0;
}
```

### 第三步: 提供完整工具链 - "一站式服务"

#### 🛠️ 开发工具套装
```
工具箱包含:
├── 🏗️ create_project.sh      # 项目生成器
├── 🔍 detect_hardware.sh     # 硬件检测器  
├── 🎨 convert_icons.py       # 图标转换器
├── 📐 layout_designer.html   # 布局设计器
├── 🧪 test_display.sh        # 显示测试器
└── 📚 generate_docs.sh       # 文档生成器
```

#### 📖 示例项目库
```
examples/
├── 🌤️ basic_weather/         # 基础天气显示
│   └── "30分钟上手版本"
├── 🎨 custom_layout/         # 自定义布局
│   └── "1小时进阶版本"  
├── 📊 multi_sensor/          # 多传感器
│   └── "2小时专业版本"
└── 🌐 web_config/            # Web配置
    └── "高级定制版本"
```

## 📦 移植包结构设计 - "标准化包装"

### 🎁 包装设计理念
**就像乐高积木盒**: 每个组件都有标准接口，可以自由组合

### 📋 标准目录结构
```
epd-display-toolkit/           # 工具包根目录
├── 📚 README.md              # "开箱即用指南"
├── 🚀 QUICK_START.md         # "30分钟上手教程"  
├── 📖 docs/                  # 完整文档
│   ├── API_Reference.md      # API参考手册
│   ├── Hardware_Guide.md     # 硬件兼容列表
│   └── Troubleshooting.md    # 常见问题解答
├── 🔧 tools/                 # 开发工具
│   ├── create_project.sh     # 项目生成器
│   ├── setup_hardware.sh     # 硬件配置向导
│   └── convert_icons.py      # 图标转换工具
├── 📚 lib/                   # 核心库文件
│   ├── EPDDisplay/           # 显示抽象层
│   ├── WeatherRenderer/      # 渲染引擎
│   └── IconManager/          # 图标管理
├── 🎨 resources/             # 资源文件
│   ├── icons/                # 完整图标库
│   ├── fonts/                # 字体文件
│   └── layouts/              # 预设布局
├── 📖 examples/              # 示例项目
│   ├── basic_weather/        # 基础版本
│   ├── advanced_layout/      # 进阶版本
│   └── custom_data/          # 自定义数据源
└── 🧪 tests/                 # 测试代码
    ├── unit_tests/           # 单元测试
    └── hardware_tests/       # 硬件测试
```

### 🎯 每个目录的作用

#### 📚 lib/ - "核心引擎"
```cpp
// 就像汽车引擎，提供核心动力
EPDDisplay/           // 显示引擎
├── EPDDisplay.h      // 统一接口定义
├── GxEPD2Adapter.cpp // GxEPD2库适配器
└── MockDisplay.cpp   // 测试用模拟显示器

WeatherRenderer/      // 渲染引擎  
├── WeatherRenderer.h // 天气渲染器
├── LayoutManager.cpp // 布局管理器
└── TextRenderer.cpp  // 文字渲染器

IconManager/          // 图标引擎
├── IconManager.h     // 图标管理器
├── IconLoader.cpp    // 图标加载器
└── IconCache.cpp     // 图标缓存器
```

#### 🔧 tools/ - "瑞士军刀"
```bash
# 项目生成器 - "一键建房"
create_project.sh my-weather 3color
# 输出: 完整的PlatformIO项目

# 硬件配置向导 - "智能装修"  
setup_hardware.sh --interactive
# 输出: 个性化的config.h文件

# 图标转换器 - "格式工厂"
convert_icons.py sunny.png --size 64
# 输出: sunny_64x64.h C数组文件
```

#### 🎨 resources/ - "素材库"
```
icons/                    # 图标素材库
├── weather/              # 天气图标
│   ├── sunny/           # 晴天系列
│   │   ├── sunny_16x16.h
│   │   ├── sunny_32x32.h
│   │   └── sunny_64x64.h
│   └── rainy/           # 雨天系列
├── system/              # 系统图标
│   ├── battery/         # 电池图标
│   └── wifi/            # WiFi图标
└── custom/              # 自定义图标
```

#### 📖 examples/ - "学习教材"
```
basic_weather/           # 新手教程
├── README.md           # "跟我学，30分钟上手"
├── src/main.cpp        # 最简单的实现
└── platformio.ini      # 基础配置

advanced_layout/        # 进阶教程  
├── README.md          # "进阶技巧，1小时掌握"
├── src/custom_layout.h # 自定义布局示例
└── src/main.cpp       # 高级功能演示

custom_data/           # 专家教程
├── README.md         # "专业定制，2小时精通"  
├── src/my_api.cpp    # 自定义数据源
└── src/main.cpp      # 完整项目示例
```

## 🚀 快速移植步骤详解

### 🎯 30分钟快速上手

#### 第1步: 环境准备 (5分钟)
```bash
# 1. 下载工具包
git clone https://github.com/epd-toolkit/epd-display-toolkit.git
cd epd-display-toolkit

# 2. 检查系统环境
./tools/check_environment.sh
# 输出: ✅ PlatformIO已安装
#      ✅ Python3已安装  
#      ✅ Git已安装

# 3. 连接硬件并检测
./tools/detect_hardware.sh
# 输出: 🔍 发现ESP32设备: /dev/ttyUSB0
#      🖥️ 检测到7.5寸三色屏
#      🔌 建议使用DESPI-C02驱动板
```

#### 第2步: 创建项目 (10分钟)
```bash
# 1. 运行项目向导
./tools/create_project.sh

# 交互式配置:
# 🎯 项目名称: my-weather-display
# 🖥️ 屏幕类型: 
#    [1] 7.5寸黑白屏 (800x480)
#    [2] 7.5寸三色屏 (800x480) ← 选择这个
#    [3] 7.5寸七色屏 (800x480)
#    [4] 7.5寸黑白屏 (640x384)
# 🔌 驱动板类型:
#    [1] DESPI-C02 ← 推荐
#    [2] Waveshare Rev2.3
# 📍 引脚配置: 使用默认配置 [Y/n] Y
# 📡 数据源: OpenWeatherMap API [Y/n] Y

# 2. 项目生成完成
cd my-weather-display
ls -la
# 输出: src/  include/  platformio.ini  README.md
```

#### 第3步: 配置密钥 (5分钟)
```bash
# 1. 复制配置模板
cp src/secrets.h.example src/secrets.h

# 2. 编辑配置文件
nano src/secrets.h

# 填入你的信息:
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"  
#define OWM_API_KEY "你的OpenWeatherMap API密钥"
#define LATITUDE 39.9042    // 你的纬度
#define LONGITUDE 116.4074  // 你的经度
```

#### 第4步: 编译上传 (10分钟)
```bash
# 1. 编译项目
pio run
# 输出: ✅ 编译成功

# 2. 上传到ESP32
pio run --target upload  
# 输出: ✅ 上传成功

# 3. 查看运行日志
pio device monitor
# 输出: 🔌 初始化显示屏...
#      📶 连接WiFi...
#      🌤️ 获取天气数据...
#      🖥️ 更新显示...
#      😴 进入深度睡眠...
```

### 🎨 1小时自定义布局

#### 修改布局文件
```cpp
// 编辑 src/my_layout.h
struct MyWeatherLayout {
    // 🌡️ 大号温度显示区 (左上角)
    Region temperature = {20, 20, 200, 100};
    
    // 🌤️ 天气图标区 (右上角)
    Region weatherIcon = {250, 20, 128, 128};
    
    // 📊 详细信息区 (中间)
    Region details = {20, 150, 360, 100};
    
    // 📅 预报区域 (底部)
    Region forecast = {20, 270, 760, 150};
    
    // 🔋 状态栏 (最底部)
    Region statusBar = {20, 440, 760, 40};
};
```

#### 自定义渲染逻辑
```cpp
// 编辑 src/my_renderer.cpp
void MyWeatherRenderer::renderTemperature(float temp) {
    // 🌡️ 画超大号温度
    display->setFont(&DejaVu_Sans_Bold_48);
    display->drawString(50, 80, String(temp, 1) + "°", BLACK);
    
    // 📊 画温度趋势图
    drawTemperatureTrend(50, 100, hourlyData);
}

void MyWeatherRenderer::renderWeatherIcon(int weatherId) {
    // 🌤️ 根据天气选择图标
    const uint8_t* icon = iconManager->getWeatherIcon(weatherId, 128);
    display->drawBitmap(250, 20, icon, 128, 128, BLACK);
    
    // ✨ 添加动画效果 (可选)
    if (weatherId == 800) { // 晴天
        drawSunRays(314, 84); // 在图标周围画阳光
    }
}
```

### 🔌 2小时集成数据源

#### 创建自定义数据源
```cpp
// 创建 src/my_data_source.h
class MyWeatherAPI : public WeatherDataProvider {
private:
    HTTPClient http;
    String apiKey;
    
public:
    MyWeatherAPI(const String& key) : apiKey(key) {}
    
    WeatherData getCurrentWeather() override {
        // 🌐 从你的API获取数据
        String url = "https://my-weather-api.com/current?key=" + apiKey;
        http.begin(url);
        int httpCode = http.GET();
        
        if (httpCode == 200) {
            String response = http.getString();
            return parseWeatherData(response);
        }
        
        return WeatherData(); // 返回空数据
    }
    
private:
    WeatherData parseWeatherData(const String& json) {
        // 📊 解析JSON数据
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, json);
        
        WeatherData data;
        data.temperature = doc["temp"];
        data.humidity = doc["humidity"];
        data.description = doc["description"].as<String>();
        // ... 解析其他字段
        
        return data;
    }
};
```

#### 集成到主程序
```cpp
// 修改 src/main.cpp
#include "my_data_source.h"

void setup() {
    // 🔌 初始化硬件
    display.init();
    
    // 📡 创建自定义数据源
    MyWeatherAPI weatherAPI("your-api-key");
    
    // 🎨 创建渲染器
    MyWeatherRenderer renderer(&display, &layout, &iconManager);
    
    // 🔄 主循环
    while (true) {
        WeatherData data = weatherAPI.getCurrentWeather();
        renderer.render(data);
        display.refresh();
        
        // 😴 深度睡眠30分钟
        esp_deep_sleep(30 * 60 * 1000000);
    }
}
```

---
*设计文档版本: v1.1*  
*更新内容: 添加通俗易懂的移植方案和详细实施步骤*
*最后更新: $(date)*

