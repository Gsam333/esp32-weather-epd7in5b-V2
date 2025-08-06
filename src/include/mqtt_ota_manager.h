#ifndef MQTT_OTA_MANAGER_H
#define MQTT_OTA_MANAGER_H

#ifdef MQTT_OTA_UPGRADE

#include "mqtt_ota_config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <Update.h>
#include <WiFi.h>

class MQTTOTAManager {
private:
  WiFiClient wifiClient;
  PubSubClient mqttClient;
  MQTTOTAConfig *config;

  // 状态变量
  OTAStatus currentStatus;
  OTAError lastError;
  String lastErrorMessage;
  unsigned long operationStartTime;
  bool messageReceived;
  UpgradeMessage pendingUpgrade;

  // 内部方法
  bool connectMQTT();
  void disconnectMQTT();
  static void mqttCallback(char *topic, byte *payload, unsigned int length);
  bool parseUpgradeMessage(const String &json, UpgradeMessage &message);
  bool validateUpgradeMessage(const UpgradeMessage &message);
  bool isNewerVersion(const String &newVersion, const String &currentVersion);
  bool checkBatteryLevel(int requiredLevel);
  bool checkFlashSpace(size_t requiredSize);
  bool downloadFirmware(const String &url, const String &expectedChecksum);
  bool verifyFirmware(const uint8_t *data, size_t length,
                      const String &expectedChecksum);
  bool installFirmware();
  void sendStatusReport(OTAStatus status, const String &message = "");
  void updateStatus(OTAStatus status, OTAError error = OTA_ERROR_NONE,
                    const String &errorMsg = "");

  // 静态实例指针（用于回调函数）
  static MQTTOTAManager *instance;

public:
  MQTTOTAManager();
  ~MQTTOTAManager();

  // 公共接口
  bool begin(MQTTOTAConfig *cfg);
  bool checkForUpgrade();
  void handleUpgradeMessage(const UpgradeMessage &message);

  // 状态查询
  OTAStatus getStatus() const { return currentStatus; }
  OTAError getLastError() const { return lastError; }
  String getLastErrorMessage() const { return lastErrorMessage; }

  // 工具方法
  void printStatus();
  void printConfig();
};

// 全局实例声明
extern MQTTOTAManager mqttOTAManager;

#endif // MQTT_OTA_UPGRADE

#endif // MQTT_OTA_MANAGER_H