# ESP32墨水屏天气显示系统 - 技术架构文档

## 📊 系统架构概览

### 🏗️ 核心架构组件

#### 1. **硬件抽象层 (HAL)**
```cpp
// 墨水屏驱动配置 (config.h)
- 支持多种7.5寸墨水屏：
  * DISP_BW_V2: 800x480px 黑白屏
  * DISP_3C_B:  800x480px 三色屏 (红/黑/白)
  * DISP_7C_F:  800x480px 七色屏
  * DISP_BW_V1: 640x384px 黑白屏(旧版)

// 驱动板支持
- DESPI-C02 (官方推荐)
- Waveshare rev2.2/2.3 (已弃用)
```

#### 2. **显示渲染系统**
```cpp
// 核心显示类 (renderer.cpp)
GxEPD2_3C<GxEPD2_750c_Z08> display  // 三色屏示例
- 字符串渲染: drawString() 支持对齐
- 图标渲染: 多尺寸位图 (16x16 到 196x196)
- 布局管理: 分区域渲染天气信息
```

#### 3. **数据处理层**
```cpp
// API响应处理 (api_response.h)
- OpenWeatherMap API集成
- 天气数据结构化存储
- 空气质量数据处理

// 显示工具 (display_utils.cpp)
- 天气图标选择算法
- 电池状态显示
- WiFi信号强度显示
```

### 🔧 墨水屏接口技术详解

#### **SPI接口配置**
```cpp
// 引脚定义 (基于DESPI-C02)
#define PIN_EPD_CS    15  // 片选
#define PIN_EPD_DC    27  // 数据/命令选择
#define PIN_EPD_RST   26  // 复位
#define PIN_EPD_BUSY  25  // 忙状态
```

#### **显示驱动核心**
```cpp
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
```

## 🎯 移植架构设计

### **Phase 1: 核心显示模块提取**

#### 1.1 创建独立的显示抽象层
```cpp
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
```

#### 1.2 布局配置系统
```cpp
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
```

### **Phase 2: 渲染引擎模块化**

#### 2.1 渲染器重构
```cpp
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
```

#### 2.2 图标资源管理
```cpp
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
```

### **Phase 3: 配置系统重构**

#### 3.1 硬件配置抽象
```cpp
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
```

#### 3.2 运行时配置
```cpp
// DisplayConfig.h
struct DisplayConfig {
    String locale = "en_US";
    bool use24HourFormat = true;
    int refreshInterval = 30;  // 分钟
    bool showBattery = true;
    bool showWiFi = true;
    uint16_t accentColor = GxEPD_RED;
};
```

## 📦 移植工具包结构

```
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
```

## 🚀 快速移植步骤

### **Step 1: 提取核心代码**
```bash
# 提取显示相关代码
mkdir epd-toolkit
cp src/display_utils.* epd-toolkit/
cp src/renderer.* epd-toolkit/
cp -r icons/ epd-toolkit/
cp -r fonts/ epd-toolkit/
```

### **Step 2: 创建抽象层**
```bash
# 创建新项目
./tools/create_project.sh my-weather-display 3c_b

# 配置硬件
./tools/config_wizard my-weather-display
```

### **Step 3: 集成数据源**
```cpp
// 在新项目中集成你的数据源
class MyDataProvider : public WeatherDataProvider {
public:
    WeatherData getCurrentWeather() override {
        // 你的数据获取逻辑
    }
};
```

## 💡 移植优势

1. **模块化设计**: 核心显示逻辑与硬件解耦
2. **多屏支持**: 轻松适配不同尺寸和类型的墨水屏
3. **布局灵活**: 可视化布局配置，支持自定义
4. **资源复用**: 图标、字体等资源可跨项目使用
5. **快速部署**: 工具链支持一键生成新项目

## 🔧 核心文件分析

### 主要源文件
- `src/main.cpp`: 主程序入口，包含深度睡眠管理
- `src/renderer.cpp`: 显示渲染引擎，处理所有UI绘制
- `src/display_utils.cpp`: 显示工具函数，图标选择和数据处理
- `src/config.h`: 硬件配置和编译选项
- `src/api_response.h`: 天气API数据结构定义

### 关键依赖库
- `GxEPD2`: 墨水屏驱动库
- `Adafruit GFX`: 图形绘制库
- `ArduinoJson`: JSON数据解析
- `WiFiClientSecure`: HTTPS通信

### 硬件接口
- SPI通信协议
- GPIO控制引脚
- ADC电池电压检测
- I2C传感器接口

---
*文档更新时间: Wed Jul 30 19:42:43 CST 2025*
*基于项目版本: esp32-weather-epd7in5b-V2*