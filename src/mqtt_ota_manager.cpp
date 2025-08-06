#ifdef MQTT_OTA_UPGRADE

#include "mqtt_ota_manager.h"
#include "config.h"
#include "display_utils.h" // 用于电池电量函数
#include <Preferences.h>
#include <esp_system.h>

// 静态成员初始化
MQTTOTAManager *MQTTOTAManager::instance = nullptr;

// 全局实例
MQTTOTAManager mqttOTAManager;
MQTTOTAConfig mqttOTAConfig;

// 错误描述字符串
const char *getOTAErrorString(OTAError error) {
  switch (error) {
  case OTA_ERROR_NONE:
    return "No error";
  case OTA_ERROR_MQTT_CONNECTION:
    return "MQTT connection failed";
  case OTA_ERROR_INVALID_MESSAGE:
    return "Invalid upgrade message";
  case OTA_ERROR_LOW_BATTERY:
    return "Battery level too low";
  case OTA_ERROR_DOWNLOAD_FAILED:
    return "Firmware download failed";
  case OTA_ERROR_VERIFICATION_FAILED:
    return "Firmware verification failed";
  case OTA_ERROR_INSTALLATION_FAILED:
    return "Firmware installation failed";
  case OTA_ERROR_VERSION_DOWNGRADE:
    return "Version downgrade not allowed";
  case OTA_ERROR_INSUFFICIENT_SPACE:
    return "Insufficient flash space";
  case OTA_ERROR_TIMEOUT:
    return "Operation timeout";
  default:
    return "Unknown error";
  }
}

// 状态描述字符串
const char *getOTAStatusString(OTAStatus status) {
  switch (status) {
  case OTA_IDLE:
    return "Idle";
  case OTA_CHECKING:
    return "Checking for updates";
  case OTA_DOWNLOADING:
    return "Downloading firmware";
  case OTA_VERIFYING:
    return "Verifying firmware";
  case OTA_INSTALLING:
    return "Installing firmware";
  case OTA_SUCCESS:
    return "Update successful";
  case OTA_FAILED:
    return "Update failed";
  default:
    return "Unknown status";
  }
}

// 升级消息状态描述字符串
const char *getUpgradeMessageStatusString(UpgradeMessageStatus status) {
  switch (status) {
  case MSG_STATUS_UNKNOWN:
    return "Unknown";
  case MSG_STATUS_RECEIVED:
    return "Received";
  case MSG_STATUS_VALIDATED:
    return "Validated";
  case MSG_STATUS_PROCESSING:
    return "Processing";
  case MSG_STATUS_COMPLETED:
    return "Completed";
  case MSG_STATUS_FAILED:
    return "Failed";
  default:
    return "Invalid status";
  }
}

MQTTOTAManager::MQTTOTAManager()
    : mqttClient(wifiClient), config(nullptr), currentStatus(OTA_IDLE),
      lastError(OTA_ERROR_NONE), operationStartTime(0), messageReceived(false) {
  instance = this;
}

MQTTOTAManager::~MQTTOTAManager() {
  if (mqttClient.connected()) {
    mqttClient.disconnect();
  }
  instance = nullptr;
}

bool MQTTOTAManager::begin(MQTTOTAConfig *cfg) {
  if (!cfg) {
    MQTT_OTA_LOG_E("Invalid configuration");
    return false;
  }

  // 验证配置参数
  if (!validateMQTTOTAConfig(*cfg)) {
    MQTT_OTA_LOG_E("MQTT OTA configuration validation failed");
    return false;
  }

  config = cfg;
  config->generateTopics();

  // 在调试模式下打印配置信息
  if (MQTT_OTA_DEBUG_LEVEL >= 3) {
    printMQTTOTAConfig(*config);
  }

  MQTT_OTA_LOG_I("MQTT OTA Manager initialized successfully");
  MQTT_OTA_LOG_D("Device ID: %s", config->deviceId.c_str());
  MQTT_OTA_LOG_D("OTA Topic: %s", config->otaTopic.c_str());
  MQTT_OTA_LOG_D("Status Topic: %s", config->statusTopic.c_str());

  return true;
}

bool MQTTOTAManager::checkForUpgrade() {
  if (!config || !config->enableOTA) {
    MQTT_OTA_LOG_D("MQTT OTA disabled");
    return false;
  }

  MQTT_OTA_LOG_I("Starting MQTT OTA check...");
  updateStatus(OTA_CHECKING);
  operationStartTime = millis();

  // 连接MQTT服务器
  if (!connectMQTT()) {
    updateStatus(OTA_IDLE, OTA_ERROR_MQTT_CONNECTION,
                 "Failed to connect to MQTT server");
    return false;
  }

  // 等待升级消息
  messageReceived = false;
  unsigned long startTime = millis();

  while (millis() - startTime < config->messageTimeout && !messageReceived) {
    mqttClient.loop();
    delay(100);
  }

  // 断开MQTT连接
  disconnectMQTT();

  if (messageReceived) {
    MQTT_OTA_LOG_I("Upgrade message received, processing...");
    handleUpgradeMessage(pendingUpgrade);
    return true;
  } else {
    MQTT_OTA_LOG_I("No upgrade message received");
    updateStatus(OTA_IDLE);
    return false;
  }
}

bool MQTTOTAManager::connectMQTT() {
  MQTT_OTA_LOG_D("Connecting to MQTT server: %s:%d", config->mqttServer.c_str(),
                 config->mqttPort);

  mqttClient.setServer(config->mqttServer.c_str(), config->mqttPort);
  mqttClient.setCallback(mqttCallback);

  unsigned long startTime = millis();

  while (!mqttClient.connected() &&
         millis() - startTime < config->connectionTimeout) {
    String clientId = config->deviceId + "-" + String(random(0xffff), HEX);

    bool connected;
    if (config->mqttUsername.isEmpty()) {
      connected = mqttClient.connect(clientId.c_str());
    } else {
      connected =
          mqttClient.connect(clientId.c_str(), config->mqttUsername.c_str(),
                             config->mqttPassword.c_str());
    }

    if (connected) {
      MQTT_OTA_LOG_I("MQTT connected successfully");

      // 订阅OTA topic
      if (mqttClient.subscribe(config->otaTopic.c_str())) {
        MQTT_OTA_LOG_D("Subscribed to topic: %s", config->otaTopic.c_str());
        return true;
      } else {
        MQTT_OTA_LOG_E("Failed to subscribe to OTA topic");
        return false;
      }
    } else {
      MQTT_OTA_LOG_W("MQTT connection failed, state: %d", mqttClient.state());
      delay(1000);
    }
  }

  MQTT_OTA_LOG_E("MQTT connection timeout");
  return false;
}

void MQTTOTAManager::disconnectMQTT() {
  if (mqttClient.connected()) {
    mqttClient.disconnect();
    MQTT_OTA_LOG_D("MQTT disconnected");
  }
}

void MQTTOTAManager::mqttCallback(char *topic, byte *payload,
                                  unsigned int length) {
  if (!instance)
    return;

  String message = String((char *)payload).substring(0, length);
  String topicStr = String(topic);

  MQTT_OTA_LOG_I("Received MQTT message on topic: %s", topic);
  MQTT_OTA_LOG_V("Message content: %s", message.c_str());

  if (topicStr == instance->config->otaTopic) {
    if (instance->parseUpgradeMessage(message, instance->pendingUpgrade)) {
      instance->messageReceived = true;
    }
  }
}

bool MQTTOTAManager::parseUpgradeMessage(const String &json,
                                         UpgradeMessage &message) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    MQTT_OTA_LOG_E("Failed to parse JSON message: %s", error.c_str());
    message.setStatus(MSG_STATUS_FAILED);
    message.setErrorMessage("JSON parsing failed: " + String(error.c_str()));
    return false;
  }

  // 清空消息并设置状态
  message.clear();
  message.setStatus(MSG_STATUS_RECEIVED);
  message.setCurrentTimestamp();

  // 解析必需字段
  message.setCommand(doc["command"].as<String>());
  message.setVersion(doc["version"].as<String>());
  message.setDownloadUrl(doc["download_url"].as<String>());

  // 解析可选字段
  message.setForceUpdate(doc["force_update"] | false);
  message.setMinBatteryLevel(doc["min_battery_level"] | 30);
  message.setChecksum(doc["checksum"].as<String>());
  message.setDescription(doc["description"].as<String>());

  // 如果JSON中有时间戳，使用它，否则使用当前时间
  if (doc.containsKey("timestamp")) {
    message.setTimestamp(doc["timestamp"].as<unsigned long>());
  }

  MQTT_OTA_LOG_D("Parsed message: command=%s, version=%s, force=%s",
                 message.getCommand().c_str(), message.getVersion().c_str(),
                 message.getForceUpdate() ? "true" : "false");

  // 验证消息并更新状态
  bool isValid = validateUpgradeMessage(message);
  if (isValid) {
    message.setStatus(MSG_STATUS_VALIDATED);
  } else {
    message.setStatus(MSG_STATUS_FAILED);
  }

  return isValid;
}

bool MQTTOTAManager::validateUpgradeMessage(const UpgradeMessage &message) {
  // 使用结构体内置的验证方法
  if (!message.isValid()) {
    MQTT_OTA_LOG_E(
        "Invalid upgrade message: missing required fields or invalid values");
    return false;
  }

  if (message.getCommand() != "upgrade") {
    MQTT_OTA_LOG_E("Invalid command: %s", message.getCommand().c_str());
    return false;
  }

  // 检查消息是否过期（默认10分钟）
  if (message.isExpired()) {
    MQTT_OTA_LOG_E("Upgrade message expired (age: %lu seconds)",
                   message.getAgeInSeconds());
    return false;
  }

  // 检查下载URL格式
  String url = message.getDownloadUrl();
  if (!url.startsWith("http://") && !url.startsWith("https://")) {
    MQTT_OTA_LOG_E("Invalid download URL format: %s", url.c_str());
    return false;
  }

  // 检查版本格式（简单检查）
  String version = message.getVersion();
  if (version.length() == 0 || version.indexOf('.') == -1) {
    MQTT_OTA_LOG_E("Invalid version format: %s", version.c_str());
    return false;
  }

  MQTT_OTA_LOG_D("Upgrade message validation passed");
  return true;
}

void MQTTOTAManager::handleUpgradeMessage(const UpgradeMessage &message) {
  MQTT_OTA_LOG_I("Processing upgrade: %s -> %s", "current_version",
                 message.getVersion().c_str());

  // 打印消息详情
  MQTT_OTA_LOG_D("Upgrade message details: %s", message.toString().c_str());

  // 检查版本
  if (!config->allowDowngrade &&
      !isNewerVersion(message.getVersion(), "1.0.0")) {
    updateStatus(OTA_FAILED, OTA_ERROR_VERSION_DOWNGRADE,
                 "Version downgrade not allowed");
    return;
  }

  // 检查电池电量
  if (!message.getForceUpdate() &&
      !checkBatteryLevel(message.getMinBatteryLevel())) {
    updateStatus(OTA_FAILED, OTA_ERROR_LOW_BATTERY,
                 "Battery level too low for upgrade");
    return;
  }

  // 开始下载固件
  if (!downloadFirmware(message.getDownloadUrl(), message.getChecksum())) {
    updateStatus(OTA_FAILED, OTA_ERROR_DOWNLOAD_FAILED,
                 "Failed to download firmware");
    return;
  }

  // 安装固件
  if (!installFirmware()) {
    updateStatus(OTA_FAILED, OTA_ERROR_INSTALLATION_FAILED,
                 "Failed to install firmware");
    return;
  }

  // 升级成功
  updateStatus(OTA_SUCCESS);
  MQTT_OTA_LOG_I("OTA upgrade completed successfully, restarting...");

  delay(1000);
  ESP.restart();
}

bool MQTTOTAManager::isNewerVersion(const String &newVersion,
                                    const String &currentVersion) {
  // 简单的版本比较实现
  // 实际项目中应该实现更完善的语义化版本比较
  return newVersion != currentVersion;
}

bool MQTTOTAManager::checkBatteryLevel(int requiredLevel) {
#if BATTERY_MONITORING
  // 读取实际电池电量
  uint32_t batteryVoltage = readBatteryVoltage();
  int batteryPercent =
      calcBatPercent(batteryVoltage, MIN_BATTERY_VOLTAGE, MAX_BATTERY_VOLTAGE);

  MQTT_OTA_LOG_D("Battery level check: current=%d%%, required=%d%%",
                 batteryPercent, requiredLevel);

  return batteryPercent >= requiredLevel;
#else
  // 如果未启用电池监控，假设电量充足
  MQTT_OTA_LOG_D("Battery monitoring disabled, assuming sufficient power");
  return true;
#endif
}

bool MQTTOTAManager::checkFlashSpace(size_t requiredSize) {
  size_t freeSpace = ESP.getFreeSketchSpace();
  MQTT_OTA_LOG_D("Flash space check: required=%d, available=%d", requiredSize,
                 freeSpace);
  return freeSpace >= requiredSize;
}

bool MQTTOTAManager::downloadFirmware(const String &url,
                                      const String &expectedChecksum) {
  MQTT_OTA_LOG_I("Starting firmware download from: %s", url.c_str());
  updateStatus(OTA_DOWNLOADING);

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    MQTT_OTA_LOG_E("HTTP GET failed: %d", httpCode);
    http.end();
    return false;
  }

  int contentLength = http.getSize();
  if (contentLength <= 0) {
    MQTT_OTA_LOG_E("Invalid content length: %d", contentLength);
    http.end();
    return false;
  }

  if (!checkFlashSpace(contentLength)) {
    MQTT_OTA_LOG_E("Insufficient flash space");
    http.end();
    return false;
  }

  MQTT_OTA_LOG_I("Firmware size: %d bytes", contentLength);
  updateStatus(OTA_VERIFYING);

  if (!Update.begin(contentLength)) {
    MQTT_OTA_LOG_E("Cannot begin update: %s", Update.errorString());
    http.end();
    return false;
  }

  WiFiClient *client = http.getStreamPtr();
  size_t written = Update.writeStream(*client);

  if (written == contentLength) {
    MQTT_OTA_LOG_I("Firmware download completed: %d bytes", written);
  } else {
    MQTT_OTA_LOG_E("Download incomplete: %d/%d bytes", written, contentLength);
    Update.abort();
    http.end();
    return false;
  }

  http.end();
  return true;
}

bool MQTTOTAManager::verifyFirmware(const uint8_t *data, size_t length,
                                    const String &expectedChecksum) {
  // 固件验证实现
  // 这里可以添加SHA-256校验和验证
  MQTT_OTA_LOG_D("Verifying firmware (length: %d)", length);
  return true;
}

bool MQTTOTAManager::installFirmware() {
  MQTT_OTA_LOG_I("Installing firmware...");
  updateStatus(OTA_INSTALLING);

  if (Update.end(true)) {
    if (Update.isFinished()) {
      MQTT_OTA_LOG_I("Firmware installation successful");
      return true;
    } else {
      MQTT_OTA_LOG_E("Update not finished");
    }
  } else {
    MQTT_OTA_LOG_E("Update failed: %s", Update.errorString());
  }

  return false;
}

void MQTTOTAManager::sendStatusReport(OTAStatus status, const String &message) {
  // 发送状态报告到MQTT服务器
  // 实现状态报告功能
  MQTT_OTA_LOG_D("Status report: %s - %s", getOTAStatusString(status),
                 message.c_str());
}

void MQTTOTAManager::updateStatus(OTAStatus status, OTAError error,
                                  const String &errorMsg) {
  currentStatus = status;
  lastError = error;
  lastErrorMessage = errorMsg;

  if (error != OTA_ERROR_NONE) {
    MQTT_OTA_LOG_E("OTA Error: %s - %s", getOTAErrorString(error),
                   errorMsg.c_str());
  } else {
    MQTT_OTA_LOG_I("OTA Status: %s", getOTAStatusString(status));
  }
}

void MQTTOTAManager::printStatus() {
  Serial.println("=== MQTT OTA Status ===");
  Serial.printf("Status: %s\n", getOTAStatusString(currentStatus));
  Serial.printf("Last Error: %s\n", getOTAErrorString(lastError));
  if (!lastErrorMessage.isEmpty()) {
    Serial.printf("Error Message: %s\n", lastErrorMessage.c_str());
  }
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Free Sketch Space: %d bytes\n", ESP.getFreeSketchSpace());
}

void MQTTOTAManager::printConfig() {
  if (!config) {
    Serial.println("MQTT OTA Config: Not initialized");
    return;
  }

  Serial.println("=== MQTT OTA Configuration ===");
  Serial.printf("MQTT Server: %s:%d\n", config->mqttServer.c_str(),
                config->mqttPort);
  Serial.printf("Device ID: %s\n", config->deviceId.c_str());
  Serial.printf("OTA Topic: %s\n", config->otaTopic.c_str());
  Serial.printf("Status Topic: %s\n", config->statusTopic.c_str());
  Serial.printf("OTA Enabled: %s\n", config->enableOTA ? "Yes" : "No");
  Serial.printf("Min Battery Level: %d%%\n", config->minBatteryLevel);
  Serial.printf("Connection Timeout: %d ms\n", config->connectionTimeout);
  Serial.printf("Message Timeout: %d ms\n", config->messageTimeout);
}

// 配置验证函数
bool validateMQTTOTAConfig(const MQTTOTAConfig &config) {
  MQTT_OTA_LOG_D("Validating MQTT OTA configuration...");

  // 检查MQTT服务器地址
  if (config.mqttServer.isEmpty() || config.mqttServer == "mqtt.example.com") {
    MQTT_OTA_LOG_E("Invalid MQTT server address: %s",
                   config.mqttServer.c_str());
    return false;
  }

  // 检查端口范围
  if (config.mqttPort < 1 || config.mqttPort > 65535) {
    MQTT_OTA_LOG_E("Invalid MQTT port: %d", config.mqttPort);
    return false;
  }

  // 检查设备ID
  if (config.deviceId.isEmpty()) {
    MQTT_OTA_LOG_E("Device ID cannot be empty");
    return false;
  }

  // 检查SSL配置的端口合理性
  if (config.enableSSL && config.mqttPort == 1883) {
    MQTT_OTA_LOG_W(
        "SSL enabled but using non-SSL port 1883, consider using 8883");
  }

  // 检查超时配置
  if (config.connectionTimeout < 1000 || config.connectionTimeout > 30000) {
    MQTT_OTA_LOG_W("Connection timeout %d ms may be too short or too long",
                   config.connectionTimeout);
  }

  if (config.messageTimeout < 5000 || config.messageTimeout > 60000) {
    MQTT_OTA_LOG_W("Message timeout %d ms may be too short or too long",
                   config.messageTimeout);
  }

  // 检查电池电量要求
  if (config.minBatteryLevel < 0 || config.minBatteryLevel > 100) {
    MQTT_OTA_LOG_E("Invalid minimum battery level: %d%%",
                   config.minBatteryLevel);
    return false;
  }

  MQTT_OTA_LOG_I("MQTT OTA configuration validation passed");
  return true;
}

// 打印配置信息（用于调试）
void printMQTTOTAConfig(const MQTTOTAConfig &config) {
  MQTT_OTA_LOG_I("=== MQTT OTA Configuration ===");
  MQTT_OTA_LOG_I("MQTT Server: %s", config.mqttServer.c_str());
  MQTT_OTA_LOG_I("MQTT Port: %d", config.mqttPort);
  MQTT_OTA_LOG_I("MQTT Username: %s", config.mqttUsername.isEmpty()
                                          ? "(none)"
                                          : config.mqttUsername.c_str());
  MQTT_OTA_LOG_I("MQTT Password: %s",
                 config.mqttPassword.isEmpty() ? "(none)" : "***");
  MQTT_OTA_LOG_I("SSL Enabled: %s", config.enableSSL ? "Yes" : "No");
  MQTT_OTA_LOG_I("Device ID: %s", config.deviceId.c_str());
  MQTT_OTA_LOG_I("OTA Topic: %s", config.otaTopic.c_str());
  MQTT_OTA_LOG_I("Status Topic: %s", config.statusTopic.c_str());
  MQTT_OTA_LOG_I("Connection Timeout: %d ms", config.connectionTimeout);
  MQTT_OTA_LOG_I("Message Timeout: %d ms", config.messageTimeout);
  MQTT_OTA_LOG_I("Max Retries: %d", config.maxRetries);
  MQTT_OTA_LOG_I("OTA Enabled: %s", config.enableOTA ? "Yes" : "No");
  MQTT_OTA_LOG_I("Min Battery Level: %d%%", config.minBatteryLevel);
  MQTT_OTA_LOG_I("Allow Downgrade: %s", config.allowDowngrade ? "Yes" : "No");
  MQTT_OTA_LOG_I("=============================");
}

#endif // MQTT_OTA_UPGRADE