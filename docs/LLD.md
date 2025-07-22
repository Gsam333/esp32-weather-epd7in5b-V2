# ESP32 电子墨水屏天气显示器 - 详细设计文档 (LLD)

## 1. 文档信息

| 项目 | 内容 |
|------|------|
| 文档标题 | ESP32 电子墨水屏天气显示器详细设计文档 |
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

本文档旨在提供ESP32电子墨水屏天气显示器项目的详细设计，包括模块设计、类设计、函数设计和数据结构设计。本文档面向项目开发人员，作为系统实现的直接指导文档。

### 3.2 范围

本文档涵盖ESP32电子墨水屏天气显示器的详细设计，包括各个模块的内部设计、函数接口、数据结构和算法设计。本文档基于概要设计文档，提供更详细的设计信息。

### 3.3 参考文档

- [ESP32_E-Paper_Weather_Display_PRD.md](ESP32_E-Paper_Weather_Display_PRD.md) - 产品需求文档
- [ESP32_E-Paper_Weather_Display_HLD.md](ESP32_E-Paper_Weather_Display_HLD.md) - 概要设计文档
- [ESP32_E-Paper_Weather_Display_Architecture.md](ESP32_E-Paper_Weather_Display_Architecture.md) - 系统架构文档
- [ESP32_E-Paper_Weather_Display_Flowchart.md](ESP32_E-Paper_Weather_Display_Flowchart.md) - 系统流程图文档
- [调试文档.md](调试文档.md) - 调试和问题解决文档## 4.
 模块详细设计

### 4.1 配置模块 (config.h/config.cpp)

#### 4.1.1 模块职责

配置模块负责定义和管理系统的各种配置参数，包括硬件引脚定义、网络设置、API设置、时间设置、显示设置和电源管理设置等。

#### 4.1.2 主要常量和变量

**硬件引脚定义**
```cpp
// LED引脚
const uint8_t PIN_LED1 = 2;                // IO2 pin for LED1

// 电池ADC引脚
const uint8_t PIN_BAT_ADC = A2;            // A0 for micro-usb firebeetle

// 电子墨水屏引脚
const uint8_t PIN_EPD_BUSY = 25;           // EPD_BUSY_PIN 25
const uint8_t PIN_EPD_CS = 15;             // EPD_CS_PIN 15
const uint8_t PIN_EPD_RST = 26;            // EPD_RST_PIN 26
const uint8_t PIN_EPD_DC = 27;             // EPD_DC_PIN 27
const uint8_t PIN_EPD_SCK = 13;            // EPD_SCK_PIN 13
const uint8_t PIN_EPD_MISO = 12;           // 未在官方示例中使用
const uint8_t PIN_EPD_MOSI = 14;           // EPD_MOSI_PIN 14
const uint8_t PIN_EPD_PWR = 26;            // 与RST共用

// BME280传感器引脚
const uint8_t PIN_BME_SDA = 17;
const uint8_t PIN_BME_SCL = 16;
const uint8_t PIN_BME_PWR = 4;
const uint8_t BME_ADDRESS = 0x76;          // 0x76 if SDO -> GND; 0x77 if SDO -> VCC
```

**网络设置**
```cpp
// WiFi设置
const char *WIFI_SSID = SECRET_WIFI_SSID;
const char *WIFI_PASSWORD = SECRET_WIFI_PASSWORD;
const unsigned long WIFI_TIMEOUT = 10000;  // ms, WiFi连接超时时间

// HTTP设置
const unsigned HTTP_CLIENT_TCP_TIMEOUT = 30000;  // ms, HTTP客户端TCP超时时间
```

**API设置**
```cpp
// OpenWeatherMap API设置
const String OWM_APIKEY = SECRET_OWM_APIKEY;
const String OWM_ENDPOINT = "api.openweathermap.org";
const String OWM_ONECALL_VERSION = "2.5";
```

**位置设置**
```cpp
// 位置设置
const String LAT = "31.2304";              // 纬度
const String LON = "121.4737";             // 经度
const String CITY_STRING = "Shanghai";     // 城市名称
```

**时间设置**
```cpp
// 时区设置
const char *TIMEZONE = "CST-8";            // 中国标准时间 (UTC+8)

// 时间格式设置
const char *TIME_FORMAT = "%H:%M";         // 24小时制
const char *HOUR_FORMAT = "%H";            // 24小时制
const char *DATE_FORMAT = "%a, %B %e";     // 例如: Sat, January 1
const char *REFRESH_TIME_FORMAT = "%x %H:%M";

// NTP服务器设置
const char *NTP_SERVER_1 = "pool.ntp.org";
const char *NTP_SERVER_2 = "time.nist.gov";
const unsigned long NTP_TIMEOUT = 20000;   // ms, NTP超时时间
```

**睡眠设置**
```cpp
// 睡眠设置
const int SLEEP_DURATION = 30;             // 分钟, 睡眠时间
const int BED_TIME = 00;                   // 夜间模式开始时间 (00:00)
const int WAKE_TIME = 06;                  // 夜间模式结束时间 (06:00)
```

**图表设置**
```cpp
// 图表设置
const int HOURLY_GRAPH_MAX = 24;           // 小时预报图表显示的小时数
```

**电池设置**
```cpp
// 电池设置
const uint32_t WARN_BATTERY_VOLTAGE = 3535;                // 警告电压 (毫伏) ~20%
const uint32_t LOW_BATTERY_VOLTAGE = 3462;                 // 低电压 (毫伏) ~10%
const uint32_t VERY_LOW_BATTERY_VOLTAGE = 3442;            // 很低电压 (毫伏) ~8%
const uint32_t CRIT_LOW_BATTERY_VOLTAGE = 3404;            // 临界低电压 (毫伏) ~5%
const unsigned long LOW_BATTERY_SLEEP_INTERVAL = 30;       // 低电量睡眠间隔 (分钟)
const unsigned long VERY_LOW_BATTERY_SLEEP_INTERVAL = 120; // 很低电量睡眠间隔 (分钟)
const uint32_t MAX_BATTERY_VOLTAGE = 4200;                 // 最大电池电压 (毫伏)
const uint32_t MIN_BATTERY_VOLTAGE = 3000;                 // 最小电池电压 (毫伏)
```

#### 4.1.3 配置宏定义

```cpp
// 调试级别
#define DEBUG_LEVEL 1                      // 0: 基本信息, 1: 详细信息, 2: API响应

// 调试模式
#define DEBUG_MODE_SKIP_HARDWARE 1         // 1: 跳过硬件检查, 0: 正常硬件检查

// 显示屏类型
#define DISP_3C_B                          // 三色显示屏 (红/黑/白)

// 温度单位
#define UNITS_TEMP_CELSIUS                 // 摄氏度

// 传感器类型
#define SENSOR_BME280                      // BME280传感器

// 电池监控
#define BATTERY_MONITORING 1               // 1: 启用电池监控, 0: 禁用电池监控

// 显示警报
#define DISPLAY_ALERTS 1                   // 1: 显示天气警报, 0: 不显示天气警报
```

### 4.2 API通信模块 (client_utils.cpp)

#### 4.2.1 模块职责

API通信模块负责与OpenWeatherMap API和NTP服务器通信，获取天气数据、空气质量数据和时间数据。

#### 4.2.2 主要函数

**WiFi连接函数**
```cpp
/**
 * 启动WiFi并连接到配置的网络
 * @param wifiRSSI 用于存储WiFi信号强度的引用
 * @return WiFi连接状态
 */
wl_status_t startWiFi(int &wifiRSSI);

/**
 * 断开WiFi连接并关闭WiFi
 */
void killWiFi();
```

**时间同步函数**
```cpp
/**
 * 打印本地时间到串口
 * @param timeInfo 时间信息结构体指针
 * @return 获取时间是否成功
 */
bool printLocalTime(tm *timeInfo);

/**
 * 等待SNTP同步完成
 * @param timeInfo 时间信息结构体指针
 * @return 同步是否成功
 */
bool waitForSNTPSync(tm *timeInfo);
```

**API请求函数**
```cpp
/**
 * 获取当前天气数据
 * @param client WiFi客户端
 * @param current 用于存储当前天气数据的结构体
 * @return HTTP状态码
 */
int getOWMcurrentWeather(WiFiClient &client, owm_current_t &current);

/**
 * 获取天气预报数据
 * @param client WiFi客户端
 * @param r 用于存储天气预报数据的结构体
 * @return HTTP状态码
 */
int getOWMonecall(WiFiClient &client, owm_resp_onecall_t &r);

/**
 * 获取空气质量数据
 * @param client WiFi客户端
 * @param r 用于存储空气质量数据的结构体
 * @return HTTP状态码
 */
int getOWMairpollution(WiFiClient &client, owm_resp_air_pollution_t &r);
```

**调试函数**
```cpp
/**
 * 打印堆内存使用情况
 */
void printHeapUsage();
```

#### 4.2.3 工作流程

1. 调用`startWiFi`函数连接WiFi
2. 调用`waitForSNTPSync`函数同步时间
3. 调用`getOWMcurrentWeather`函数获取当前天气数据
4. 调用`getOWMonecall`函数获取天气预报数据
5. 调用`getOWMairpollution`函数获取空气质量数据
6. 调用`killWiFi`函数断开WiFi连接

### 4.3 数据处理模块 (api_response.cpp)

#### 4.3.1 模块职责

数据处理模块负责解析API返回的JSON数据，将其转换为系统内部使用的数据结构。

#### 4.3.2 主要函数

**JSON解析函数**
```cpp
/**
 * 解析天气预报API响应
 * @param json API响应JSON数据
 * @param r 用于存储解析结果的结构体
 * @return 解析错误码
 */
DeserializationError deserializeOneCall(WiFiClient &json, owm_resp_onecall_t &r);

/**
 * 解析空气质量API响应
 * @param json API响应JSON数据
 * @param r 用于存储解析结果的结构体
 * @return 解析错误码
 */
DeserializationError deserializeAirQuality(WiFiClient &json, owm_resp_air_pollution_t &r);
```

#### 4.3.3 数据结构

**天气数据结构**
```cpp
// 天气描述结构体
typedef struct {
  int id;                  // 天气ID
  const char *main;        // 主要天气描述
  const char *description; // 详细天气描述
  const char *icon;        // 天气图标代码
} owm_weather_t;

// 当前天气结构体
typedef struct {
  int64_t dt;              // 数据时间戳
  float temp;              // 温度
  float feels_like;        // 体感温度
  int pressure;            // 气压
  int humidity;            // 湿度
  int visibility;          // 能见度
  float uvi;               // 紫外线指数
  int clouds;              // 云量
  float wind_speed;        // 风速
  int wind_deg;            // 风向
  float wind_gust;         // 阵风风速
  float rain_1h;           // 1小时降雨量
  float snow_1h;           // 1小时降雪量
  int64_t sunrise;         // 日出时间
  int64_t sunset;          // 日落时间
  owm_weather_t weather;   // 天气描述
} owm_current_t;

// 小时预报结构体
typedef struct {
  int64_t dt;              // 数据时间戳
  float temp;              // 温度
  float feels_like;        // 体感温度
  int pressure;            // 气压
  int humidity;            // 湿度
  float dew_point;         // 露点
  float uvi;               // 紫外线指数
  int clouds;              // 云量
  int visibility;          // 能见度
  float wind_speed;        // 风速
  int wind_deg;            // 风向
  float wind_gust;         // 阵风风速
  float pop;               // 降水概率
  float rain_1h;           // 1小时降雨量
  float snow_1h;           // 1小时降雪量
  owm_weather_t weather;   // 天气描述
} owm_hourly_t;

// 天气预报响应结构体
typedef struct {
  float lat;                                // 纬度
  float lon;                                // 经度
  const char *timezone;                     // 时区
  int timezone_offset;                      // 时区偏移
  owm_current_t current;                    // 当前天气
  owm_hourly_t hourly[OWM_NUM_HOURLY];      // 小时预报
  owm_daily_t daily[OWM_NUM_DAILY];         // 每日预报
  owm_alert_t alerts[OWM_NUM_ALERTS];       // 天气警报
  int alerts_cnt;                           // 天气警报数量
} owm_resp_onecall_t;
```

**空气质量数据结构**
```cpp
// 空气质量响应结构体
typedef struct {
  struct {
    float lat;                              // 纬度
    float lon;                              // 经度
  } coord;
  int main_aqi[OWM_NUM_AIR_POLLUTION];      // 空气质量指数
  struct {
    float co[OWM_NUM_AIR_POLLUTION];        // 一氧化碳浓度
    float no[OWM_NUM_AIR_POLLUTION];        // 一氧化氮浓度
    float no2[OWM_NUM_AIR_POLLUTION];       // 二氧化氮浓度
    float o3[OWM_NUM_AIR_POLLUTION];        // 臭氧浓度
    float so2[OWM_NUM_AIR_POLLUTION];       // 二氧化硫浓度
    float pm2_5[OWM_NUM_AIR_POLLUTION];     // PM2.5浓度
    float pm10[OWM_NUM_AIR_POLLUTION];      // PM10浓度
    float nh3[OWM_NUM_AIR_POLLUTION];       // 氨浓度
  } components;
  int64_t dt[OWM_NUM_AIR_POLLUTION];        // 数据时间戳
} owm_resp_air_pollution_t;
```

#### 4.3.4 解析算法

**天气预报解析算法**
1. 创建JSON文档对象，大小为128KB
2. 使用ArduinoJson库解析JSON数据
3. 检查解析是否成功
4. 提取城市信息（坐标、时区等）
5. 提取当前天气数据
6. 提取小时预报数据
7. 从3小时预报数据中提取每日预报数据
8. 返回解析结果

**空气质量解析算法**
1. 创建JSON文档对象，大小为32KB
2. 使用ArduinoJson库解析JSON数据
3. 检查解析是否成功
4. 提取坐标信息
5. 提取空气质量指数和污染物浓度数据
6. 返回解析结果#
## 4.4 显示渲染模块 (renderer.cpp, display_utils.cpp)

#### 4.4.1 模块职责

显示渲染模块负责在电子墨水屏上渲染界面，包括当前天气、天气预报、图表和状态信息等。

#### 4.4.2 主要函数

**显示初始化函数**
```cpp
/**
 * 初始化电子墨水显示屏
 */
void initDisplay();

/**
 * 关闭电子墨水显示屏电源
 */
void powerOffDisplay();
```

**渲染函数**
```cpp
/**
 * 渲染当前天气条件
 * @param current 当前天气数据
 * @param daily 每日预报数据
 * @param air_pollution 空气质量数据
 * @param inTemp 室内温度
 * @param inHumidity 室内湿度
 */
void drawCurrentConditions(const owm_current_t &current, const owm_daily_t &daily,
                          const owm_resp_air_pollution_t &air_pollution,
                          float inTemp, float inHumidity);

/**
 * 渲染天气预报图表
 * @param hourly 小时预报数据
 * @param daily 每日预报数据
 * @param timeInfo 时间信息
 */
void drawOutlookGraph(const owm_hourly_t hourly[], const owm_daily_t daily[],
                     tm timeInfo);

/**
 * 渲染每日天气预报
 * @param daily 每日预报数据
 * @param timeInfo 时间信息
 */
void drawForecast(const owm_daily_t daily[], tm timeInfo);

/**
 * 渲染位置和日期信息
 * @param city 城市名称
 * @param date 日期字符串
 */
void drawLocationDate(const String &city, const String &date);

/**
 * 渲染天气警报信息
 * @param alerts 天气警报数据
 * @param city 城市名称
 * @param date 日期字符串
 */
void drawAlerts(const owm_alert_t alerts[], const String &city, const String &date);

/**
 * 渲染状态栏
 * @param statusStr 状态字符串
 * @param refreshTimeStr 刷新时间字符串
 * @param wifiRSSI WiFi信号强度
 * @param batteryVoltage 电池电压
 */
void drawStatusBar(const String &statusStr, const String &refreshTimeStr,
                  int wifiRSSI, uint32_t batteryVoltage);

/**
 * 渲染错误信息
 * @param icon 错误图标
 * @param line1 错误信息第一行
 * @param line2 错误信息第二行
 */
void drawError(const unsigned char *icon, const String &line1, const String &line2 = "");
```

**辅助函数**
```cpp
/**
 * 获取刷新时间字符串
 * @param refreshTimeStr 用于存储刷新时间字符串的引用
 * @param timeConfigured 时间是否已配置
 * @param timeInfo 时间信息
 */
void getRefreshTimeStr(String &refreshTimeStr, bool timeConfigured, tm *timeInfo);

/**
 * 获取日期字符串
 * @param dateStr 用于存储日期字符串的引用
 * @param timeInfo 时间信息
 */
void getDateStr(String &dateStr, tm *timeInfo);

/**
 * 获取HTTP响应短语
 * @param code HTTP状态码
 * @return HTTP响应短语
 */
const char *getHttpResponsePhrase(int code);

/**
 * 获取WiFi状态短语
 * @param status WiFi状态
 * @return WiFi状态短语
 */
const char *getWifiStatusPhrase(wl_status_t status);

/**
 * 禁用内置LED
 */
void disableBuiltinLED();
```

#### 4.4.3 渲染算法

**当前天气渲染算法**
1. 渲染天气图标
2. 渲染当前温度和体感温度
3. 渲染日出日落时间
4. 渲染风速和风向
5. 渲染湿度和气压
6. 渲染紫外线指数和能见度
7. 渲染空气质量指数
8. 渲染室内温度和湿度

**天气预报图表渲染算法**
1. 计算图表区域和比例
2. 绘制坐标轴和网格
3. 绘制温度曲线
4. 绘制降水概率柱状图
5. 绘制天气图标
6. 绘制时间标签

**每日天气预报渲染算法**
1. 计算每日预报区域
2. 循环渲染每天的预报
   - 渲染星期几
   - 渲染天气图标
   - 渲染最高温度和最低温度

**错误信息渲染算法**
1. 清空显示屏
2. 渲染错误图标
3. 渲染错误信息文本
4. 渲染错误代码和描述（如果有）

### 4.5 电源管理模块 (main.cpp)

#### 4.5.1 模块职责

电源管理模块负责系统的低功耗管理和电池监控，包括深度睡眠模式、唤醒机制和电池电量检测。

#### 4.5.2 主要函数

**深度睡眠函数**
```cpp
/**
 * 使ESP32进入深度睡眠模式
 * @param startTime 系统启动时间
 * @param timeInfo 时间信息
 */
void beginDeepSleep(unsigned long startTime, tm *timeInfo);
```

**电池电量检测函数**
```cpp
/**
 * 读取电池电压
 * @return 电池电压 (毫伏)
 */
uint32_t readBatteryVoltage();
```

**LED控制函数**
```cpp
/**
 * LED1闪烁函数
 */
void blinkLED1();
```

#### 4.5.3 深度睡眠算法

1. 获取当前时间
2. 计算相对于WAKE_TIME的时间
3. 计算对齐到SLEEP_DURATION的时间
4. 检查预计唤醒时间是否在睡眠时段
   - 如果是，调整睡眠时间到下一个WAKE_TIME
   - 如果否，使用标准SLEEP_DURATION
5. 添加额外延迟补偿
6. 设置定时器唤醒
7. 进入深度睡眠模式

#### 4.5.4 电池电量检测算法

1. 配置ADC通道
2. 读取ADC值
3. 将ADC值转换为电压值
4. 应用校准因子
5. 返回电池电压

### 4.6 主程序模块 (main.cpp)

#### 4.6.1 模块职责

主程序模块是系统的入口点，负责协调各个模块的工作，实现系统的主要功能。

#### 4.6.2 主要函数

**主函数**
```cpp
/**
 * 程序入口点
 */
void setup();

/**
 * 主循环函数 (在本项目中不使用，因为系统使用深度睡眠模式)
 */
void loop();
```

#### 4.6.3 主程序流程

1. 初始化系统
   - 初始化串口通信
   - 初始化LED1
   - 禁用内置LED
   - 打开非易失性存储

2. 检查电池电量
   - 如果电池电量低，显示低电量警告并进入睡眠模式
   - 如果电池电量正常，继续执行

3. 连接WiFi
   - 如果连接失败，显示WiFi错误并进入睡眠模式
   - 如果连接成功，继续执行

4. 同步时间
   - 如果同步失败，显示时间错误并进入睡眠模式
   - 如果同步成功，继续执行

5. 获取天气数据
   - 获取当前天气数据
   - 获取天气预报数据
   - 获取空气质量数据
   - 如果任何API请求失败，显示API错误并进入睡眠模式

6. 获取传感器数据
   - 读取室内温度和湿度

7. 渲染显示内容
   - 初始化显示屏
   - 渲染当前天气条件
   - 渲染天气预报图表
   - 渲染每日天气预报
   - 渲染位置和日期信息
   - 渲染天气警报信息（如果有）
   - 渲染状态栏
   - 关闭显示屏电源

8. 进入深度睡眠模式
   - 计算下次唤醒时间
   - 设置定时器唤醒
   - 进入深度睡眠模式

## 5. 算法详细设计

### 5.1 深度睡眠时间计算算法

```cpp
void beginDeepSleep(unsigned long startTime, tm *timeInfo) {
  // 获取当前时间
  if (!getLocalTime(timeInfo)) {
    Serial.println(TXT_REFERENCING_OLDER_TIME_NOTICE);
  }

  // 计算相对于WAKE_TIME的时间
  int bedtimeHour = INT_MAX;
  if (BED_TIME != WAKE_TIME) {
    bedtimeHour = (BED_TIME - WAKE_TIME + 24) % 24;
  }

  int curHour = (timeInfo->tm_hour - WAKE_TIME + 24) % 24;
  const int curMinute = curHour * 60 + timeInfo->tm_min;
  const int curSecond = curHour * 3600 + timeInfo->tm_min * 60 + timeInfo->tm_sec;
  const int desiredSleepSeconds = SLEEP_DURATION * 60;
  const int offsetMinutes = curMinute % SLEEP_DURATION;
  const int offsetSeconds = curSecond % desiredSleepSeconds;

  // 计算对齐到SLEEP_DURATION的时间
  int sleepMinutes = SLEEP_DURATION - offsetMinutes;
  if (desiredSleepSeconds - offsetSeconds < 120 ||
      offsetSeconds / (float)desiredSleepSeconds > 0.95f) {
    sleepMinutes += SLEEP_DURATION;
  }

  // 检查预计唤醒时间是否在睡眠时段
  const int predictedWakeHour = ((curMinute + sleepMinutes) / 60) % 24;

  uint64_t sleepDuration;
  if (predictedWakeHour < bedtimeHour) {
    sleepDuration = sleepMinutes * 60 - timeInfo->tm_sec;
  } else {
    const int hoursUntilWake = 24 - curHour;
    sleepDuration = hoursUntilWake * 3600ULL - (timeInfo->tm_min * 60ULL + timeInfo->tm_sec);
  }

  // 添加额外延迟补偿
  sleepDuration += 3ULL;
  sleepDuration *= 1.0015f;

  // 强制进行垃圾回收，释放未使用的内存
  ESP.getFreeHeap();

  // 设置定时器唤醒
  esp_sleep_enable_timer_wakeup(sleepDuration * 1000000ULL);
  
  // 进入深度睡眠模式
  esp_deep_sleep_start();
}
```

### 5.2 天气图标选择算法

```cpp
const unsigned char* getWeatherIcon(const owm_weather_t &weather, bool isDay) {
  // 获取天气ID
  int id = weather.id;
  
  // 获取天气图标代码
  const char *icon = weather.icon;
  
  // 判断是白天还是夜晚
  bool day = isDay;
  if (icon != nullptr && strlen(icon) >= 3) {
    day = (icon[2] == 'd');
  }
  
  // 根据天气ID和白天/夜晚选择合适的图标
  if (id >= 200 && id < 300) {
    // 雷暴
    return day ? wi_day_thunderstorm_196x196 : wi_night_thunderstorm_196x196;
  } else if (id >= 300 && id < 400) {
    // 毛毛雨
    return day ? wi_day_sprinkle_196x196 : wi_night_sprinkle_196x196;
  } else if (id >= 500 && id < 600) {
    // 雨
    return day ? wi_day_rain_196x196 : wi_night_rain_196x196;
  } else if (id >= 600 && id < 700) {
    // 雪
    return day ? wi_day_snow_196x196 : wi_night_snow_196x196;
  } else if (id >= 700 && id < 800) {
    // 雾霾
    return day ? wi_day_fog_196x196 : wi_night_fog_196x196;
  } else if (id == 800) {
    // 晴
    return day ? wi_day_sunny_196x196 : wi_night_clear_196x196;
  } else if (id > 800 && id < 900) {
    // 多云
    return day ? wi_day_cloudy_196x196 : wi_night_cloudy_196x196;
  } else {
    // 未知天气
    return wi_na_196x196;
  }
}
```

### 5.3 温度单位转换算法

```cpp
float convertTemperature(float kelvin) {
#ifdef UNITS_TEMP_KELVIN
  return kelvin;
#elif defined(UNITS_TEMP_CELSIUS)
  return kelvin - 273.15f;
#elif defined(UNITS_TEMP_FAHRENHEIT)
  return (kelvin - 273.15f) * 9.0f / 5.0f + 32.0f;
#endif
}
```

### 5.4 电池电量百分比计算算法

```cpp
int getBatteryPercentage(uint32_t voltage) {
  // 将电压限制在最小和最大范围内
  voltage = constrain(voltage, MIN_BATTERY_VOLTAGE, MAX_BATTERY_VOLTAGE);
  
  // 计算电池电量百分比
  return map(voltage, MIN_BATTERY_VOLTAGE, MAX_BATTERY_VOLTAGE, 0, 100);
}
```

## 6. 数据结构详细设计

### 6.1 天气数据结构

详见4.3.3节

### 6.2 配置数据结构

详见4.1.2节

### 6.3 显示缓冲区

```cpp
// 显示缓冲区
GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT> display(GxEPD2_750(PIN_EPD_CS, PIN_EPD_DC, PIN_EPD_RST, PIN_EPD_BUSY));
```

## 7. 接口详细设计

### 7.1 模块间接口

#### 7.1.1 配置模块 → API通信模块

配置模块通过全局常量向API通信模块提供配置参数：

```cpp
// WiFi配置
const char *WIFI_SSID;
const char *WIFI_PASSWORD;
const unsigned long WIFI_TIMEOUT;

// HTTP配置
const unsigned HTTP_CLIENT_TCP_TIMEOUT;

// API配置
const String OWM_APIKEY;
const String OWM_ENDPOINT;
const String LAT;
const String LON;
```

#### 7.1.2 API通信模块 → 数据处理模块

API通信模块通过函数参数向数据处理模块提供API响应数据：

```cpp
DeserializationError deserializeOneCall(WiFiClient &json, owm_resp_onecall_t &r);
DeserializationError deserializeAirQuality(WiFiClient &json, owm_resp_air_pollution_t &r);
```

#### 7.1.3 数据处理模块 → 显示渲染模块

数据处理模块通过全局变量向显示渲染模块提供解析后的数据：

```cpp
owm_resp_onecall_t owm_onecall;
owm_resp_air_pollution_t owm_air_pollution;
```

### 7.2 硬件接口

#### 7.2.1 ESP32 → 电子墨水屏

通过SPI接口连接：

```cpp
// SPI接口
const uint8_t PIN_EPD_BUSY = 25;
const uint8_t PIN_EPD_CS = 15;
const uint8_t PIN_EPD_RST = 26;
const uint8_t PIN_EPD_DC = 27;
const uint8_t PIN_EPD_SCK = 13;
const uint8_t PIN_EPD_MOSI = 14;
```

#### 7.2.2 ESP32 → BME280/BME680

通过I2C接口连接：

```cpp
// I2C接口
const uint8_t PIN_BME_SDA = 17;
const uint8_t PIN_BME_SCL = 16;
const uint8_t PIN_BME_PWR = 4;
const uint8_t BME_ADDRESS = 0x76;
```

## 8. 错误处理设计

### 8.1 错误类型

1. **WiFi连接错误**
   - 错误代码：WiFi状态码
   - 错误图标：wifi_x_196x196
   - 错误信息："WiFi Connection Failed" 或 "SSID Not Available"

2. **API错误**
   - 错误代码：HTTP状态码或自定义错误码
   - 错误图标：wi_cloud_down_196x196
   - 错误信息：HTTP错误描述或自定义错误描述

3. **时间同步错误**
   - 错误图标：wi_time_4_196x196
   - 错误信息："Time Synchronization Failed"

4. **低电量错误**
   - 错误图标：battery_alert_0deg_196x196
   - 错误信息："Low Battery"

### 8.2 错误处理流程

1. **检测错误**
   - 检查WiFi连接状态
   - 检查API响应状态码
   - 检查时间同步状态
   - 检查电池电量

2. **显示错误**
   - 初始化显示屏
   - 调用drawError函数显示错误信息
   - 关闭显示屏电源

3. **错误恢复**
   - 进入深度睡眠模式
   - 在下次唤醒时重试

## 9. 总结

本详细设计文档描述了ESP32电子墨水屏天气显示器的各个模块的详细设计，包括模块职责、主要函数、数据结构、算法设计和接口设计。系统采用模块化设计，各模块之间通过明确的接口进行通信，确保系统的可维护性和可扩展性。系统实现了低功耗设计，通过深度睡眠模式延长电池续航时间。系统还实现了完善的错误处理机制，确保在各种异常情况下能够正常工作或提供降级服务。

---

*文档版本：1.0*  
*最后更新：2025年7月22日*