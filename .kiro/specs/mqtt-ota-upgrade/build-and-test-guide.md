# MQTT OTA升级功能编译和测试指南

## 编译准备

### 1. 启用MQTT OTA功能

在 `platformio.ini` 文件中，找到以下注释行并取消注释：

```ini
; MQTT OTA功能开关（默认关闭，需要时取消注释）
-DMQTT_OTA_UPGRADE
-DMQTT_OTA_DEBUG_LEVEL=2
```

修改为：

```ini
; MQTT OTA功能开关（默认关闭，需要时取消注释）
-DMQTT_OTA_UPGRADE
-DMQTT_OTA_DEBUG_LEVEL=2
```

### 2. 配置MQTT服务器

复制配置模板：

```bash
cp src/include/secrets.h.example src/include/secrets.h
```

编辑 `src/include/secrets.h` 文件，配置你的MQTT服务器：

```cpp
#ifdef MQTT_OTA_UPGRADE
// 基本MQTT服务器配置（必须配置）
#define SECRET_MQTT_OTA_SERVER "your-mqtt-server.com"
#define SECRET_MQTT_OTA_PORT 1883
#define SECRET_MQTT_OTA_USERNAME "your_username"  // 留空""表示无认证
#define SECRET_MQTT_OTA_PASSWORD "your_password"  // 留空""表示无认证
#define SECRET_MQTT_OTA_USE_SSL false
#endif
```

## 编译命令

### 基本编译

```bash
# 编译项目
pio run

# 编译并上传
pio run --target upload

# 编译并监控串口
pio run --target upload --target monitor
```

### 指定开发板编译

```bash
# 为DFRobot FireBeetle ESP32-E编译
pio run -e dfrobot_firebeetle2_esp32e

# 为ESP32-DevKitC编译
pio run -e board_esp32dev

# 为FireBeetle32编译
pio run -e firebeetle32
```

### 清理和重新编译

```bash
# 清理编译缓存
pio run --target clean

# 清理并重新编译
pio run --target clean && pio run
```

## 编译验证

### 检查编译输出

编译成功后，应该看到类似输出：

```
Building in release mode
Compiling .pio/build/dfrobot_firebeetle2_esp32e/src/main.cpp.o
Compiling .pio/build/dfrobot_firebeetle2_esp32e/src/mqtt_ota_manager.cpp.o
Compiling .pio/build/dfrobot_firebeetle2_esp32e/src/mqtt_ota_test.cpp.o
...
Linking .pio/build/dfrobot_firebeetle2_esp32e/firmware.elf
Building .pio/build/dfrobot_firebeetle2_esp32e/firmware.bin
RAM:   [====      ]  XX.X% (used XXXXX bytes from XXXXXX bytes)
Flash: [======    ]  XX.X% (used XXXXXX bytes from XXXXXXX bytes)
```

### 内存使用检查

启用MQTT OTA功能后，预期内存使用：
- Flash增加约15-20KB
- RAM增加约5-10KB

## 功能测试

### 1. 基本连接测试

上传固件后，打开串口监视器（115200波特率）：

```bash
pio device monitor --baud 115200
```

### 2. 串口调试命令

在串口监视器中输入以下命令进行测试：

```
help                 - 显示所有可用命令
mqtt_test           - 测试MQTT连接
upgrade_msg_test    - 测试UpgradeMessage结构体
sys_info            - 显示系统信息
ota_status          - 显示OTA状态
ota_config          - 显示OTA配置
```

### 3. MQTT连接测试

输入 `mqtt_test` 命令，应该看到类似输出：

```
=== MQTT Connection Test ===
MQTT OTA Manager initialized successfully
Device ID: weather-display-AABBCCDDEEFF
OTA Topic: devices/weather-display-AABBCCDDEEFF/ota/upgrade
Status Topic: devices/weather-display-AABBCCDDEEFF/ota/status
```

### 4. UpgradeMessage结构体测试

输入 `upgrade_msg_test` 命令，测试消息结构体功能：

```
=== UpgradeMessage Structure Test ===
Testing initial state:
  Command: ''
  Version: ''
  Valid: false
  Status: Unknown

Testing setter methods:
  Command: 'upgrade'
  Version: '1.2.3'
  Download URL: 'https://example.com/firmware.bin'
  Force Update: true
  Min Battery: 40%
  ...
```

## 升级测试

### 1. 准备测试固件

创建一个简单的测试固件文件，或使用现有的编译输出：

```bash
# 复制编译好的固件作为测试文件
cp .pio/build/dfrobot_firebeetle2_esp32e/firmware.bin test-firmware.bin
```

### 2. 设置HTTP服务器

使用Python快速启动HTTP服务器：

```bash
# 在固件文件目录下启动HTTP服务器
python3 -m http.server 8080

# 或使用Node.js
npx http-server -p 8080
```

### 3. 发送升级指令

使用MQTT客户端发送升级消息：

```bash
# 使用mosquitto客户端
mosquitto_pub -h your-mqtt-server.com -t "devices/weather-display-AABBCCDDEEFF/ota/upgrade" -m '{
    "command": "upgrade",
    "version": "1.0.1",
    "download_url": "http://192.168.1.100:8080/test-firmware.bin",
    "force_update": false,
    "min_battery_level": 30,
    "description": "Test upgrade"
}'
```

### 4. 监控升级过程

在串口监视器中观察升级过程：

```
[MQTT-OTA-I] Processing upgrade: current_version -> 1.0.1
[MQTT-OTA-I] Checking battery level: 85%
[MQTT-OTA-I] Starting firmware download...
[MQTT-OTA-I] Download progress: 25%
[MQTT-OTA-I] Download progress: 50%
[MQTT-OTA-I] Download progress: 75%
[MQTT-OTA-I] Download progress: 100%
[MQTT-OTA-I] Firmware verification successful
[MQTT-OTA-I] Starting OTA update...
[MQTT-OTA-I] OTA update successful, restarting...
```

## 故障排除

### 编译错误

**错误：`mqtt_ota_config.h: No such file or directory`**
- 确保已启用 `-DMQTT_OTA_UPGRADE` 编译标志

**错误：`PubSubClient.h: No such file or directory`**
- 检查 `platformio.ini` 中是否包含 `knolleary/PubSubClient @ 2.8`

**错误：`secrets.h: No such file or directory`**
- 复制 `secrets.h.example` 到 `secrets.h` 并配置MQTT参数

### 运行时错误

**MQTT连接失败**
- 检查MQTT服务器地址和端口配置
- 验证网络连接和防火墙设置
- 确认用户名密码正确（如果需要认证）

**升级消息解析失败**
- 检查JSON格式是否正确
- 验证必需字段是否完整
- 查看调试日志了解具体错误

**固件下载失败**
- 确认HTTP服务器可访问
- 检查固件文件是否存在
- 验证设备网络连接

### 调试技巧

1. **增加调试级别**：
   ```ini
   -DMQTT_OTA_DEBUG_LEVEL=4  # 最详细的日志
   ```

2. **使用串口命令**：
   ```
   ota_status   # 查看当前状态
   ota_config   # 查看配置信息
   sys_info     # 查看系统信息
   ```

3. **监控内存使用**：
   ```cpp
   Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
   ```

## 性能测试

### 功耗测试

测量启用MQTT OTA功能前后的功耗差异：

1. 记录正常工作模式下的电流消耗
2. 启用MQTT OTA功能后重新测量
3. 计算增加的功耗和对电池续航的影响

### 内存使用测试

```cpp
// 在代码中添加内存监控
void printMemoryUsage() {
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Largest free block: %d bytes\n", ESP.getMaxAllocHeap());
    Serial.printf("Total heap size: %d bytes\n", ESP.getHeapSize());
}
```

### 网络流量测试

监控MQTT检查过程中的网络流量：
- 连接握手流量
- 消息订阅流量
- 升级指令接收流量
- 固件下载流量

## 部署建议

### 生产环境配置

1. **关闭详细调试**：
   ```ini
   -DMQTT_OTA_DEBUG_LEVEL=1  # 仅错误和警告
   ```

2. **配置SSL/TLS**（如果支持）：
   ```cpp
   #define SECRET_MQTT_OTA_USE_SSL true
   #define SECRET_MQTT_OTA_PORT 8883
   ```

3. **设置合理的超时**：
   ```cpp
   mqttOTAConfig.connectionTimeout = 10000;  // 10秒
   mqttOTAConfig.messageTimeout = 15000;     // 15秒
   ```

### 监控和维护

1. 定期检查设备升级状态
2. 监控升级成功率
3. 收集升级失败日志
4. 定期更新MQTT服务器证书（如使用SSL）

## 总结

通过以上步骤，你可以成功编译、部署和测试MQTT OTA升级功能。该功能提供了可靠的远程升级能力，同时保持了良好的功耗控制和错误处理机制。