# MQTT OTA升级功能使用指南

## 功能启用

### 1. 编译时启用功能

在 `platformio.ini` 文件中取消注释以下行：

```ini
; MQTT OTA功能开关（默认关闭，需要时取消注释）
-DMQTT_OTA_UPGRADE
-DMQTT_OTA_DEBUG_LEVEL=2
```

### 2. 配置MQTT服务器

#### 2.1 复制配置模板

首先，复制配置模板文件：

```bash
cp src/include/secrets.h.example src/include/secrets.h
```

#### 2.2 修改MQTT配置

在 `src/include/secrets.h` 中修改MQTT服务器配置：

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

#### 2.3 常见配置示例

**本地MQTT服务器（无认证）：**
```cpp
#define SECRET_MQTT_OTA_SERVER "192.168.1.100"
#define SECRET_MQTT_OTA_PORT 1883
#define SECRET_MQTT_OTA_USERNAME ""
#define SECRET_MQTT_OTA_PASSWORD ""
#define SECRET_MQTT_OTA_USE_SSL false
```

**Home Assistant MQTT：**
```cpp
#define SECRET_MQTT_OTA_SERVER "homeassistant.local"
#define SECRET_MQTT_OTA_PORT 1883
#define SECRET_MQTT_OTA_USERNAME "mqtt_user"
#define SECRET_MQTT_OTA_PASSWORD "mqtt_password"
#define SECRET_MQTT_OTA_USE_SSL false
```

**SSL/TLS加密连接：**
```cpp
#define SECRET_MQTT_OTA_SERVER "secure-mqtt-broker.com"
#define SECRET_MQTT_OTA_PORT 8883
#define SECRET_MQTT_OTA_USERNAME "your_username"
#define SECRET_MQTT_OTA_PASSWORD "your_password"
#define SECRET_MQTT_OTA_USE_SSL true
```

#### 2.4 配置验证

系统会在启动时自动验证配置参数：
- MQTT服务器地址不能为空或默认值
- 端口必须在1-65535范围内
- 设备ID不能为空
- SSL配置与端口的合理性检查

**注意：** `secrets.h` 文件包含敏感信息，确保它已添加到 `.gitignore` 中，不会被提交到版本控制系统。

### 3. 编译和上传

```bash
# 编译项目
platformio run

# 上传到设备
platformio run --target upload
```

## MQTT消息格式

### 升级指令消息

**Topic:** `devices/{device_id}/ota/upgrade`

**消息格式：**
```json
{
    "command": "upgrade",
    "version": "1.2.0",
    "download_url": "https://your-server.com/firmware/weather-v1.2.0.bin",
    "force_update": false,
    "min_battery_level": 30,
    "checksum": "sha256:abcdef123456789...",
    "timestamp": 1640995200,
    "description": "Bug fixes and performance improvements"
}
```

**字段说明：**
- `command`: 必须为 "upgrade"
- `version`: 新版本号
- `download_url`: 固件下载URL
- `force_update`: 是否强制升级（忽略电池电量检查）
- `min_battery_level`: 最低电池电量要求（百分比）
- `checksum`: 固件校验和（可选）
- `timestamp`: 时间戳（可选）
- `description`: 升级描述（可选）

### 设备ID生成规则

设备ID格式：`weather-display-{MAC地址}`

例如：`weather-display-AABBCCDDEEFF`

### Topic命名规则

- 升级指令topic: `devices/{device_id}/ota/upgrade`
- 状态报告topic: `devices/{device_id}/ota/status`

## 使用流程

### 1. 设备启动

设备启动后会：
1. 连接WiFi
2. 同步时间
3. 连接MQTT服务器
4. 订阅升级topic
5. 等待10秒接收升级消息
6. 断开MQTT连接
7. 继续正常的天气更新流程

### 2. 发送升级指令

通过MQTT客户端向设备发送升级消息：

```bash
# 使用mosquitto客户端发送升级指令
mosquitto_pub -h your-mqtt-server.com -t "devices/weather-display-AABBCCDDEEFF/ota/upgrade" -m '{
    "command": "upgrade",
    "version": "1.2.0",
    "download_url": "https://your-server.com/firmware.bin",
    "force_update": false,
    "min_battery_level": 30
}'
```

### 3. 升级过程

设备收到升级指令后会：
1. 验证消息格式
2. 检查版本号
3. 检查电池电量
4. 下载固件
5. 验证固件完整性
6. 安装固件
7. 重启设备

## 调试和监控

### 串口调试

启用调试后，可以通过串口监控升级过程：

```
[MQTT-OTA-I] Starting MQTT OTA check...
[MQTT-OTA-D] MQTT Config: server=mqtt.example.com, port=1883
[MQTT-OTA-I] MQTT connected successfully
[MQTT-OTA-D] Subscribed to topic: devices/weather-display-AABBCC/ota/upgrade
[MQTT-OTA-I] Received MQTT message on topic: devices/weather-display-AABBCC/ota/upgrade
[MQTT-OTA-I] Upgrade available: 1.1.0 -> 1.2.0
[MQTT-OTA-I] Starting firmware download...
[MQTT-OTA-I] Firmware download completed
[MQTT-OTA-I] Installing firmware...
[MQTT-OTA-I] OTA upgrade completed successfully, restarting...
```

### 调试级别

在 `platformio.ini` 中设置调试级别：

```ini
-DMQTT_OTA_DEBUG_LEVEL=0  # 仅错误信息
-DMQTT_OTA_DEBUG_LEVEL=1  # 警告信息
-DMQTT_OTA_DEBUG_LEVEL=2  # 基本信息（推荐）
-DMQTT_OTA_DEBUG_LEVEL=3  # 详细调试信息
-DMQTT_OTA_DEBUG_LEVEL=4  # 完整详细信息
```

### 串口命令

在串口监视器中输入以下命令进行调试：

```
help          - 显示帮助信息
mqtt_test     - 测试MQTT连接
msg_test      - 测试消息解析
sys_info      - 显示系统信息
ota_status    - 显示OTA状态
ota_config    - 显示OTA配置
```

## 测试和调试

### 串口调试命令

连接设备串口（115200波特率），可以使用以下调试命令：

```
help                 - 显示所有可用命令
mqtt_test           - 测试MQTT连接
msg_test            - 测试消息解析
upgrade_msg_test    - 测试UpgradeMessage结构体（新增）
sys_info            - 显示系统信息
ota_status          - 显示OTA状态
ota_config          - 显示OTA配置
```

### UpgradeMessage结构体测试

新增的 `upgrade_msg_test` 命令会测试：
- 消息创建和设置
- Getter/Setter方法
- 消息验证逻辑
- 状态管理
- 消息年龄和过期检查
- 拷贝构造和清理操作
- 错误处理场景

### 调试日志级别

在 `platformio.ini` 中设置调试级别：

```ini
# 调试级别设置
-DMQTT_OTA_DEBUG_LEVEL=0  # 仅错误
-DMQTT_OTA_DEBUG_LEVEL=1  # 错误和警告
-DMQTT_OTA_DEBUG_LEVEL=2  # 错误、警告和信息（推荐）
-DMQTT_OTA_DEBUG_LEVEL=3  # 错误、警告、信息和调试
-DMQTT_OTA_DEBUG_LEVEL=4  # 所有日志（详细）
```

### 测试升级消息格式

可以使用以下JSON格式测试消息解析：

```json
{
    "command": "upgrade",
    "version": "1.2.3",
    "download_url": "https://example.com/test.bin",
    "force_update": true,
    "min_battery_level": 20,
    "checksum": "sha256:test123",
    "description": "Test upgrade",
    "timestamp": 1640995200
}
```

## 故障排除

### 常见问题

#### 1. MQTT连接失败
```
[MQTT-OTA-E] MQTT connection failed: Connection refused (code: -2)
```

**解决方案：**
- 检查MQTT服务器地址和端口
- 验证网络连接
- 检查用户名密码（如果需要认证）

#### 2. 固件下载失败
```
[MQTT-OTA-E] HTTP GET failed: 404
```

**解决方案：**
- 验证固件URL是否正确
- 检查固件文件是否存在
- 确认服务器可访问

#### 3. 电池电量不足
```
[MQTT-OTA-W] Battery level too low for upgrade: 25% < 30%
```

**解决方案：**
- 充电后重试
- 设置 `force_update: true` 强制升级
- 降低 `min_battery_level` 要求

#### 4. 版本降级被拒绝
```
[MQTT-OTA-W] New version (1.1.0) is not newer than current (1.2.0)
```

**解决方案：**
- 检查版本号是否正确
- 在配置中启用 `MQTT_OTA_ALLOW_DOWNGRADE`

### 日志分析

查看完整的调试日志来诊断问题：

1. 设置调试级别为3或4
2. 重新编译和上传
3. 监控串口输出
4. 分析错误信息和状态变化

## 安全考虑

### 1. 固件验证

建议启用固件校验和验证：

```json
{
    "checksum": "sha256:your_firmware_sha256_hash"
}
```

### 2. MQTT安全

- 使用SSL/TLS加密连接
- 配置MQTT用户名密码认证
- 限制topic访问权限

### 3. 固件分发

- 使用HTTPS服务器分发固件
- 实现访问控制和认证
- 定期更新SSL证书

## 性能影响

### 功耗影响

- 每次唤醒增加约10秒的MQTT检查时间
- 日功耗增加约2-3mAh（约20%）
- 电池续航从12个月减少到约10个月

### 内存使用

- 代码增加约15-20KB Flash使用
- 运行时增加约5-10KB RAM使用
- JSON解析需要临时内存约1KB

### 网络使用

- 每次检查消耗约1-2KB数据流量
- 固件下载根据固件大小（通常1-2MB）
- MQTT心跳消息很小（<100字节）

## 最佳实践

### 1. 升级策略

- 在设备电量充足时进行升级
- 选择网络稳定的时间段
- 分批升级，避免同时升级所有设备

### 2. 固件管理

- 使用语义化版本号
- 保留多个版本以支持回滚
- 测试固件后再发布

### 3. 监控和维护

- 监控升级成功率
- 收集升级失败日志
- 定期检查MQTT服务器状态

### 4. 功耗优化

- 根据需要调整检查频率
- 在低电量时禁用OTA检查
- 使用快速MQTT连接设置