# ESP32 电子墨水屏天气显示器 - 概要设计文档 (HLD)

## 1. 文档信息

| 项目 | 内容 |
|------|------|
| 文档标题 | ESP32 电子墨水屏天气显示器概要设计文档 |
| 版本 | 1.0 |
| 日期 | 2025年7月22日 |
| 状态 | 已审核 |
| 作者 | 项目团队 |

## 2. 修订历史

| 版本 | 日期 | 描述 | 作者 |
|------|------|------|------|
| 1.0 | 2025-07-22 | 初始版本 | 项目团队 |

## 3. 引言

### 3.1 目的

本文档旨在提供ESP32电子墨水屏天气显示器项目的概要设计，包括系统架构、主要组件、接口定义和数据流。本文档面向项目开发人员、测试人员和项目管理人员，作为系统实现的指导性文档。

### 3.2 范围

本文档涵盖ESP32电子墨水屏天气显示器的整体系统设计，包括硬件架构、软件架构、接口设计和数据流设计。不包括具体的代码实现细节和测试用例，这些内容将在详细设计文档和测试用例文档中描述。

### 3.3 参考文档

- [ESP32_E-Paper_Weather_Display_PRD.md](ESP32_E-Paper_Weather_Display_PRD.md) - 产品需求文档
- [ESP32_E-Paper_Weather_Display_Architecture.md](ESP32_E-Paper_Weather_Display_Architecture.md) - 系统架构文档
- [ESP32_E-Paper_Weather_Display_Flowchart.md](ESP32_E-Paper_Weather_Display_Flowchart.md) - 系统流程图文档
- [调试文档.md](调试文档.md) - 调试和问题解决文档

## 4. 系统概述

### 4.1 系统目标

开发一款低功耗、长续航的智能家居天气显示设备，通过WiFi连接互联网获取实时天气数据，并在电子墨水屏上清晰展示。同时，通过板载传感器监测室内环境参数，为用户提供全面的天气信息服务。

### 4.2 系统上下文

ESP32电子墨水屏天气显示器作为一个独立的物联网设备，通过WiFi连接互联网，从OpenWeatherMap API获取天气数据，并通过电子墨水屏显示给用户。系统还通过BME280/BME680传感器获取室内环境数据，并通过NTP服务器同步时间。

### 4.3 设计约束

- **硬件约束**：使用ESP32微控制器、7.5英寸电子墨水屏和BME280/BME680传感器
- **功耗约束**：系统需要实现超低功耗，以延长电池续航时间
- **显示约束**：电子墨水屏刷新速度较慢，需要优化显示更新策略
- **网络约束**：依赖WiFi连接和互联网访问，需要处理网络不稳定的情况
- **API约束**：使用OpenWeatherMap免费API，有调用频率限制

## 5. 系统架构

### 5.1 整体架构

系统采用分层架构，从底层到顶层依次为：

```
+-------------------------------------+
|            功能层                    |
+-------------------------------------+
| 天气数据获取 | 时间同步 | 低功耗管理 | 错误处理 |
+-------------------------------------+
              |
+-------------------------------------+
|            应用层                    |
+-------------------------------------+
| 配置管理 | API通信 | 数据处理 | 显示渲染 |
+-------------------------------------+
              |
+-------------------------------------+
|            驱动层                    |
+-------------------------------------+
| GxEPD2库 | WiFi库 | 传感器驱动 | 电源管理 |
+-------------------------------------+
              |
+-------------------------------------+
|            硬件层                    |
+-------------------------------------+
| ESP32微控制器 | 电子墨水屏 | BME280传感器 |
+-------------------------------------+
```

### 5.2 硬件架构

#### 5.2.1 主要硬件组件

- **ESP32微控制器**：FireBeetle 2 ESP32-E，负责系统控制、网络通信和数据处理
- **电子墨水显示屏**：7.5英寸800×480像素电子墨水屏，负责信息显示
- **环境传感器**：BME280/BME680，负责室内环境数据采集
- **电源管理**：3.7V锂电池和充电电路，提供系统电源

#### 5.2.2 硬件连接图

ESP32与外设的连接如下：

- **ESP32与电子墨水屏连接**
  - BUSY: GPIO25
  - CS: GPIO15
  - RST: GPIO26
  - DC: GPIO27
  - SCK: GPIO13
  - MOSI: GPIO14
  - MISO: GPIO12
  - PWR: GPIO26

- **ESP32与BME280/BME680连接**
  - SDA: GPIO17
  - SCL: GPIO16
  - PWR: GPIO4

- **ESP32与LED指示灯连接**
  - LED1: GPIO2

- **ESP32与电池连接**
  - BAT_ADC: A2

### 5.3 软件架构

#### 5.3.1 软件模块

1. **配置模块**
   - 负责系统参数配置和管理
   - 主要文件：config.h/config.cpp

2. **API通信模块**
   - 负责与OpenWeatherMap API和NTP服务器通信
   - 主要文件：client_utils.cpp

3. **数据处理模块**
   - 负责解析API返回的JSON数据和传感器数据
   - 主要文件：api_response.cpp

4. **显示渲染模块**
   - 负责在电子墨水屏上渲染界面
   - 主要文件：renderer.cpp, display_utils.cpp

5. **电源管理模块**
   - 负责系统低功耗管理和电池监控
   - 主要文件：main.cpp

6. **错误处理模块**
   - 负责处理各种错误情况并显示错误信息
   - 主要文件：main.cpp, display_utils.cpp

#### 5.3.2 软件依赖

- **GxEPD2**：电子墨水屏驱动库
- **ArduinoJson**：JSON解析库
- **WiFi/HTTPClient**：网络通信库
- **Adafruit_Sensor/Adafruit_BME280**：传感器驱动库
- **Time**：时间管理库

## 6. 接口设计

### 6.1 外部接口

#### 6.1.1 OpenWeatherMap API接口

- **Current Weather API**
  - 端点：`/data/2.5/weather`
  - 参数：lat, lon, units, lang, appid
  - 返回：当前天气数据（JSON格式）

- **Forecast API**
  - 端点：`/data/2.5/forecast`
  - 参数：lat, lon, units, lang, cnt, appid
  - 返回：5天天气预报数据（JSON格式）

- **Air Pollution API**
  - 端点：`/data/2.5/air_pollution/history`
  - 参数：lat, lon, start, end, appid
  - 返回：空气质量数据（JSON格式）

#### 6.1.2 NTP服务器接口

- 使用标准NTP协议
- 服务器：pool.ntp.org, time.nist.gov

### 6.2 内部接口

#### 6.2.1 模块间接口

- **配置模块 → API通信模块**
  - 提供API密钥、坐标、语言等配置参数

- **API通信模块 → 数据处理模块**
  - 提供API响应数据

- **数据处理模块 → 显示渲染模块**
  - 提供解析后的天气数据和传感器数据

- **电源管理模块 → 所有模块**
  - 控制模块的电源状态和工作模式

#### 6.2.2 硬件接口

- **ESP32 → 电子墨水屏**：SPI接口
- **ESP32 → BME280/BME680**：I2C接口
- **ESP32 → LED指示灯**：GPIO接口
- **ESP32 → 电池**：ADC接口

## 7. 数据设计

### 7.1 数据结构

#### 7.1.1 天气数据结构

```cpp
// 当前天气数据结构
typedef struct {
  int64_t dt;
  float temp;
  float feels_like;
  int pressure;
  int humidity;
  int visibility;
  float uvi;
  int clouds;
  float wind_speed;
  int wind_deg;
  float wind_gust;
  float rain_1h;
  float snow_1h;
  int64_t sunrise;
  int64_t sunset;
  owm_weather_t weather;
} owm_current_t;

// 天气预报数据结构
typedef struct {
  int64_t dt;
  int64_t sunrise;
  int64_t sunset;
  int64_t moonrise;
  int64_t moonset;
  float moon_phase;
  struct {
    float day;
    float min;
    float max;
    float night;
    float eve;
    float morn;
  } temp;
  struct {
    float day;
    float night;
    float eve;
    float morn;
  } feels_like;
  int pressure;
  int humidity;
  float dew_point;
  float uvi;
  int clouds;
  int visibility;
  float wind_speed;
  int wind_deg;
  float wind_gust;
  float pop;
  float rain;
  float snow;
  owm_weather_t weather;
} owm_daily_t;
```

#### 7.1.2 空气质量数据结构

```cpp
// 空气质量数据结构
typedef struct {
  struct {
    float lat;
    float lon;
  } coord;
  int main_aqi[OWM_NUM_AIR_POLLUTION];
  struct {
    float co[OWM_NUM_AIR_POLLUTION];
    float no[OWM_NUM_AIR_POLLUTION];
    float no2[OWM_NUM_AIR_POLLUTION];
    float o3[OWM_NUM_AIR_POLLUTION];
    float so2[OWM_NUM_AIR_POLLUTION];
    float pm2_5[OWM_NUM_AIR_POLLUTION];
    float pm10[OWM_NUM_AIR_POLLUTION];
    float nh3[OWM_NUM_AIR_POLLUTION];
  } components;
  int64_t dt[OWM_NUM_AIR_POLLUTION];
} owm_resp_air_pollution_t;
```

### 7.2 数据流

1. **API数据获取流**
   - ESP32通过WiFi连接OpenWeatherMap API
   - 获取天气数据和空气质量数据
   - 解析JSON响应数据
   - 存储到相应的数据结构中

2. **传感器数据获取流**
   - ESP32通过I2C接口读取BME280/BME680传感器
   - 获取室内温度、湿度和气压数据
   - 存储到相应的变量中

3. **显示数据流**
   - 从数据结构中读取天气数据和传感器数据
   - 格式化数据以适应显示需求
   - 渲染到电子墨水屏上

## 8. 低功耗设计

### 8.1 睡眠模式

- **深度睡眠模式**：ESP32进入深度睡眠状态，仅保留RTC功能，功耗约14μA
- **唤醒机制**：使用RTC定时器唤醒，定时更新天气数据
- **睡眠时间计算**：根据配置的更新频率和当前时间计算下次唤醒时间
- **内存管理**：在进入深度睡眠前进行内存清理，避免内存泄漏

### 8.2 电源管理策略

- **夜间节电模式**：在配置的夜间时段减少更新频率
- **低电量策略**：根据电池电量调整更新频率，延长电池寿命
- **外设电源控制**：仅在需要时为传感器和显示屏供电
- **LED指示灯控制**：使用低电平点亮模式，减少功耗

## 9. 错误处理策略

### 9.1 错误类型

- **WiFi连接错误**：无法连接WiFi网络
- **API错误**：API请求失败或返回错误
- **时间同步错误**：无法从NTP服务器获取时间
- **传感器错误**：无法读取传感器数据
- **低电量错误**：电池电量过低

### 9.2 错误处理机制

- **错误显示**：在电子墨水屏上显示错误信息和图标
- **错误恢复**：定期尝试重新连接和请求
- **降级服务**：在部分功能不可用时，继续提供可用的功能

## 10. 安全考虑

### 10.1 数据安全

- **API密钥保护**：API密钥存储在单独的secrets.h文件中，不包含在版本控制中
- **HTTPS支持**：可选择使用HTTPS进行API通信，提高数据传输安全性

### 10.2 硬件安全

- **电池保护**：实现低电量保护机制，防止电池过度放电
- **硬件接口保护**：确保硬件接口连接正确，防止损坏组件

## 11. 性能考虑

### 11.1 响应时间

- **唤醒到显示时间**：约15秒（包括WiFi连接、API请求和显示刷新）
- **显示刷新时间**：约2-3秒（电子墨水屏刷新时间）

### 11.2 内存使用

- **RAM使用**：优化内存使用，确保不超过ESP32的520KB SRAM
- **Flash使用**：优化代码和资源，确保不超过ESP32的4MB Flash

### 11.3 电池续航

- **预期续航时间**：6-12个月（使用5000mAh电池，30分钟更新频率）
- **功耗指标**：睡眠模式约14μA，刷新时约83mA

## 12. 可扩展性设计

### 12.1 硬件扩展

- **支持不同型号的电子墨水屏**：通过配置参数适配不同分辨率和颜色的显示屏
- **支持不同型号的传感器**：通过条件编译支持BME280和BME680

### 12.2 软件扩展

- **支持更多API**：预留接口以支持其他天气API
- **支持更多功能**：预留接口以支持未来功能扩展

## 13. 部署考虑

### 13.1 开发环境

- **IDE**：Visual Studio Code + PlatformIO
- **编程语言**：C++
- **框架**：Arduino框架

### 13.2 编译和上传

- **编译工具**：PlatformIO
- **上传方式**：通过USB连接上传到ESP32

### 13.3 调试方法

- **串口监视器**：通过串口输出调试信息
- **调试级别**：通过DEBUG_LEVEL配置调试信息详细程度

## 14. 总结

本概要设计文档描述了ESP32电子墨水屏天气显示器的系统架构、主要组件、接口设计和数据流。系统采用分层架构，通过WiFi连接互联网获取天气数据，并在电子墨水屏上显示。系统实现了低功耗设计，通过深度睡眠模式延长电池续航时间。系统还实现了完善的错误处理机制，确保在各种异常情况下能够正常工作或提供降级服务。

---

*文档版本：1.0*  
*最后更新：2025年7月22日*