# MQTT OTA升级功能设计文档

## 概述

本文档描述了为ESP32电子墨水屏天气显示器添加MQTT OTA升级功能的详细设计。该功能在现有的天气更新流程中集成MQTT升级检查，实现远程固件升级能力。

## 架构设计

### 系统架构

```
现有系统流程：
唤醒 → WiFi连接 → 天气API → 传感器 → 显示 → 睡眠

新增MQTT OTA流程：
唤醒 → WiFi连接 → [MQTT升级检查] → 天气API → 传感器 → 显示 → 睡眠
                      ↓
                 升级消息? → 下载固件 → 验证 → 安装 → 重启
```

### 组件设计

#### 1. MQTT OTA管理器 (MQTTOTAManager)

```cpp
class MQTTOTAManager {
private:
    PubSubClient mqttClient;
    WiFiClient wifiClient;
    String deviceId;
    String otaTopic;
    bool otaEnabled;
    
public:
    bool begin();
    bool checkForUpgrade();
    void handleOTAMessage(const String& message);
    bool performOTAUpgrade(const String& url, const String& version);
    void disconnect();
};
```

**职责：**
- MQTT连接管理
- 升级消息订阅和处理
- OTA升级执行
- 错误处理和日志记录

#### 2. 固件下载器 (FirmwareDownloader)

```cpp
class FirmwareDownloader {
private:
    HTTPClient httpClient;
    
public:
    bool downloadFirmware(const String& url, Stream& output);
    bool verifyFirmware(const uint8_t* data, size_t length);
    size_t getContentLength(const String& url);
};
```

**职责：**
- HTTP固件下载
- 下载进度监控
- 固件完整性验证

#### 3. 升级消息解析器 (UpgradeMessageParser)

```cpp
struct UpgradeMessage {
    String command;
    String version;
    String downloadUrl;
    bool forceUpdate;
    int minBatteryLevel;
    String checksum;
};

class UpgradeMessageParser {
public:
    bool parseMessage(const String& json, UpgradeMessage& message);
    bool validateMessage(const UpgradeMessage& message);
    bool isNewerVersion(const String& newVersion, const String& currentVersion);
};
```

**职责：**
- JSON消息解析
- 消息格式验证
- 版本比较逻辑

## 接口设计

### MQTT消息格式

#### 升级指令消息

**Topic:** `devices/{device_id}/ota/upgrade`

**消息格式：**
```json
{
    "command": "upgrade",
    "version": "1.2.0",
    "download_url": "https://firmware-server.com/weather-display/v1.2.0.bin",
    "force_update": false,
    "min_battery_level": 30,
    "checksum": "sha256:abcdef123456...",
    "timestamp": 1640995200,
    "description": "Bug fixes and performance improvements"
}
```

#### 状态报告消息

**Topic:** `devices/{device_id}/ota/status`

**消息格式：**
```json
{
    "device_id": "weather-display-001",
    "current_version": "1.1.0",
    "status": "upgrade_success",
    "battery_level": 85,
    "timestamp": 1640995200,
    "message": "Upgraded to version 1.2.0 successfully"
}
```

### 配置接口

```cpp
struct MQTTOTAConfig {
    String mqttServer = "mqtt.example.com";
    int mqttPort = 1883;
    String mqttUsername = "";
    String mqttPassword = "";
    String deviceId = "weather-display-001";
    bool enableSSL = false;
    int connectionTimeout = 5000;  // ms
    int messageTimeout = 10000;    // ms
    bool enableOTA = true;
    int minBatteryLevel = 30;      // %
};
```

## 数据模型

### 版本信息存储

```cpp
struct VersionInfo {
    String currentVersion;
    String previousVersion;
    unsigned long lastUpgradeTime;
    int upgradeCount;
    bool rollbackAvailable;
};
```

**存储位置：** NVS (Non-Volatile Storage)
**键名：** "ota_version_info"

### 升级状态跟踪

```cpp
enum OTAStatus {
    OTA_IDLE,
    OTA_CHECKING,
    OTA_DOWNLOADING,
    OTA_VERIFYING,
    OTA_INSTALLING,
    OTA_SUCCESS,
    OTA_FAILED
};

struct OTAState {
    OTAStatus status;
    String errorMessage;
    int progress;  // 0-100
    unsigned long startTime;
};
```

## 错误处理

### 错误类型定义

```cpp
enum OTAError {
    OTA_ERROR_NONE = 0,
    OTA_ERROR_MQTT_CONNECTION = 1,
    OTA_ERROR_INVALID_MESSAGE = 2,
    OTA_ERROR_LOW_BATTERY = 3,
    OTA_ERROR_DOWNLOAD_FAILED = 4,
    OTA_ERROR_VERIFICATION_FAILED = 5,
    OTA_ERROR_INSTALLATION_FAILED = 6,
    OTA_ERROR_VERSION_DOWNGRADE = 7,
    OTA_ERROR_INSUFFICIENT_SPACE = 8
};
```

### 错误处理策略

1. **MQTT连接错误**：记录日志，继续正常流程
2. **下载错误**：重试3次，失败后放弃本次升级
3. **验证错误**：立即停止升级，保持当前版本
4. **安装错误**：尝试回滚到前一版本
5. **低电量错误**：延迟升级到下次电量充足时

## 测试策略

### 单元测试

1. **消息解析测试**
   - 有效JSON消息解析
   - 无效JSON消息处理
   - 缺失字段处理
   - 版本比较逻辑

2. **MQTT连接测试**
   - 正常连接和断开
   - 连接超时处理
   - 消息订阅和接收
   - SSL连接测试

3. **OTA功能测试**
   - 固件下载测试
   - 校验和验证
   - 安装过程测试
   - 回滚功能测试

### 集成测试

1. **端到端升级测试**
   - 完整升级流程
   - 网络中断恢复
   - 电量不足处理
   - 强制升级测试

2. **功耗测试**
   - MQTT检查功耗测量
   - 升级过程功耗分析
   - 长期续航影响评估

3. **稳定性测试**
   - 连续升级测试
   - 异常情况恢复
   - 内存泄漏检测

## 安全考虑

### 固件验证

1. **校验和验证**：使用SHA-256校验固件完整性
2. **版本验证**：防止版本降级攻击
3. **来源验证**：验证固件下载URL的合法性

### 通信安全

1. **MQTT SSL/TLS**：支持加密连接
2. **消息认证**：MQTT用户名密码认证
3. **Topic权限**：设备只能订阅自己的升级topic

### 升级安全

1. **电量检查**：确保升级过程中不会因电量不足导致设备损坏
2. **空间检查**：验证Flash空间足够存储新固件
3. **回滚机制**：保留前一版本以支持紧急回滚

## 性能考虑

### 功耗优化

1. **快速连接**：MQTT连接超时设置为5秒
2. **消息等待**：最多等待10秒接收升级消息
3. **立即断开**：检查完成后立即断开MQTT连接
4. **条件检查**：只在WiFi连接成功后才进行MQTT检查

### 内存优化

1. **流式下载**：使用流式处理避免将整个固件加载到内存
2. **缓冲区管理**：使用固定大小的缓冲区处理数据
3. **及时释放**：及时释放不再使用的内存资源

### 网络优化

1. **连接复用**：复用现有的WiFi连接
2. **超时控制**：设置合理的网络超时时间
3. **重试机制**：实现指数退避的重试策略

## 配置管理

### 编译时配置

```cpp
// config.h
#define ENABLE_MQTT_OTA 1
#define MQTT_OTA_TIMEOUT 10000
#define MQTT_CONNECTION_TIMEOUT 5000
#define OTA_MIN_BATTERY_LEVEL 30
#define OTA_MAX_RETRIES 3
```

### 运行时配置

```cpp
// 从NVS读取配置
void loadMQTTOTAConfig() {
    preferences.begin("mqtt_ota", true);
    mqttConfig.mqttServer = preferences.getString("server", "mqtt.example.com");
    mqttConfig.mqttPort = preferences.getInt("port", 1883);
    mqttConfig.enableOTA = preferences.getBool("enable", true);
    preferences.end();
}
```

## 部署考虑

### 固件服务器要求

1. **HTTP/HTTPS服务器**：支持固件文件下载
2. **文件完整性**：提供SHA-256校验和
3. **版本管理**：支持多版本固件存储
4. **访问控制**：限制固件下载权限

### MQTT服务器要求

1. **稳定性**：高可用的MQTT服务器
2. **认证**：支持用户名密码认证
3. **SSL/TLS**：支持加密连接
4. **Topic权限**：支持基于设备的topic权限控制

### 监控和日志

1. **升级状态监控**：实时监控设备升级状态
2. **错误日志收集**：收集和分析升级错误
3. **性能指标**：监控升级成功率和耗时
4. **电量统计**：分析升级对电池续航的影响