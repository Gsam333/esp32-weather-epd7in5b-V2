#ifdef MQTT_OTA_UPGRADE

// MQTT OTA功能测试文件
// 这个文件包含用于测试MQTT OTA功能的辅助函数

#include "config.h"
#include "mqtt_ota_manager.h"
#include <Arduino.h>

// 函数声明
void testUpgradeMessage();
void testMessageStatusEnum();
void runUpgradeMessageTests();

// 测试MQTT连接
void testMQTTConnection() {
  Serial.println("=== MQTT Connection Test ===");

  // 配置测试参数
  mqttOTAConfig.mqttServer = MQTT_OTA_SERVER;
  mqttOTAConfig.mqttPort = MQTT_OTA_PORT;
  mqttOTAConfig.deviceId = "test-device-001";
  mqttOTAConfig.generateTopics();

  // 初始化管理器
  if (mqttOTAManager.begin(&mqttOTAConfig)) {
    Serial.println("MQTT OTA Manager initialized successfully");
    mqttOTAManager.printConfig();
  } else {
    Serial.println("Failed to initialize MQTT OTA Manager");
  }
}

// 测试消息解析
void testMessageParsing() {
  Serial.println("=== Message Parsing Test ===");

  // 测试有效的升级消息
  String validMessage = R"({
        "command": "upgrade",
        "version": "1.2.0",
        "download_url": "https://example.com/firmware.bin",
        "force_update": false,
        "min_battery_level": 30,
        "checksum": "sha256:abcdef123456",
        "description": "Test upgrade"
    })";

  Serial.println("Testing valid message:");
  Serial.println(validMessage);

  // 测试无效的升级消息
  String invalidMessage = R"({
        "command": "invalid",
        "version": "1.2.0"
    })";

  Serial.println("Testing invalid message:");
  Serial.println(invalidMessage);
}

// 打印系统信息
void printSystemInfo() {
  Serial.println("=== System Information ===");
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Free Sketch Space: %d bytes\n", ESP.getFreeSketchSpace());
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
}

// 串口命令处理
void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "mqtt_test") {
      testMQTTConnection();
    } else if (command == "msg_test") {
      testMessageParsing();
    } else if (command == "sys_info") {
      printSystemInfo();
    } else if (command == "ota_status") {
      mqttOTAManager.printStatus();
    } else if (command == "ota_config") {
      mqttOTAManager.printConfig();
    } else if (command == "upgrade_msg_test") {
      runUpgradeMessageTests();
    } else if (command == "help") {
      Serial.println("Available commands:");
      Serial.println("  mqtt_test         - Test MQTT connection");
      Serial.println("  msg_test          - Test message parsing");
      Serial.println("  upgrade_msg_test  - Test UpgradeMessage structure");
      Serial.println("  sys_info          - Show system information");
      Serial.println("  ota_status        - Show OTA status");
      Serial.println("  ota_config        - Show OTA configuration");
      Serial.println("  help              - Show this help");
    } else if (command.length() > 0) {
      Serial.println("Unknown command: " + command);
      Serial.println("Type 'help' for available commands");
    }
  }
}

// 测试UpgradeMessage结构体
void testUpgradeMessage() {
  Serial.println("=== UpgradeMessage Structure Test ===");

  // 创建升级消息实例
  UpgradeMessage msg;

  // 测试初始状态
  Serial.println("Testing initial state:");
  Serial.printf("  Command: '%s'\n", msg.getCommand().c_str());
  Serial.printf("  Version: '%s'\n", msg.getVersion().c_str());
  Serial.printf("  Valid: %s\n", msg.isValid() ? "true" : "false");
  Serial.printf("  Status: %s\n",
                getUpgradeMessageStatusString(msg.getStatus()));

  // 测试设置值
  Serial.println("\nTesting setter methods:");
  msg.setCommand("upgrade");
  msg.setVersion("1.2.3");
  msg.setDownloadUrl("https://example.com/firmware.bin");
  msg.setForceUpdate(true);
  msg.setMinBatteryLevel(40);
  msg.setChecksum("sha256:abcdef123456789");
  msg.setDescription("Test firmware update");
  msg.setCurrentTimestamp();
  msg.setStatus(MSG_STATUS_RECEIVED);

  // 测试getter方法
  Serial.printf("  Command: '%s'\n", msg.getCommand().c_str());
  Serial.printf("  Version: '%s'\n", msg.getVersion().c_str());
  Serial.printf("  Download URL: '%s'\n", msg.getDownloadUrl().c_str());
  Serial.printf("  Force Update: %s\n",
                msg.getForceUpdate() ? "true" : "false");
  Serial.printf("  Min Battery: %d%%\n", msg.getMinBatteryLevel());
  Serial.printf("  Checksum: '%s'\n", msg.getChecksum().c_str());
  Serial.printf("  Description: '%s'\n", msg.getDescription().c_str());
  Serial.printf("  Status: %s\n",
                getUpgradeMessageStatusString(msg.getStatus()));

  // 测试验证方法
  Serial.println("\nTesting validation methods:");
  Serial.printf("  Valid: %s\n", msg.isValid() ? "true" : "false");
  Serial.printf("  Forced: %s\n", msg.isForcedUpdate() ? "true" : "false");
  Serial.printf("  Has Checksum: %s\n", msg.hasChecksum() ? "true" : "false");
  Serial.printf("  Age: %lu seconds\n", msg.getAgeInSeconds());
  Serial.printf("  Expired: %s\n", msg.isExpired() ? "true" : "false");

  // 测试toString方法
  Serial.println("\nTesting toString method:");
  Serial.printf("  %s\n", msg.toString().c_str());

  // 测试拷贝构造函数
  Serial.println("\nTesting copy constructor:");
  UpgradeMessage msgCopy = msg;
  Serial.printf("  Copy valid: %s\n", msgCopy.isValid() ? "true" : "false");
  Serial.printf("  Copy version: '%s'\n", msgCopy.getVersion().c_str());

  // 测试clear方法
  Serial.println("\nTesting clear method:");
  msg.clear();
  Serial.printf("  After clear - Valid: %s\n",
                msg.isValid() ? "true" : "false");
  Serial.printf("  After clear - Command: '%s'\n", msg.getCommand().c_str());
  Serial.printf("  After clear - Status: %s\n",
                getUpgradeMessageStatusString(msg.getStatus()));

  // 测试无效消息
  Serial.println("\nTesting invalid message scenarios:");

  // 无效命令
  msg.setCommand("invalid");
  msg.setVersion("1.0.0");
  msg.setDownloadUrl("https://example.com/test.bin");
  Serial.printf("  Invalid command - Valid: %s\n",
                msg.isValid() ? "true" : "false");

  // 无效电池电量
  msg.setCommand("upgrade");
  msg.setMinBatteryLevel(150); // 超出范围
  Serial.printf("  Invalid battery level - Valid: %s\n",
                msg.isValid() ? "true" : "false");

  // 恢复有效状态
  msg.setMinBatteryLevel(50);
  Serial.printf("  Fixed battery level - Valid: %s\n",
                msg.isValid() ? "true" : "false");

  Serial.println("UpgradeMessage test completed.\n");
}

// 测试消息状态枚举
void testMessageStatusEnum() {
  Serial.println("=== Message Status Enum Test ===");

  UpgradeMessageStatus statuses[] = {
      MSG_STATUS_UNKNOWN,    MSG_STATUS_RECEIVED,  MSG_STATUS_VALIDATED,
      MSG_STATUS_PROCESSING, MSG_STATUS_COMPLETED, MSG_STATUS_FAILED};

  for (int i = 0; i < 6; i++) {
    Serial.printf("  Status %d: %s\n", (int)statuses[i],
                  getUpgradeMessageStatusString(statuses[i]));
  }

  Serial.println("Message status enum test completed.\n");
}

// 运行所有UpgradeMessage相关测试
void runUpgradeMessageTests() {
  Serial.println("========================================");
  Serial.println("Running UpgradeMessage Tests");
  Serial.println("========================================");

  testUpgradeMessage();
  testMessageStatusEnum();

  Serial.println("All UpgradeMessage tests completed.");
  Serial.println("========================================\n");
}

#endif // MQTT_OTA_UPGRADE