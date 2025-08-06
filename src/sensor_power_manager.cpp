/**
 * 传感器电源管理模块实现
 * 负责管理BMP280+AHT20传感器的电源状态和GPIO保持功能
 */

#include "sensor_power_manager.h"
#include "esp32-hal-gpio.h"

// 全局实例
SensorPowerManager sensorPowerManager;

SensorPowerManager::SensorPowerManager()
    : powerEnabled(false), gpioHoldEnabled(false), lastPowerChange(0) {}

bool SensorPowerManager::initialize() {
  Serial.println("🔋 初始化传感器电源管理系统...");

  // 释放可能存在的GPIO保持状态
  disableGPIOHold();

  // 配置GPIO4为输出模式
  pinMode(PIN_BME_PWR, OUTPUT);

  // 启用电源
  enablePower();

  // 验证电源状态
  if (validatePowerState()) {
    Serial.println("✅ 传感器电源管理系统初始化成功");
    return true;
  } else {
    Serial.println("❌ 传感器电源管理系统初始化失败");
    return false;
  }
}

void SensorPowerManager::enablePower() {
  setPowerPin(true);
  powerEnabled = true;
  logPowerChange(true);

  // 等待电源稳定
  delay(50);

  Serial.println("🔋 传感器电源已开启 (GPIO4 = HIGH, 3.3V)");
}

void SensorPowerManager::disablePower() {
  // 注意：在新的电源管理策略中，通常不建议关闭传感器电源
  Serial.println("⚠️  警告：准备关闭传感器电源 - 这可能影响传感器稳定性");

  setPowerPin(false);
  powerEnabled = false;
  logPowerChange(false);

  Serial.println("🔋 传感器电源已关闭 (GPIO4 = LOW, 0V)");
}

bool SensorPowerManager::isPowerEnabled() const { return powerEnabled; }

void SensorPowerManager::enableGPIOHold() {
  if (!powerEnabled) {
    Serial.println("⚠️  警告：电源未开启，无法启用GPIO保持功能");
    return;
  }

  // 确保GPIO4为高电平
  setPowerPin(true);

  // 启用GPIO保持功能
  gpio_hold_en(GPIO_NUM_4);
  gpioHoldEnabled = true;

  Serial.println("🔒 GPIO4保持功能已启用 (深度睡眠期间保持HIGH)");
}

void SensorPowerManager::disableGPIOHold() {
  gpio_hold_dis(GPIO_NUM_4);
  gpioHoldEnabled = false;

  Serial.println("🔓 GPIO4保持功能已禁用");
}

bool SensorPowerManager::isGPIOHoldEnabled() const { return gpioHoldEnabled; }

void SensorPowerManager::prepareForDeepSleep() {
  Serial.println("🌙 准备进入深度睡眠 - 配置传感器电源管理...");

  // 确保电源开启
  if (!powerEnabled) {
    enablePower();
  }

  // 启用GPIO保持功能
  enableGPIOHold();

  Serial.println("✅ 传感器电源管理已配置完成，可安全进入深度睡眠");
}

void SensorPowerManager::wakeupFromDeepSleep() {
  Serial.println("☀️  从深度睡眠唤醒 - 恢复传感器电源管理...");

  // 禁用GPIO保持功能
  disableGPIOHold();

  // 确保电源状态正确
  pinMode(PIN_BME_PWR, OUTPUT);
  enablePower();

  // 验证电源状态
  if (validatePowerState()) {
    Serial.println("✅ 传感器电源管理恢复成功");
  } else {
    Serial.println("⚠️  传感器电源状态异常，正在重新初始化...");
    initialize();
  }
}

float SensorPowerManager::getPowerVoltage() const {
  // 如果GPIO4为HIGH，返回3.3V，否则返回0V
  return powerEnabled ? 3.3f : 0.0f;
}

void SensorPowerManager::printStatus() const {
  Serial.println("📊 传感器电源管理状态：");
  Serial.printf("  电源状态: %s\n", powerEnabled ? "开启 ✅" : "关闭 ❌");
  Serial.printf("  GPIO4电平: %.1fV\n", getPowerVoltage());
  Serial.printf("  GPIO保持: %s\n", gpioHoldEnabled ? "启用 🔒" : "禁用 🔓");
  Serial.printf("  引脚配置: GPIO%d (输出模式)\n", PIN_BME_PWR);

  if (lastPowerChange > 0) {
    unsigned long timeSinceChange = millis() - lastPowerChange;
    Serial.printf("  上次变更: %lu毫秒前\n", timeSinceChange);
  }

  Serial.println();
}

bool SensorPowerManager::testPowerPin() {
  Serial.println("🔧 测试GPIO4电源引脚功能...");

  // 测试高电平
  setPowerPin(true);
  delay(100);
  bool highTest = digitalRead(PIN_BME_PWR) == HIGH;

  // 测试低电平
  setPowerPin(false);
  delay(100);
  bool lowTest = digitalRead(PIN_BME_PWR) == LOW;

  // 恢复到高电平
  setPowerPin(true);
  powerEnabled = true;

  bool testPassed = highTest && lowTest;

  Serial.printf("  高电平测试: %s\n", highTest ? "通过 ✅" : "失败 ❌");
  Serial.printf("  低电平测试: %s\n", lowTest ? "通过 ✅" : "失败 ❌");
  Serial.printf("  整体测试: %s\n", testPassed ? "通过 ✅" : "失败 ❌");

  return testPassed;
}

void SensorPowerManager::runDiagnostics() {
  Serial.println("🔍 运行传感器电源管理诊断...");
  Serial.println();

  // 打印当前状态
  printStatus();

  // 测试引脚功能
  testPowerPin();

  // 验证电源状态
  bool validation = validatePowerState();
  Serial.printf("电源状态验证: %s\n", validation ? "通过 ✅" : "失败 ❌");

  Serial.println("🔍 诊断完成");
  Serial.println();
}

// 私有方法实现

void SensorPowerManager::setPowerPin(bool state) {
  digitalWrite(PIN_BME_PWR, state ? HIGH : LOW);
  lastPowerChange = millis();
}

bool SensorPowerManager::validatePowerState() const {
  // 读取GPIO4的实际状态
  int actualState = digitalRead(PIN_BME_PWR);
  bool expectedState = powerEnabled;

  return (actualState == HIGH) == expectedState;
}

void SensorPowerManager::logPowerChange(bool newState) {
  Serial.printf("🔋 传感器电源状态变更: %s → %s\n",
                powerEnabled ? "开启" : "关闭", newState ? "开启" : "关闭");
}

// 便捷函数实现

void initializeSensorPower() { sensorPowerManager.initialize(); }

void enableSensorPower() { sensorPowerManager.enablePower(); }

void disableSensorPower() { sensorPowerManager.disablePower(); }

bool isSensorPowerEnabled() { return sensorPowerManager.isPowerEnabled(); }