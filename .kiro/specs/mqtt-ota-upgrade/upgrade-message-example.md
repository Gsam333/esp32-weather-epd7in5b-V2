# UpgradeMessage 结构体使用示例

## 概述

`UpgradeMessage` 结构体是MQTT OTA升级功能的核心数据结构，用于表示和管理升级消息的所有信息。

## 基本使用

### 创建和设置消息

```cpp
#include "mqtt_ota_config.h"

// 创建升级消息实例
UpgradeMessage msg;

// 设置基本信息
msg.setCommand("upgrade");
msg.setVersion("1.2.3");
msg.setDownloadUrl("https://example.com/firmware.bin");
msg.setDescription("Bug fixes and performance improvements");

// 设置可选参数
msg.setForceUpdate(false);
msg.setMinBatteryLevel(40);
msg.setChecksum("sha256:abcdef123456789");
msg.setCurrentTimestamp();
msg.setStatus(MSG_STATUS_RECEIVED);
```

### 读取消息信息

```cpp
// 获取基本信息
String command = msg.getCommand();
String version = msg.getVersion();
String url = msg.getDownloadUrl();
String description = msg.getDescription();

// 获取配置参数
bool isForced = msg.getForceUpdate();
int minBattery = msg.getMinBatteryLevel();
String checksum = msg.getChecksum();

// 获取状态信息
UpgradeMessageStatus status = msg.getStatus();
unsigned long age = msg.getAgeInSeconds();
bool expired = msg.isExpired();
```

## 消息验证

### 基本验证

```cpp
// 检查消息是否有效
if (msg.isValid()) {
    Serial.println("Message is valid");
} else {
    Serial.println("Message is invalid");
}

// 检查特定条件
if (msg.isForcedUpdate()) {
    Serial.println("This is a forced update");
}

if (msg.hasChecksum()) {
    Serial.println("Message includes checksum verification");
}

if (msg.isExpired()) {
    Serial.println("Message has expired");
}
```

### 自定义验证

```cpp
// 检查消息年龄（自定义超时时间）
if (msg.isExpired(300)) { // 5分钟超时
    Serial.println("Message expired (5 min timeout)");
}

// 获取详细的年龄信息
unsigned long ageSeconds = msg.getAgeInSeconds();
Serial.printf("Message age: %lu seconds\n", ageSeconds);
```

## 消息状态管理

### 状态枚举

```cpp
enum UpgradeMessageStatus {
    MSG_STATUS_UNKNOWN = 0,     // 未知状态
    MSG_STATUS_RECEIVED = 1,    // 已接收
    MSG_STATUS_VALIDATED = 2,   // 已验证
    MSG_STATUS_PROCESSING = 3,  // 处理中
    MSG_STATUS_COMPLETED = 4,   // 已完成
    MSG_STATUS_FAILED = 5       // 失败
};
```

### 状态操作

```cpp
// 设置状态
msg.setStatus(MSG_STATUS_PROCESSING);

// 获取状态
UpgradeMessageStatus currentStatus = msg.getStatus();

// 获取状态描述
const char* statusStr = getUpgradeMessageStatusString(currentStatus);
Serial.printf("Current status: %s\n", statusStr);

// 设置错误信息
msg.setErrorMessage("Download failed: connection timeout");
String errorMsg = msg.getErrorMessage();
```

## 消息拷贝和清理

### 拷贝消息

```cpp
// 拷贝构造函数
UpgradeMessage originalMsg;
originalMsg.setVersion("1.0.0");

UpgradeMessage copyMsg = originalMsg;

// 赋值操作符
UpgradeMessage anotherMsg;
anotherMsg = originalMsg;
```

### 清理消息

```cpp
// 清空所有字段
msg.clear();

// 验证清理结果
if (!msg.isValid()) {
    Serial.println("Message cleared successfully");
}
```

## 调试和日志

### 消息字符串表示

```cpp
// 获取消息的字符串表示
String msgStr = msg.toString();
Serial.println("Message: " + msgStr);

// 输出示例：
// Message: UpgradeMessage{command=upgrade, version=1.2.3, url=https://example.com/firmware.bin, force=false, minBattery=40, checksum=sha256:a..., status=2, age=15s}
```

### 详细调试信息

```cpp
// 打印所有字段
Serial.println("=== Upgrade Message Details ===");
Serial.printf("Command: %s\n", msg.getCommand().c_str());
Serial.printf("Version: %s\n", msg.getVersion().c_str());
Serial.printf("Download URL: %s\n", msg.getDownloadUrl().c_str());
Serial.printf("Force Update: %s\n", msg.getForceUpdate() ? "Yes" : "No");
Serial.printf("Min Battery: %d%%\n", msg.getMinBatteryLevel());
Serial.printf("Checksum: %s\n", msg.getChecksum().c_str());
Serial.printf("Description: %s\n", msg.getDescription().c_str());
Serial.printf("Status: %s\n", getUpgradeMessageStatusString(msg.getStatus()));
Serial.printf("Age: %lu seconds\n", msg.getAgeInSeconds());
Serial.printf("Expired: %s\n", msg.isExpired() ? "Yes" : "No");
Serial.printf("Valid: %s\n", msg.isValid() ? "Yes" : "No");
```

## JSON解析示例

### 从JSON创建消息

```cpp
// JSON消息示例
String jsonMessage = R"({
    "command": "upgrade",
    "version": "1.2.3",
    "download_url": "https://example.com/firmware.bin",
    "force_update": false,
    "min_battery_level": 40,
    "checksum": "sha256:abcdef123456789",
    "description": "Bug fixes and improvements",
    "timestamp": 1640995200
})";

// 解析JSON（通常在MQTTOTAManager中完成）
UpgradeMessage msg;
if (mqttOTAManager.parseUpgradeMessage(jsonMessage, msg)) {
    Serial.println("JSON parsed successfully");
    Serial.println(msg.toString());
} else {
    Serial.println("Failed to parse JSON");
}
```

## 最佳实践

### 1. 消息生命周期管理

```cpp
// 创建消息时设置时间戳
msg.setCurrentTimestamp();

// 处理前检查过期
if (msg.isExpired()) {
    Serial.println("Message expired, ignoring");
    return;
}

// 更新状态以跟踪处理进度
msg.setStatus(MSG_STATUS_PROCESSING);
```

### 2. 错误处理

```cpp
// 验证消息
if (!msg.isValid()) {
    msg.setStatus(MSG_STATUS_FAILED);
    msg.setErrorMessage("Invalid message format");
    return;
}

// 处理过程中的错误
if (downloadFailed) {
    msg.setStatus(MSG_STATUS_FAILED);
    msg.setErrorMessage("Download failed: " + errorDetails);
}
```

### 3. 安全检查

```cpp
// 检查强制更新标志
if (!msg.getForceUpdate()) {
    // 执行额外的安全检查
    if (batteryLevel < msg.getMinBatteryLevel()) {
        Serial.println("Battery too low for update");
        return;
    }
}

// 验证校验和
if (msg.hasChecksum()) {
    // 下载后验证文件完整性
    if (!verifyChecksum(firmwareData, msg.getChecksum())) {
        msg.setStatus(MSG_STATUS_FAILED);
        msg.setErrorMessage("Checksum verification failed");
        return;
    }
}
```

## 测试

可以使用内置的测试函数来验证 `UpgradeMessage` 的功能：

```cpp
// 在串口监视器中输入以下命令
upgrade_msg_test
```

这将运行完整的 `UpgradeMessage` 结构体测试，包括：
- 基本getter/setter功能
- 消息验证
- 状态管理
- 拷贝和清理操作
- 错误处理场景