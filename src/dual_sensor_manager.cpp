/**
 * 双传感器管理模块实现
 * 负责管理BMP280+AHT20双传感器系统
 */

#include "dual_sensor_manager.h"
#include "sensor_power_manager.h"

// 全局实例
DualSensorManager dualSensorManager;

DualSensorManager::DualSensorManager() : i2c(nullptr), initialized(false) {
#if defined(SENSOR_BMP280)
  bmp280 = nullptr;
#endif
#if defined(SENSOR_AHT20)
  aht20 = nullptr;
#endif
}

DualSensorManager::~DualSensorManager() { shutdown(); }

bool DualSensorManager::initialize() {
  Serial.println("🌡️ 初始化双传感器管理系统...");

  // 确保传感器电源开启
  if (!sensorPowerManager.isPowerEnabled()) {
    Serial.println("⚠️  传感器电源未开启，正在启用...");
    sensorPowerManager.enablePower();
    delay(100); // 等待电源稳定
  }

  // 初始化I2C总线
  if (!initializeI2C()) {
    logError("I2C总线初始化失败");
    return false;
  }

  // 扫描I2C设备
  scanI2CDevices();

  // 初始化传感器对象
#if defined(SENSOR_BMP280)
  if (bmp280 == nullptr) {
    bmp280 = new Adafruit_BMP280(i2c);
  }
#endif
#if defined(SENSOR_AHT20)
  if (aht20 == nullptr) {
    aht20 = new Adafruit_AHTX0();
  }
#endif

  // 初始化所有传感器
  bool success = initializeAllSensors();

  if (success) {
    initialized = true;
    status.errorCount = 0;
    logSuccess("双传感器系统初始化完成");
    printStatus();
  } else {
    logError("双传感器系统初始化失败");
  }

  return success;
}

bool DualSensorManager::initializeI2C() {
  Serial.println("🔌 初始化I2C总线...");

  if (i2c == nullptr) {
    i2c = new TwoWire(0);
  }

  // 初始化I2C总线
  bool success = i2c->begin(PIN_BME_SDA, PIN_BME_SCL, I2C_FREQUENCY);

  if (success) {
    status.i2cInitialized = true;
    Serial.printf("✅ I2C总线初始化成功 (SDA=GPIO%d, SCL=GPIO%d, %dkHz)\n",
                  PIN_BME_SDA, PIN_BME_SCL, I2C_FREQUENCY / 1000);
  } else {
    status.i2cInitialized = false;
    Serial.println("❌ I2C总线初始化失败");
  }

  return success;
}

void DualSensorManager::shutdown() {
  Serial.println("🔌 关闭双传感器管理系统...");

#if defined(SENSOR_BMP280)
  if (bmp280 != nullptr) {
    delete bmp280;
    bmp280 = nullptr;
  }
#endif
#if defined(SENSOR_AHT20)
  if (aht20 != nullptr) {
    delete aht20;
    aht20 = nullptr;
  }
#endif

  if (i2c != nullptr) {
    delete i2c;
    i2c = nullptr;
  }

  initialized = false;
  status = SensorStatus();

  Serial.println("✅ 双传感器管理系统已关闭");
}

void DualSensorManager::scanI2CDevices() {
  Serial.println("🔍 扫描I2C总线设备...");

  if (!status.i2cInitialized) {
    Serial.println("❌ I2C总线未初始化，无法扫描");
    return;
  }

  int deviceCount = 0;
  Serial.println();
  Serial.println("地址  设备类型        状态");
  Serial.println("----  ------------  --------");

  for (uint8_t address = 1; address < 127; address++) {
    if (isDevicePresent(address)) {
      deviceCount++;
      Serial.printf("0x%02X  ", address);

      // 识别设备类型
      if (address == BMP280_ADDR_1 || address == BMP280_ADDR_2) {
        Serial.printf("BMP280 传感器   发现 ✅");
        status.bmp280Address = address;
      } else if (address == AHT20_ADDR) {
        Serial.printf("AHT20 传感器    发现 ✅");
        status.aht20Address = address;
      } else {
        Serial.printf("未知设备        发现 ❓");
      }
      Serial.println();
    }
  }

  Serial.println("----  ------------  --------");
  Serial.printf("总计发现 %d 个I2C设备\n", deviceCount);
  Serial.println();

  status.lastScan = getCurrentTime();

  if (deviceCount == 0) {
    logError("未发现任何I2C设备");
    Serial.println("🔍 请检查：");
    Serial.println("  - 硬件接线是否正确");
    Serial.println("  - 传感器是否已连接");
    Serial.println("  - 电源是否正常 (GPIO5 = HIGH)");
    Serial.println("  - I2C上拉电阻是否存在");
  }
}

bool DualSensorManager::isDevicePresent(uint8_t address) {
  if (!status.i2cInitialized || i2c == nullptr) {
    return false;
  }

  i2c->beginTransmission(address);
  uint8_t error = i2c->endTransmission();
  return (error == 0);
}

void DualSensorManager::printI2CScanResults() { scanI2CDevices(); }

bool DualSensorManager::initializeBMP280() {
#if defined(SENSOR_BMP280)
  Serial.print("🌪️  初始化BMP280传感器... ");

  if (bmp280 == nullptr) {
    Serial.println("失败 - 对象未创建");
    return false;
  }

  // 尝试两个可能的地址
  bool success = false;
  if (bmp280->begin(BMP280_ADDR_1)) {
    status.bmp280Address = BMP280_ADDR_1;
    success = true;
    Serial.printf("成功 (地址: 0x%02X)\n", BMP280_ADDR_1);
  } else if (bmp280->begin(BMP280_ADDR_2)) {
    status.bmp280Address = BMP280_ADDR_2;
    success = true;
    Serial.printf("成功 (地址: 0x%02X)\n", BMP280_ADDR_2);
  } else {
    Serial.println("失败 ❌");
    status.bmp280Available = false;
    return false;
  }

  if (success) {
    // 配置BMP280参数
    success = configureBMP280();
    if (success) {
      status.bmp280Available = true;
      Serial.println("✅ BMP280传感器配置完成");
    } else {
      status.bmp280Available = false;
      Serial.println("❌ BMP280传感器配置失败");
    }
  }

  return success;
#else
  Serial.println("⚠️  BMP280支持未启用");
  return false;
#endif
}

bool DualSensorManager::initializeAHT20() {
#if defined(SENSOR_AHT20)
  Serial.print("💧 初始化AHT20传感器... ");

  if (aht20 == nullptr) {
    Serial.println("失败 - 对象未创建");
    return false;
  }

  bool success = aht20->begin(i2c);

  if (success) {
    status.aht20Available = true;
    status.aht20Address = AHT20_ADDR;
    Serial.println("成功 ✅");
  } else {
    status.aht20Available = false;
    Serial.println("失败 ❌");
  }

  return success;
#else
  Serial.println("⚠️  AHT20支持未启用");
  return false;
#endif
}

bool DualSensorManager::initializeAllSensors() {
  Serial.println("🔧 初始化所有传感器...");

  bool bmp280Success = initializeBMP280();
  bool aht20Success = initializeAHT20();

  // 检查初始化结果
  if (bmp280Success && aht20Success) {
    Serial.println("🎉 所有传感器初始化成功！");
    return true;
  } else if (bmp280Success || aht20Success) {
    Serial.println("⚠️  部分传感器初始化成功");
    Serial.printf("  BMP280: %s\n", bmp280Success ? "可用 ✅" : "不可用 ❌");
    Serial.printf("  AHT20:  %s\n", aht20Success ? "可用 ✅" : "不可用 ❌");
    return true; // 至少一个传感器可用
  } else {
    Serial.println("❌ 所有传感器初始化失败");
    status.errorCount++;
    return false;
  }
}

SensorData DualSensorManager::readAllSensors() {
  SensorData data;
  data.timestamp = getCurrentTime();

  if (!initialized) {
    logError("传感器系统未初始化");
    return data;
  }

  // 读取BMP280数据
  if (status.bmp280Available) {
    float bmpTemp, pressure, altitude;
    if (readBMP280Data(bmpTemp, pressure, altitude)) {
      data.pressure = pressure;
      data.pressureValid = true;
      data.altitude = altitude;
      data.altitudeValid = true;

      // 如果AHT20不可用，使用BMP280的温度
      if (!status.aht20Available) {
        data.temperature = bmpTemp;
        data.temperatureValid = true;
      }
    }
  }

  // 读取AHT20数据
  if (status.aht20Available) {
    float ahtTemp, humidity;
    if (readAHT20Data(ahtTemp, humidity)) {
      data.temperature = ahtTemp;
      data.temperatureValid = true;
      data.humidity = humidity;
      data.humidityValid = true;
    }
  }

  return data;
}

bool DualSensorManager::readBMP280Data(float &temperature, float &pressure,
                                       float &altitude) {
#if defined(SENSOR_BMP280)
  if (!status.bmp280Available || bmp280 == nullptr) {
    return false;
  }

  temperature = bmp280->readTemperature();
  pressure = bmp280->readPressure();
  altitude = bmp280->readAltitude(SEA_LEVEL_PRESSURE);

  bool valid = validateSensorData(temperature) &&
               validateSensorData(pressure) && validateSensorData(altitude);

  if (!valid) {
    status.errorCount++;
    logError("BMP280数据读取失败或无效");
  }

  return valid;
#else
  return false;
#endif
}

bool DualSensorManager::readAHT20Data(float &temperature, float &humidity) {
#if defined(SENSOR_AHT20)
  if (!status.aht20Available || aht20 == nullptr) {
    return false;
  }

  sensors_event_t humidityEvent, tempEvent;
  aht20->getEvent(&humidityEvent, &tempEvent);

  temperature = tempEvent.temperature;
  humidity = humidityEvent.relative_humidity;

  bool valid = validateSensorData(temperature) && validateSensorData(humidity);

  if (!valid) {
    status.errorCount++;
    logError("AHT20数据读取失败或无效");
  }

  return valid;
#else
  return false;
#endif
}

SensorStatus DualSensorManager::getStatus() const { return status; }

bool DualSensorManager::isAnyAvailable() const {
  return status.bmp280Available || status.aht20Available;
}

bool DualSensorManager::areBothAvailable() const {
  return status.bmp280Available && status.aht20Available;
}

void DualSensorManager::printStatus() const {
  Serial.println("📊 双传感器系统状态：");
  Serial.printf("  系统初始化: %s\n", initialized ? "完成 ✅" : "未完成 ❌");
  Serial.printf("  I2C总线: %s\n",
                status.i2cInitialized ? "已初始化 ✅" : "未初始化 ❌");
  Serial.printf("  电源状态: %s\n",
                sensorPowerManager.isPowerEnabled() ? "开启 ✅" : "关闭 ❌");
  Serial.println();

  Serial.println("📡 传感器状态：");
  Serial.printf("  BMP280: %s",
                status.bmp280Available ? "可用 ✅" : "不可用 ❌");
  if (status.bmp280Available) {
    Serial.printf(" (地址: 0x%02X)", status.bmp280Address);
  }
  Serial.println();

  Serial.printf("  AHT20:  %s",
                status.aht20Available ? "可用 ✅" : "不可用 ❌");
  if (status.aht20Available) {
    Serial.printf(" (地址: 0x%02X)", status.aht20Address);
  }
  Serial.println();

  Serial.printf("  错误计数: %d\n", status.errorCount);

  if (status.lastScan > 0) {
    unsigned long timeSinceScan = getCurrentTime() - status.lastScan;
    Serial.printf("  上次扫描: %lu毫秒前\n", timeSinceScan);
  }

  Serial.println();
}

bool DualSensorManager::retryInitialization() {
  Serial.println("🔄 重试传感器初始化...");

  // 重置状态
  status.bmp280Available = false;
  status.aht20Available = false;
  status.errorCount = 0;

  // 重新扫描和初始化
  scanI2CDevices();
  return initializeAllSensors();
}

void DualSensorManager::resetErrorCount() { status.errorCount = 0; }

int DualSensorManager::getErrorCount() const { return status.errorCount; }

void DualSensorManager::runDiagnostics() {
  Serial.println("🔍 运行双传感器系统诊断...");
  Serial.println();

  // 打印系统状态
  printStatus();

  // 测试I2C通信
  testI2CCommunication();

  // 测试传感器数据读取
  Serial.println("📖 测试传感器数据读取：");
  SensorData data = readAllSensors();
  printSensorData(data);

  Serial.println("🔍 诊断完成");
  Serial.println();
}

bool DualSensorManager::testI2CCommunication() {
  Serial.println("🔌 测试I2C通信：");

  if (!status.i2cInitialized) {
    Serial.println("  ❌ I2C总线未初始化");
    return false;
  }

  bool bmp280Comm = isDevicePresent(status.bmp280Address);
  bool aht20Comm = isDevicePresent(status.aht20Address);

  Serial.printf("  BMP280通信: %s\n", bmp280Comm ? "正常 ✅" : "异常 ❌");
  Serial.printf("  AHT20通信:  %s\n", aht20Comm ? "正常 ✅" : "异常 ❌");

  return bmp280Comm || aht20Comm;
}

void DualSensorManager::printSensorData(const SensorData &data) const {
  Serial.println("📊 传感器数据：");

  if (data.temperatureValid) {
    Serial.printf("  🌡️  温度: %.2f°C\n", data.temperature);
  } else {
    Serial.println("  🌡️  温度: 无效数据 ❌");
  }

  if (data.humidityValid) {
    Serial.printf("  💧 湿度: %.2f%%\n", data.humidity);
  } else {
    Serial.println("  💧 湿度: 无效数据 ❌");
  }

  if (data.pressureValid) {
    Serial.printf("  🌪️  气压: %.2f Pa (%.2f hPa)\n", data.pressure,
                  data.pressure / 100.0);
  } else {
    Serial.println("  🌪️  气压: 无效数据 ❌");
  }

  if (data.altitudeValid) {
    Serial.printf("  🏔️  海拔: %.2f m\n", data.altitude);
  } else {
    Serial.println("  🏔️  海拔: 无效数据 ❌");
  }

  Serial.printf("  ⏰ 时间戳: %lu\n", data.timestamp);
  Serial.println();
}

// 私有方法实现

bool DualSensorManager::configureBMP280() {
#if defined(SENSOR_BMP280)
  if (bmp280 == nullptr) {
    return false;
  }

  // 配置BMP280参数
  bmp280->setSampling(Adafruit_BMP280::MODE_NORMAL,     // 工作模式
                      Adafruit_BMP280::SAMPLING_X2,     // 温度过采样
                      Adafruit_BMP280::SAMPLING_X16,    // 气压过采样
                      Adafruit_BMP280::FILTER_X16,      // 滤波
                      Adafruit_BMP280::STANDBY_MS_500); // 待机时间

  return true;
#else
  return false;
#endif
}

bool DualSensorManager::validateSensorData(float value) const {
  return !isnan(value) && isfinite(value);
}

void DualSensorManager::logError(const String &message) {
  Serial.println("❌ 错误: " + message);
}

void DualSensorManager::logSuccess(const String &message) {
  Serial.println("✅ " + message);
}

unsigned long DualSensorManager::getCurrentTime() const { return millis(); }

// 便捷函数实现

bool initializeDualSensors() { return dualSensorManager.initialize(); }

SensorData readSensorData() { return dualSensorManager.readAllSensors(); }

bool isDualSensorAvailable() { return dualSensorManager.isAnyAvailable(); }

void printDualSensorStatus() { dualSensorManager.printStatus(); }