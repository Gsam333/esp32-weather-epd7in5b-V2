# 引脚配置修改总结

## 修改内容

### 1. 电池监控配置修改

**修改文件**: `src/config.cpp`
- **原配置**: `const uint8_t PIN_BAT_ADC = A2;` (对应GPIO34)
- **新配置**: `const uint8_t PIN_BAT_ADC = 32;` (GPIO32)
- **LED引脚**: `const uint8_t PIN_LED1 = 2;` (保持不变，用于状态指示)

**修改文件**: `src/include/config.h`
- **原配置**: `#define BATTERY_MONITORING 0` (关闭)
- **新配置**: `#define BATTERY_MONITORING 1` (开启)

**修改文件**: `platformio.ini`
- **原配置**: `-DA2=32 ; Map A2 to GPIO32 for battery ADC`
- **新配置**: `-DPIN_BAT_ADC=32 ; Map battery ADC to GPIO32`

### 2. I2C引脚配置修改

**修改文件**: `src/config.cpp`
- **原SDA配置**: `const uint8_t PIN_BME_SDA = 17;`
- **新SDA配置**: `const uint8_t PIN_BME_SDA = 18;`
- **原SCL配置**: `const uint8_t PIN_BME_SCL = 16;`
- **新SCL配置**: `const uint8_t PIN_BME_SCL = 19;`

### 3. 传感器电源引脚修改

**修改文件**: `src/config.cpp`
- **原配置**: `const uint8_t PIN_BME_PWR = 4;`
- **新配置**: `const uint8_t PIN_BME_PWR = 5;`

**修改文件**: `src/sensor_power_manager.cpp`
- 更新所有GPIO4相关的注释和日志信息为GPIO5
- 更新GPIO保持功能从`GPIO_NUM_4`到`GPIO_NUM_5`

**修改文件**: `src/dual_sensor_manager.cpp`
- 更新电源状态检查信息从GPIO4到GPIO5

## 引脚分配对比

| 功能 | 原引脚 | 新引脚 | 说明 |
|------|--------|--------|------|
| 电池监控ADC | A2 (GPIO34) | GPIO32 | 电池电压监控 |
| I2C数据线(SDA) | GPIO17 | GPIO18 | BMP280+AHT20数据线 |
| I2C时钟线(SCL) | GPIO16 | GPIO19 | BMP280+AHT20时钟线 |
| 传感器电源 | GPIO4 | GPIO5 | 传感器VCC控制 |
| 状态LED | GPIO2 | GPIO2 | LED状态指示 |
| 电池监控开关 | 关闭 | 开启 | 启用低电压检测 |

## 功能验证

### 电池监控功能
- ✅ GPIO32配置为ADC输入
- ✅ 电池监控功能已启用
- ✅ 低电压检测功能已开启
- ✅ 支持深度睡眠时的电池保护

### 传感器I2C通信
- ✅ I2C总线配置为GPIO18(SDA)/GPIO19(SCL)
- ✅ 支持BMP280温度/气压传感器
- ✅ 支持AHT20温湿度传感器
- ✅ 100kHz I2C时钟频率

### 传感器电源管理
- ✅ GPIO2控制传感器电源
- ✅ 支持深度睡眠时关闭传感器电源
- ✅ 唤醒时自动恢复传感器电源
- ✅ GPIO保持功能支持

## 测试建议

1. **硬件连接检查**:
   - 确认电池监控电路连接到GPIO32
   - 确认BMP280/AHT20的SDA连接到GPIO18
   - 确认BMP280/AHT20的SCL连接到GPIO19
   - 确认传感器VCC通过GPIO2控制

2. **功能测试**:
   - 运行`test_pin_config.cpp`验证引脚配置
   - 检查电池电压读取是否正常
   - 验证I2C设备扫描能找到传感器
   - 测试传感器电源开关功能

3. **系统集成测试**:
   - 编译并上传主程序
   - 检查串口输出确认传感器初始化成功
   - 验证电池监控和低电压保护功能
   - 测试深度睡眠和唤醒功能

## 注意事项

1. **GPIO32特性**: GPIO32是ADC1_CH4，适合用作电池电压监控
2. **GPIO2特性**: GPIO2是内置LED引脚，用于状态指示
3. **GPIO5特性**: GPIO5用作传感器电源控制，支持深度睡眠保持功能
3. **I2C上拉电阻**: 确保GPIO18/19有适当的上拉电阻(通常4.7kΩ)
4. **电源管理**: 新的电源管理策略在深度睡眠时关闭传感器电源以节省功耗

## 兼容性

- ✅ 兼容ESP32-DevKitC-V4开发板
- ✅ 兼容BMP280+AHT20双传感器配置
- ✅ 兼容现有的深度睡眠和OTA升级功能
- ✅ 保持原有的显示和网络功能不变