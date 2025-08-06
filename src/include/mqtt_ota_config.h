#ifndef MQTT_OTA_CONFIG_H
#define MQTT_OTA_CONFIG_H

#ifdef MQTT_OTA_UPGRADE

#include "secrets.h"
#include <Arduino.h>

// MQTT OTA调试级别定义
#ifndef MQTT_OTA_DEBUG_LEVEL
#define MQTT_OTA_DEBUG_LEVEL 1 // 默认信息级别
#endif

// 调试日志宏定义
#define MQTT_OTA_LOG_E(format, ...)                                            \
  if (MQTT_OTA_DEBUG_LEVEL >= 0)                                               \
  Serial.printf("[MQTT-OTA-E] " format "\n", ##__VA_ARGS__)
#define MQTT_OTA_LOG_W(format, ...)                                            \
  if (MQTT_OTA_DEBUG_LEVEL >= 1)                                               \
  Serial.printf("[MQTT-OTA-W] " format "\n", ##__VA_ARGS__)
#define MQTT_OTA_LOG_I(format, ...)                                            \
  if (MQTT_OTA_DEBUG_LEVEL >= 2)                                               \
  Serial.printf("[MQTT-OTA-I] " format "\n", ##__VA_ARGS__)
#define MQTT_OTA_LOG_D(format, ...)                                            \
  if (MQTT_OTA_DEBUG_LEVEL >= 3)                                               \
  Serial.printf("[MQTT-OTA-D] " format "\n", ##__VA_ARGS__)
#define MQTT_OTA_LOG_V(format, ...)                                            \
  if (MQTT_OTA_DEBUG_LEVEL >= 4)                                               \
  Serial.printf("[MQTT-OTA-V] " format "\n", ##__VA_ARGS__)

// MQTT OTA配置结构体
struct MQTTOTAConfig {
  // MQTT服务器配置 - 从secrets.h获取
  String mqttServer = SECRET_MQTT_OTA_SERVER;
  int mqttPort = SECRET_MQTT_OTA_PORT;
  String mqttUsername = SECRET_MQTT_OTA_USERNAME;
  String mqttPassword = SECRET_MQTT_OTA_PASSWORD;
  bool enableSSL = SECRET_MQTT_OTA_USE_SSL;

  // 设备配置
  String deviceId = "weather-display-001";
  String otaTopic = "";    // 将自动生成为 devices/{deviceId}/ota/upgrade
  String statusTopic = ""; // 将自动生成为 devices/{deviceId}/ota/status

  // 连接配置
  int connectionTimeout = 5000; // MQTT连接超时 (ms)
  int messageTimeout = 10000;   // 消息等待超时 (ms)
  int maxRetries = 3;           // 最大重试次数

  // 升级配置
  bool enableOTA = true;       // 是否启用OTA功能
  int minBatteryLevel = 30;    // 最低电池电量要求 (%)
  bool allowDowngrade = false; // 是否允许版本降级

  // 自动生成topic名称
  void generateTopics() {
    if (otaTopic.isEmpty()) {
      otaTopic = "devices/" + deviceId + "/ota/upgrade";
    }
    if (statusTopic.isEmpty()) {
      statusTopic = "devices/" + deviceId + "/ota/status";
    }
  }
};

// 升级消息状态枚举
enum UpgradeMessageStatus {
  MSG_STATUS_UNKNOWN = 0,
  MSG_STATUS_RECEIVED = 1,
  MSG_STATUS_VALIDATED = 2,
  MSG_STATUS_PROCESSING = 3,
  MSG_STATUS_COMPLETED = 4,
  MSG_STATUS_FAILED = 5
};

// 升级消息结构体
struct UpgradeMessage {
private:
  String _command = "";
  String _version = "";
  String _downloadUrl = "";
  bool _forceUpdate = false;
  int _minBatteryLevel = 30;
  String _checksum = "";
  String _description = "";
  unsigned long _timestamp = 0;
  UpgradeMessageStatus _status = MSG_STATUS_UNKNOWN;
  String _errorMessage = "";

public:
  // 构造函数
  UpgradeMessage() = default;

  // 拷贝构造函数
  UpgradeMessage(const UpgradeMessage &other) = default;

  // 赋值操作符
  UpgradeMessage &operator=(const UpgradeMessage &other) = default;

  // Getter方法
  const String &getCommand() const { return _command; }
  const String &getVersion() const { return _version; }
  const String &getDownloadUrl() const { return _downloadUrl; }
  bool getForceUpdate() const { return _forceUpdate; }
  int getMinBatteryLevel() const { return _minBatteryLevel; }
  const String &getChecksum() const { return _checksum; }
  const String &getDescription() const { return _description; }
  unsigned long getTimestamp() const { return _timestamp; }
  UpgradeMessageStatus getStatus() const { return _status; }
  const String &getErrorMessage() const { return _errorMessage; }

  // Setter方法
  void setCommand(const String &command) { _command = command; }
  void setVersion(const String &version) { _version = version; }
  void setDownloadUrl(const String &url) { _downloadUrl = url; }
  void setForceUpdate(bool force) { _forceUpdate = force; }
  void setMinBatteryLevel(int level) { _minBatteryLevel = level; }
  void setChecksum(const String &checksum) { _checksum = checksum; }
  void setDescription(const String &description) { _description = description; }
  void setTimestamp(unsigned long timestamp) { _timestamp = timestamp; }
  void setStatus(UpgradeMessageStatus status) { _status = status; }
  void setErrorMessage(const String &error) { _errorMessage = error; }

  // 便利方法
  void clear() {
    _command = "";
    _version = "";
    _downloadUrl = "";
    _forceUpdate = false;
    _minBatteryLevel = 30;
    _checksum = "";
    _description = "";
    _timestamp = 0;
    _status = MSG_STATUS_UNKNOWN;
    _errorMessage = "";
  }

  // 验证消息完整性
  bool isValid() const {
    return !_command.isEmpty() && !_version.isEmpty() &&
           !_downloadUrl.isEmpty() && _command == "upgrade" &&
           _minBatteryLevel >= 0 && _minBatteryLevel <= 100;
  }

  // 检查是否为强制更新
  bool isForcedUpdate() const { return _forceUpdate; }

  // 检查是否有校验和
  bool hasChecksum() const { return !_checksum.isEmpty(); }

  // 获取消息年龄（秒）
  unsigned long getAgeInSeconds() const {
    if (_timestamp == 0)
      return 0;
    return (millis() - _timestamp) / 1000;
  }

  // 检查消息是否过期（默认10分钟）
  bool isExpired(unsigned long maxAgeSeconds = 600) const {
    return getAgeInSeconds() > maxAgeSeconds;
  }

  // 设置当前时间戳
  void setCurrentTimestamp() { _timestamp = millis(); }

  // 调试输出
  String toString() const {
    String result = "UpgradeMessage{";
    result += "command=" + _command;
    result += ", version=" + _version;
    result += ", url=" + _downloadUrl;
    result += ", force=" + String(_forceUpdate ? "true" : "false");
    result += ", minBattery=" + String(_minBatteryLevel);
    result +=
        ", checksum=" +
        (_checksum.isEmpty() ? "none" : _checksum.substring(0, 8) + "...");
    result += ", status=" + String((int)_status);
    result += ", age=" + String(getAgeInSeconds()) + "s";
    result += "}";
    return result;
  }
};

// OTA状态枚举
enum OTAStatus {
  OTA_IDLE = 0,
  OTA_CHECKING = 1,
  OTA_DOWNLOADING = 2,
  OTA_VERIFYING = 3,
  OTA_INSTALLING = 4,
  OTA_SUCCESS = 5,
  OTA_FAILED = 6
};

// OTA错误代码
enum OTAError {
  OTA_ERROR_NONE = 0,
  OTA_ERROR_MQTT_CONNECTION = 1,
  OTA_ERROR_INVALID_MESSAGE = 2,
  OTA_ERROR_LOW_BATTERY = 3,
  OTA_ERROR_DOWNLOAD_FAILED = 4,
  OTA_ERROR_VERIFICATION_FAILED = 5,
  OTA_ERROR_INSTALLATION_FAILED = 6,
  OTA_ERROR_VERSION_DOWNGRADE = 7,
  OTA_ERROR_INSUFFICIENT_SPACE = 8,
  OTA_ERROR_TIMEOUT = 9
};

// 获取错误描述
const char *getOTAErrorString(OTAError error);

// 获取状态描述
const char *getOTAStatusString(OTAStatus status);

// 获取升级消息状态描述
const char *getUpgradeMessageStatusString(UpgradeMessageStatus status);

// 配置验证函数
bool validateMQTTOTAConfig(const MQTTOTAConfig &config);

// 打印配置信息（用于调试）
void printMQTTOTAConfig(const MQTTOTAConfig &config);

// 全局配置实例声明
extern MQTTOTAConfig mqttOTAConfig;

#endif // MQTT_OTA_UPGRADE

#endif // MQTT_OTA_CONFIG_H