# ESP32 电子墨水屏天气显示器 - 架构与流程分析

## 项目概述

ESP32 电子墨水屏天气显示器是一个低功耗的天气显示设备，使用支持WiFi的ESP32微控制器和7.5英寸电子墨水屏。天气数据从OpenWeatherMap API获取，板载传感器提供室内温度和湿度数据。

## 系统架构图

```
+-------------------------------------+
|            硬件层                    |
+-------------------------------------+
| ESP32微控制器 | 电子墨水屏 | BME280传感器 |
+-------------------------------------+
              |
+-------------------------------------+
|            驱动层                    |
+-------------------------------------+
| GxEPD2库 | WiFi库 | 传感器驱动 | 电源管理 |
+-------------------------------------+
              |
+-------------------------------------+
|            应用层                    |
+-------------------------------------+
| 配置管理 | API通信 | 数据处理 | 显示渲染 |
+-------------------------------------+
              |
+-------------------------------------+
|            功能层                    |
+-------------------------------------+
| 天气数据获取 | 时间同步 | 低功耗管理 | 错误处理 |
+-------------------------------------+
```

## 核心组件详解

### 1. 硬件组件

- **ESP32微控制器**：FireBeetle 2 ESP32-E，具有低功耗设计、USB-C接口和电池管理功能
- **电子墨水显示屏**：支持多种型号，主要为7.5英寸800x480像素黑白屏幕
- **适配板**：DESPI-C02
- **传感器**：BME280，用于测量室内温度、湿度和气压
- **电池**：3.7V锂电池，JST-PH2.0接口

### 2. 软件模块

#### 配置模块 (config.h/config.cpp)
- 定义系统参数、API密钥、WiFi凭据
- 设置显示选项、单位选择、时间格式
- 配置低功耗参数和睡眠时间

#### API通信模块 (client_utils.cpp)
- 处理与OpenWeatherMap API的HTTP/HTTPS通信
- 获取天气数据和空气质量数据

#### 数据处理模块 (api_response.cpp)
- 解析API返回的JSON数据
- 存储天气预报、当前天气和空气质量信息

#### 显示渲染模块 (renderer.cpp)
- 负责在电子墨水屏上绘制界面元素
- 显示当前天气、预报、图表和状态信息

#### 电源管理模块 (main.cpp)
- 实现深度睡眠功能
- 电池电量监控和低电量处理
- 优化功耗以延长电池寿命

## 系统流程图

```
+----------------+     +----------------+     +----------------+
|  系统启动/唤醒  | --> |  初始化硬件组件  | --> |  检查电池电量  |
+----------------+     +----------------+     +----------------+
                                                     |
+----------------+     +----------------+     +----------------+
| 进入深度睡眠模式 | <-- |   渲染显示内容   | <-- |  获取传感器数据 |
+----------------+     +----------------+     +----------------+
                                                     ^
                                                     |
                       +----------------+     +----------------+
                       |   解析API数据   | <-- |  连接WiFi网络  |
                       +----------------+     +----------------+
                             ^                       |
                             |                       v
                       +----------------+     +----------------+
                       |  处理错误情况   | <-- |  API数据请求   |
                       +----------------+     +----------------+
```

## 主要流程详解

### 1. 启动与初始化流程

1. 系统从深度睡眠中唤醒或上电启动
2. 初始化串口通信(115200波特率)
3. 禁用内置LED以节省电量
4. 打开非易失性存储空间(NVS)读取配置
5. 检查电池电量状态
   - 如果电池电量低，显示低电量警告并进入睡眠模式
   - 如果电池电量正常，继续执行

### 2. 网络连接与数据获取流程

1. 连接WiFi网络
   - 如果连接失败，显示错误信息并进入睡眠模式
2. 与NTP服务器同步时间
   - 如果同步失败，显示错误信息并进入睡眠模式
3. 向OpenWeatherMap API发送请求
   - 获取One Call API数据(当前天气和预报)
   - 获取空气质量数据
   - 如果API请求失败，显示错误信息并进入睡眠模式

### 3. 传感器数据获取流程

1. 通过I2C接口初始化BME280/BME680传感器
2. 读取室内温度和湿度数据
3. 关闭传感器电源以节省电量

### 4. 显示渲染流程

1. 初始化电子墨水显示屏
2. 渲染当前天气条件(温度、体感温度、图标)
3. 渲染天气预报图表(未来小时和天气预报)
4. 渲染5天天气预报
5. 渲染位置和日期信息
6. 渲染天气警报信息(如果有)
7. 渲染状态栏(刷新时间、WiFi信号、电池电量)
8. 完成渲染并关闭显示屏电源

### 5. 低功耗管理流程

1. 计算下一次唤醒时间
   - 根据SLEEP_DURATION配置(默认30分钟)
   - 考虑BED_TIME和WAKE_TIME设置(节电模式)
2. 设置定时器唤醒
3. 进入深度睡眠模式(<11μA功耗)

## 错误处理机制

系统设计了完善的错误处理机制，主要包括：

1. **低电量错误**：当电池电压低于阈值时，显示低电量警告并延长睡眠时间
2. **WiFi连接错误**：无法连接WiFi时，显示网络错误信息
3. **API错误**：API请求失败时，显示相应的HTTP错误代码和描述
4. **时间服务器错误**：无法从NTP服务器获取时间时，显示时间同步失败信息

## 配置选项

系统提供了丰富的配置选项，可以根据用户需求进行定制：

1. **硬件配置**：支持不同型号的电子墨水屏和传感器
2. **显示选项**：字体选择、图标精度、显示内容控制
3. **单位选择**：支持公制和英制单位(温度、风速、气压等)
4. **电源管理**：可配置睡眠时间、唤醒时间和低电量阈值
5. **网络设置**：WiFi凭据、API密钥、NTP服务器
6. **本地化**：支持多种语言和时间格式

## 总结

ESP32电子墨水屏天气显示器是一个精心设计的低功耗物联网设备，通过优化的软硬件架构实现了长电池寿命和丰富的功能。系统采用模块化设计，各组件之间职责明确，便于维护和扩展。深度睡眠模式和电源管理策略使设备能够在单次充电后运行数月，是一个理想的家庭天气显示解决方案。