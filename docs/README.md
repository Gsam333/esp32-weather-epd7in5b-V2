# ESP32 电子墨水屏天气显示器项目文档

本目录包含ESP32电子墨水屏天气显示器项目的相关文档，旨在帮助新开发者快速了解项目架构、功能和实现细节，无需逐行阅读代码。

## 文档目录

### 1. 产品需求文档 (PRD)

[ESP32_E-Paper_Weather_Display_PRD.md](ESP32_E-Paper_Weather_Display_PRD.md) - 详细描述了项目的功能需求、技术规格、用户体验和实现要求等。这是了解项目整体设计和目标的最佳起点。

### 2. 架构文档

[ESP32_E-Paper_Weather_Display_Architecture.md](ESP32_E-Paper_Weather_Display_Architecture.md) - 介绍了系统的整体架构，包括硬件组件、软件模块和它们之间的交互关系。

### 3. 流程图文档

[ESP32_E-Paper_Weather_Display_Flowchart.md](ESP32_E-Paper_Weather_Display_Flowchart.md) - 使用流程图详细说明了系统的各个工作流程，包括主程序流程、深度睡眠计算、API数据获取等。

### 4. 调试文档

[调试文档.md](调试文档.md) - 记录了项目开发过程中遇到的问题和解决方案，包括硬件配置、软件设置、问题排查和功能增强等内容。

## 项目概述

ESP32电子墨水屏天气显示器是一款低功耗、长续航的智能家居天气显示设备，通过WiFi连接互联网获取实时天气数据，并在电子墨水屏上清晰展示。设备采用ESP32微控制器和7.5英寸电子墨水屏，配合BME280/BME680传感器监测室内环境参数，为用户提供全面的天气信息服务。

### 主要特点

- **低功耗设计**：深度睡眠模式下仅消耗约14μA电流，单次充电可持续使用6-12个月
- **全面天气信息**：显示当前天气、5天预报、24小时预报图表、空气质量等数据
- **室内环境监测**：通过BME280/BME680传感器监测室内温度、湿度和气压
- **高度可定制**：支持多语言、多种单位制、多种显示格式
- **错误处理机制**：提供WiFi连接错误、API错误、低电量等错误提示
- **LED状态指示**：通过LED指示灯显示设备工作状态

### 技术栈

- **硬件**：ESP32微控制器、7.5英寸电子墨水屏、BME280/BME680传感器
- **软件**：C++、Arduino框架、PlatformIO开发环境
- **API**：OpenWeatherMap API、NTP时间服务器
- **库**：GxEPD2、ArduinoJson、WiFi/HTTPClient、Adafruit_Sensor

## 快速入门

1. 查看[产品需求文档](ESP32_E-Paper_Weather_Display_PRD.md)了解项目整体设计
2. 参考[架构文档](ESP32_E-Paper_Weather_Display_Architecture.md)了解系统架构
3. 通过[流程图文档](ESP32_E-Paper_Weather_Display_Flowchart.md)理解工作流程
4. 阅读[调试文档](调试文档.md)了解常见问题和解决方案
5. 查看项目根目录的[README.md](../README.md)获取组件清单和设置指南

## 贡献指南

如果您想为项目做出贡献，请遵循以下步骤：

1. Fork项目仓库
2. 创建您的特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交您的更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 打开Pull Request

## 联系方式

如有任何问题或建议，请通过以下方式联系我们：

- 项目仓库Issues页面
- 电子邮件：[项目维护者邮箱]

---

*文档版本：1.0*  
*最后更新：2025年7月22日*