/**
 * 双传感器管理模块
 * 负责管理BMP280+AHT20双传感器系统
 *
 * 功能：
 * - I2C设备扫描和发现
 * - BMP280和AHT20传感器初始化
 * - 传感器状态管理和监控
 * - 统一的数据读取接口
 * - 错误处理和重试机制
 */

#ifndef __DUAL_SENSOR_MANAGER_H__
#define __DUAL_SENSOR_MANAGER_H__

#include "config.h"
#include <Arduino.h>
#include <Wire.h>

#if defined(SENSOR_BMP280)
#include <Adafruit_BMP280.h>
#endif
#if defined(SENSOR_AHT20)
#include <Adafruit_AHTX0.h>
#endif

// 传感器数据结构
struct SensorData {
  // 温度数据
  float temperature;     // 摄氏度
  bool temperatureValid; // 温度数据有效性

  // 湿度数据
  float humidity;     // 相对湿度百分比
  bool humidityValid; // 湿度数据有效性

  // 气压数据
  float pressure;     // 帕斯卡
  bool pressureValid; // 气压数据有效性

  // 计算数据
  float altitude;     // 海拔高度（米）
  bool altitudeValid; // 海拔数据有效性

  // 时间戳
  unsigned long timestamp; // 数据读取时间戳

  // 构造函数
  SensorData()
      : temperature(NAN), temperatureValid(false), humidity(NAN),
        humidityValid(false), pressure(NAN), pressureValid(false),
        altitude(NAN), altitudeValid(false), timestamp(0) {}
};

// 传感器状态结构
struct SensorStatus {
  bool bmp280Available;   // BMP280可用性
  bool aht20Available;    // AHT20可用性
  uint8_t bmp280Address;  // BMP280 I2C地址
  uint8_t aht20Address;   // AHT20 I2C地址
  bool i2cInitialized;    // I2C总线初始化状态
  unsigned long lastScan; // 上次扫描时间
  int errorCount;         // 错误计数

  // 构造函数
  SensorStatus()
      : bmp280Available(false), aht20Available(false), bmp280Address(0),
        aht20Address(0x38), i2cInitialized(false), lastScan(0), errorCount(0) {}
};

class DualSensorManager {
public:
  DualSensorManager();
  ~DualSensorManager();

  // 初始化和配置
  bool initialize();
  bool initializeI2C();
  void shutdown();

  // I2C设备扫描
  void scanI2CDevices();
  bool isDevicePresent(uint8_t address);
  void printI2CScanResults();

  // 传感器初始化
  bool initializeBMP280();
  bool initializeAHT20();
  bool initializeAllSensors();

  // 数据读取
  SensorData readAllSensors();
  bool readBMP280Data(float &temperature, float &pressure, float &altitude);
  bool readAHT20Data(float &temperature, float &humidity);

  // 状态管理
  SensorStatus getStatus() const;
  bool isAnyAvailable() const;
  bool areBothAvailable() const;
  void printStatus() const;

  // 错误处理和重试
  bool retryInitialization();
  void resetErrorCount();
  int getErrorCount() const;

  // 诊断和测试
  void runDiagnostics();
  bool testI2CCommunication();
  void printSensorData(const SensorData &data) const;

private:
  // I2C对象
  TwoWire *i2c;

  // 传感器对象
#if defined(SENSOR_BMP280)
  Adafruit_BMP280 *bmp280;
#endif
#if defined(SENSOR_AHT20)
  Adafruit_AHTX0 *aht20;
#endif

  // 状态变量
  SensorStatus status;
  bool initialized;

  // 配置常量
  static const uint8_t BMP280_ADDR_1 = 0x76;    // SDO->GND
  static const uint8_t BMP280_ADDR_2 = 0x77;    // SDO->VCC
  static const uint8_t AHT20_ADDR = 0x38;       // 固定地址
  static const uint32_t I2C_FREQUENCY = 100000; // 100kHz
  static const int MAX_RETRY_COUNT = 3;
  static constexpr float SEA_LEVEL_PRESSURE = 1013.25; // hPa

  // 内部辅助函数
  bool configureBMP280();
  bool validateSensorData(float value) const;
  void logError(const String &message);
  void logSuccess(const String &message);
  unsigned long getCurrentTime() const;
};

// 全局实例
extern DualSensorManager dualSensorManager;

// 便捷函数
bool initializeDualSensors();
SensorData readSensorData();
bool isDualSensorAvailable();
void printDualSensorStatus();

#endif // __DUAL_SENSOR_MANAGER_H__