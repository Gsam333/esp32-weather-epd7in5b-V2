/**
 * 传感器电源管理模块
 * 负责管理BMP280+AHT20传感器的电源状态和GPIO保持功能
 *
 * 功能：
 * - GPIO4电源控制
 * - 深度睡眠期间GPIO状态保持
 * - 电源状态查询和监控
 * - 传感器电源初始化
 */

#ifndef __SENSOR_POWER_MANAGER_H__
#define __SENSOR_POWER_MANAGER_H__

#include "config.h"
#include <Arduino.h>

class SensorPowerManager {
public:
  SensorPowerManager();

  // 电源管理核心功能
  bool initialize();
  void enablePower();
  void disablePower();
  bool isPowerEnabled() const;

  // GPIO保持功能（深度睡眠支持）
  void enableGPIOHold();
  void disableGPIOHold();
  bool isGPIOHoldEnabled() const;

  // 深度睡眠集成
  void prepareForDeepSleep();
  void wakeupFromDeepSleep();

  // 状态查询
  float getPowerVoltage() const;
  void printStatus() const;

  // 调试和诊断
  bool testPowerPin();
  void runDiagnostics();

private:
  bool powerEnabled;
  bool gpioHoldEnabled;
  unsigned long lastPowerChange;

  // 内部辅助函数
  void setPowerPin(bool state);
  bool validatePowerState() const;
  void logPowerChange(bool newState);
};

// 全局实例
extern SensorPowerManager sensorPowerManager;

// 便捷函数
void initializeSensorPower();
void enableSensorPower();
void disableSensorPower();
bool isSensorPowerEnabled();

#endif // __SENSOR_POWER_MANAGER_H__