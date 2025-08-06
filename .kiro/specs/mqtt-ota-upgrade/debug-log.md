# MQTT OTA升级功能调试日志规范

## 日志级别定义

### 级别分类

```cpp
enum OTALogLevel {
    OTA_LOG_ERROR = 0,    // 错误：升级失败、连接错误等
    OTA_LOG_WARN = 1,     // 警告：电量不足、版本问题等
    OTA_LOG_INFO = 2,     // 信息：升级开始、完成状态等
    OTA_LOG_DEBUG = 3,    // 调试：详细流程、参数值等
    OTA_LOG_VERBOSE = 4   // 详细：所有MQTT消息、HTTP请求等
};
```

### 日志输出宏

```cpp
#define OTA_LOG_E(format, ...) if(OTA_DEBUG_LEVEL >= OTA_LOG_ERROR) Serial.printf("[OTA-E] " format "\n", ##__VA_ARGS__)
#define OTA_LOG_W(format, ...) if(OTA_DEBUG_LEVEL >= OTA_LOG_WARN) Serial.printf("[OTA-W] " format "\n", ##__VA_ARGS__)
#define OTA_LOG_I(format, ...) if(OTA_DEBUG_LEVEL >= OTA_LOG_INFO) Serial.printf("[OTA-I] " format "\n", ##__VA_ARGS__)
#define OTA_LOG_D(format, ...) if(OTA_DEBUG_LEVEL >= OTA_LOG_DEBUG) Serial.printf("[OTA-D] " format "\n", ##__VA_ARGS__)
#define OTA_LOG_V(format, ...) if(OTA_DEBUG_LEVEL >= OTA_LOG_VERBOSE) Serial.printf("[OTA-V] " format "\n", ##__VA_ARGS__)
```

## 关键流程日志

### 1. MQTT连接流程

#### 成功流程日志
```
[OTA-I] Starting MQTT OTA check...
[OTA-D] MQTT Config: server=mqtt.example.com, port=1883, device_id=weather-001
[OTA-D] Connecting to MQTT broker...
[OTA-I] MQTT connected successfully
[OTA-D] Subscribing to topic: devices/weather-001/ota/upgrade
[OTA-I] Subscribed to OTA topic, waiting for messages...
[OTA-D] Waiting for upgrade message (timeout: 10000ms)
[OTA-I] No upgrade message received, disconnecting
[OTA-D] MQTT disconnected
[OTA-I] MQTT OTA check completed (duration: 3.2s)
```

#### 连接失败日志
```
[OTA-I] Starting MQTT OTA check...
[OTA-D] MQTT Config: server=mqtt.example.com, port=1883, device_id=weather-001
[OTA-D] Connecting to MQTT broker...
[OTA-E] MQTT connection failed: Connection refused (code: -2)
[OTA-W] MQTT OTA check skipped due to connection failure
[OTA-I] Continuing with normal weather update flow
```

### 2. 升级消息处理

#### 消息接收和解析
```
[OTA-I] Received MQTT message on topic: devices/weather-001/ota/upgrade
[OTA-V] Raw message: {"command":"upgrade","version":"1.2.0","download_url":"https://..."}
[OTA-D] Parsing upgrade message...
[OTA-D] Parsed: command=upgrade, version=1.2.0, force=false, min_battery=30%
[OTA-I] Upgrade available: 1.1.0 -> 1.2.0
[OTA-D] Current battery level: 85%
[OTA-I] Battery level sufficient for upgrade (85% >= 30%)
[OTA-I] Starting OTA upgrade process...
```

#### 消息格式错误
```
[OTA-I] Received MQTT message on topic: devices/weather-001/ota/upgrade
[OTA-V] Raw message: {"invalid":"json","missing":"fields"}
[OTA-E] Failed to parse upgrade message: Missing required field 'command'
[OTA-W] Ignoring invalid upgrade message
```

#### 版本检查
```
[OTA-D] Version comparison: current=1.2.0, new=1.1.0
[OTA-W] New version (1.1.0) is not newer than current (1.2.0)
[OTA-I] Skipping downgrade, no upgrade needed
```

### 3. 固件下载流程

#### 成功下载
```
[OTA-I] Starting firmware download...
[OTA-D] Download URL: https://firmware.example.com/weather-v1.2.0.bin
[OTA-D] Checking content length...
[OTA-I] Firmware size: 1,234,567 bytes
[OTA-D] Available flash space: 2,097,152 bytes
[OTA-I] Sufficient space available for firmware
[OTA-I] Starting firmware download...
[OTA-D] Download progress: 25% (307,642 bytes)
[OTA-D] Download progress: 50% (617,284 bytes)
[OTA-D] Download progress: 75% (926,925 bytes)
[OTA-D] Download progress: 100% (1,234,567 bytes)
[OTA-I] Firmware download completed
[OTA-D] Verifying firmware checksum...
[OTA-I] Firmware verification successful
```

#### 下载失败
```
[OTA-I] Starting firmware download...
[OTA-D] Download URL: https://firmware.example.com/weather-v1.2.0.bin
[OTA-E] HTTP request failed: 404 Not Found
[OTA-W] Firmware download failed, retrying... (attempt 1/3)
[OTA-D] Retry delay: 2000ms
[OTA-E] HTTP request failed: 404 Not Found
[OTA-W] Firmware download failed, retrying... (attempt 2/3)
[OTA-D] Retry delay: 4000ms
[OTA-E] HTTP request failed: 404 Not Found
[OTA-E] Firmware download failed after 3 attempts
[OTA-E] OTA upgrade aborted due to download failure
```

### 4. OTA安装流程

#### 成功安装
```
[OTA-I] Starting OTA installation...
[OTA-D] Initializing Update library...
[OTA-D] Update partition size: 1,966,080 bytes
[OTA-I] Writing firmware to flash...
[OTA-D] Write progress: 25% (308,400 bytes)
[OTA-D] Write progress: 50% (617,000 bytes)
[OTA-D] Write progress: 75% (925,500 bytes)
[OTA-D] Write progress: 100% (1,234,567 bytes)
[OTA-I] Firmware write completed
[OTA-D] Finalizing update...
[OTA-I] OTA installation successful
[OTA-I] Saving version info to NVS...
[OTA-D] Saved: current_version=1.2.0, previous_version=1.1.0
[OTA-I] Restarting device...
```

#### 安装失败
```
[OTA-I] Starting OTA installation...
[OTA-D] Initializing Update library...
[OTA-E] Update initialization failed: Insufficient space
[OTA-E] Available space: 1,500,000 bytes, required: 1,600,000 bytes
[OTA-E] OTA installation failed
[OTA-W] Keeping current firmware version 1.1.0
```

## 错误代码和诊断

### MQTT错误代码

```cpp
const char* getMQTTErrorString(int error) {
    switch(error) {
        case -4: return "Connection timeout";
        case -3: return "Connection lost";
        case -2: return "Connection refused";
        case -1: return "Disconnected";
        case 0: return "Connected";
        case 1: return "Bad protocol version";
        case 2: return "Bad client ID";
        case 3: return "Server unavailable";
        case 4: return "Bad credentials";
        case 5: return "Not authorized";
        default: return "Unknown error";
    }
}
```

#### 错误日志示例
```
[OTA-E] MQTT connection failed: Bad credentials (code: 4)
[OTA-D] Check MQTT username and password in configuration
[OTA-E] MQTT connection failed: Server unavailable (code: 3)
[OTA-D] Check MQTT server address and network connectivity
```

### HTTP错误代码

#### 常见HTTP错误日志
```
[OTA-E] Firmware download failed: 401 Unauthorized
[OTA-D] Check firmware server authentication
[OTA-E] Firmware download failed: 403 Forbidden
[OTA-D] Check firmware access permissions
[OTA-E] Firmware download failed: 404 Not Found
[OTA-D] Check firmware URL and file existence
[OTA-E] Firmware download failed: 500 Internal Server Error
[OTA-D] Firmware server error, try again later
```

### OTA错误代码

```cpp
const char* getOTAErrorString(int error) {
    switch(error) {
        case UPDATE_ERROR_OK: return "No error";
        case UPDATE_ERROR_WRITE: return "Flash write failed";
        case UPDATE_ERROR_ERASE: return "Flash erase failed";
        case UPDATE_ERROR_READ: return "Flash read failed";
        case UPDATE_ERROR_SPACE: return "Insufficient space";
        case UPDATE_ERROR_SIZE: return "Invalid size";
        case UPDATE_ERROR_STREAM: return "Stream error";
        case UPDATE_ERROR_MD5: return "MD5 verification failed";
        case UPDATE_ERROR_MAGIC_BYTE: return "Invalid magic byte";
        case UPDATE_ERROR_ACTIVATE: return "Activation failed";
        case UPDATE_ERROR_NO_PARTITION: return "No update partition";
        case UPDATE_ERROR_BAD_ARGUMENT: return "Bad argument";
        case UPDATE_ERROR_ABORT: return "Update aborted";
        default: return "Unknown OTA error";
    }
}
```

## 性能监控日志

### 内存使用监控

```cpp
void logMemoryUsage(const char* stage) {
    OTA_LOG_D("Memory usage at %s:", stage);
    OTA_LOG_D("  Free heap: %d bytes", ESP.getFreeHeap());
    OTA_LOG_D("  Heap size: %d bytes", ESP.getHeapSize());
    OTA_LOG_D("  Free PSRAM: %d bytes", ESP.getFreePsram());
    OTA_LOG_D("  PSRAM size: %d bytes", ESP.getPsramSize());
}
```

#### 输出示例
```
[OTA-D] Memory usage at MQTT_START:
[OTA-D]   Free heap: 234,567 bytes
[OTA-D]   Heap size: 327,680 bytes
[OTA-D]   Free PSRAM: 4,194,304 bytes
[OTA-D]   PSRAM size: 4,194,304 bytes
```

### 时间性能监控

```cpp
class OTATimer {
private:
    unsigned long startTime;
    String operation;
    
public:
    OTATimer(const String& op) : operation(op) {
        startTime = millis();
        OTA_LOG_D("Starting %s...", operation.c_str());
    }
    
    ~OTATimer() {
        unsigned long duration = millis() - startTime;
        OTA_LOG_I("%s completed in %lu ms", operation.c_str(), duration);
    }
};
```

#### 使用示例和输出
```cpp
{
    OTATimer timer("MQTT Connection");
    // MQTT连接代码
}
// 输出: [OTA-I] MQTT Connection completed in 2,345 ms

{
    OTATimer timer("Firmware Download");
    // 固件下载代码
}
// 输出: [OTA-I] Firmware Download completed in 45,678 ms
```

### 网络性能监控

```cpp
void logNetworkStats() {
    OTA_LOG_D("Network statistics:");
    OTA_LOG_D("  WiFi RSSI: %d dBm", WiFi.RSSI());
    OTA_LOG_D("  WiFi channel: %d", WiFi.channel());
    OTA_LOG_D("  Local IP: %s", WiFi.localIP().toString().c_str());
    OTA_LOG_D("  Gateway: %s", WiFi.gatewayIP().toString().c_str());
    OTA_LOG_D("  DNS: %s", WiFi.dnsIP().toString().c_str());
}
```

## 调试配置

### 编译时调试配置

```cpp
// config.h
#ifndef OTA_DEBUG_LEVEL
#define OTA_DEBUG_LEVEL OTA_LOG_INFO  // 默认信息级别
#endif

#ifndef OTA_ENABLE_VERBOSE_MQTT
#define OTA_ENABLE_VERBOSE_MQTT 0     // 默认关闭MQTT详细日志
#endif

#ifndef OTA_ENABLE_MEMORY_DEBUG
#define OTA_ENABLE_MEMORY_DEBUG 0     // 默认关闭内存调试
#endif

#ifndef OTA_ENABLE_TIMING_DEBUG
#define OTA_ENABLE_TIMING_DEBUG 1     // 默认开启时间调试
#endif
```

### 运行时调试控制

```cpp
// 通过串口命令控制调试级别
void handleDebugCommand(const String& command) {
    if (command.startsWith("ota_debug ")) {
        int level = command.substring(10).toInt();
        if (level >= 0 && level <= 4) {
            otaDebugLevel = level;
            OTA_LOG_I("OTA debug level set to %d", level);
        }
    } else if (command == "ota_test") {
        // 触发测试升级检查
        testOTACheck();
    } else if (command == "ota_status") {
        // 显示当前OTA状态
        printOTAStatus();
    }
}
```

## 故障排除指南

### 常见问题诊断

#### 1. MQTT连接失败
```
问题症状：
[OTA-E] MQTT connection failed: Connection refused (code: -2)

可能原因：
1. MQTT服务器地址或端口错误
2. 网络连接问题
3. MQTT服务器未运行
4. 防火墙阻止连接

诊断步骤：
1. 检查网络连接：ping MQTT服务器
2. 检查端口：telnet server port
3. 验证MQTT服务器状态
4. 检查防火墙设置
```

#### 2. 固件下载失败
```
问题症状：
[OTA-E] Firmware download failed: 404 Not Found

可能原因：
1. 固件URL错误
2. 固件文件不存在
3. 服务器权限问题
4. 网络连接中断

诊断步骤：
1. 验证固件URL可访问性
2. 检查固件文件存在性
3. 验证服务器权限配置
4. 检查网络稳定性
```

#### 3. OTA安装失败
```
问题症状：
[OTA-E] Update initialization failed: Insufficient space

可能原因：
1. Flash空间不足
2. 分区表配置错误
3. 固件大小超限
4. Flash硬件故障

诊断步骤：
1. 检查Flash使用情况
2. 验证分区表配置
3. 优化固件大小
4. 测试Flash硬件
```

### 调试工具和命令

#### 串口调试命令
```cpp
// 在串口监视器中输入以下命令进行调试
"ota_debug 4"     // 设置详细调试级别
"ota_test"        // 触发测试升级检查
"ota_status"      // 显示当前状态
"ota_config"      // 显示配置信息
"ota_memory"      // 显示内存使用情况
"ota_reset"       // 重置OTA配置
```

#### 日志分析脚本
```python
# log_analyzer.py - 分析OTA日志的Python脚本
import re
import sys

def analyze_ota_log(log_file):
    with open(log_file, 'r') as f:
        lines = f.readlines()
    
    # 统计各类日志数量
    error_count = len([l for l in lines if '[OTA-E]' in l])
    warn_count = len([l for l in lines if '[OTA-W]' in l])
    info_count = len([l for l in lines if '[OTA-I]' in l])
    
    print(f"Log Analysis Results:")
    print(f"  Errors: {error_count}")
    print(f"  Warnings: {warn_count}")
    print(f"  Info: {info_count}")
    
    # 提取升级时间
    upgrade_times = []
    for line in lines:
        if 'completed in' in line:
            match = re.search(r'(\d+) ms', line)
            if match:
                upgrade_times.append(int(match.group(1)))
    
    if upgrade_times:
        print(f"  Average upgrade time: {sum(upgrade_times)/len(upgrade_times):.1f} ms")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        analyze_ota_log(sys.argv[1])
    else:
        print("Usage: python log_analyzer.py <log_file>")
```

## 日志轮转和存储

### NVS日志存储

```cpp
// 将关键日志存储到NVS以便重启后查看
void saveOTALogToNVS(const String& message) {
    preferences.begin("ota_logs", false);
    
    // 获取当前日志计数
    int logCount = preferences.getInt("count", 0);
    
    // 存储日志（最多保存10条）
    String key = "log_" + String(logCount % 10);
    String timestampedMessage = String(millis()) + ": " + message;
    preferences.putString(key.c_str(), timestampedMessage);
    
    // 更新计数
    preferences.putInt("count", logCount + 1);
    preferences.end();
}

// 读取NVS中的日志
void printStoredOTALogs() {
    preferences.begin("ota_logs", true);
    int logCount = preferences.getInt("count", 0);
    
    Serial.println("=== Stored OTA Logs ===");
    for (int i = 0; i < min(10, logCount); i++) {
        String key = "log_" + String(i);
        String log = preferences.getString(key.c_str(), "");
        if (log.length() > 0) {
            Serial.println(log);
        }
    }
    preferences.end();
}
```

这个调试日志规范提供了：

1. **分级日志系统**：从错误到详细的5个级别
2. **关键流程日志**：MQTT连接、消息处理、下载、安装的完整日志
3. **错误诊断**：详细的错误代码和解决方案
4. **性能监控**：内存、时间、网络性能的监控日志
5. **调试工具**：串口命令、日志分析脚本等
6. **故障排除**：常见问题的诊断步骤和解决方案

这将大大简化MQTT OTA功能的开发、测试和维护工作。