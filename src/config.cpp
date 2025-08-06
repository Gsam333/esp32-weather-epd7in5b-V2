/* Configuration options for esp32-weather-epd.
 * Copyright (C) 2022-2025  Luke Marzen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "secrets.h"
#include <Arduino.h>

// PINS
// The configuration below is intended for use with the project's official
// wiring diagrams using the FireBeetle 2 ESP32-E microcontroller board.
//
// Note: LED_BUILTIN pin will be disabled to reduce power draw.  Refer to your
//       board's pinout to ensure you avoid using a pin with this shared
//       functionality.
//
// LED1 pin for status indication
const uint8_t PIN_LED1 = 2; // IO2 pin for LED1

// ADC pin used to measure battery voltage
const uint8_t PIN_BAT_ADC = A2; // A0 for micro-usb firebeetle

// Pins for E-Paper Driver Board
// 原始项目配置 - 使用FireBeetle 2 ESP32-E的默认配置
// const uint8_t PIN_EPD_BUSY = 14; // 5 for micro-usb firebeetle
// const uint8_t PIN_EPD_CS = 13;
// const uint8_t PIN_EPD_RST = 21;
// const uint8_t PIN_EPD_DC = 22;
// const uint8_t PIN_EPD_SCK = 18;
// const uint8_t PIN_EPD_MISO = 19; // Master-In Slave-Out not used, as no data
// from display const uint8_t PIN_EPD_MOSI = 23; const uint8_t PIN_EPD_PWR = 26;
// // Irrelevant if directly connected to 3.3V

// 微雪epd7in5b_V2-demo官方示例GPIO配置
const uint8_t PIN_EPD_BUSY = 25; // EPD_BUSY_PIN 25
const uint8_t PIN_EPD_CS = 15;   // EPD_CS_PIN 15
const uint8_t PIN_EPD_RST = 26;  // EPD_RST_PIN 26
const uint8_t PIN_EPD_DC = 27;   // EPD_DC_PIN 27
const uint8_t PIN_EPD_SCK = 13;  // EPD_SCK_PIN 13
const uint8_t PIN_EPD_MISO = 12; // 未在官方示例中使用，这里设置一个不冲突的引脚
const uint8_t PIN_EPD_MOSI = 14; // EPD_MOSI_PIN 14
const uint8_t PIN_EPD_PWR = 26;  // 与RST共用，如果直接连接到3.3V则不需要
// I2C Pins used for BME280
const uint8_t PIN_BME_SDA = 17;
const uint8_t PIN_BME_SCL = 16;
const uint8_t PIN_BME_PWR = 4; // Irrelevant if directly connected to 3.3V
const uint8_t BME_ADDRESS = 0x76;
// 0x76 if SDO -> GND; 0x77 if SDO -> VCC
// CSB  ->   3.3V
// 芯片选择引脚必须接VCC以启用I2C模式，接GND会切换到SPI模式）
// SDO  ->   GND
// 设备地址选择引脚（接GND，对应工程中定义的0x76地址）

// WIFI
const char *WIFI_SSID = SECRET_WIFI_SSID;
const char *WIFI_PASSWORD = SECRET_WIFI_PASSWORD;
const unsigned long WIFI_TIMEOUT = 10000; // ms, WiFi connection timeout.

// HTTP
// The following errors are likely the result of insuffient http client tcp
// timeout:
//   -1   Connection Refused
//   -11  Read Timeout
//   -258 Deserialization Incomplete Input
const unsigned HTTP_CLIENT_TCP_TIMEOUT =
    30000; // ms，增加到30秒以处理大型JSON响应

// OPENWEATHERMAP API
// OpenWeatherMap API key, https://openweathermap.org/
const String OWM_APIKEY = SECRET_OWM_APIKEY;
const String OWM_ENDPOINT = "api.openweathermap.org";
// OpenWeatherMap One Call 2.5 API is deprecated for all new free users
// (accounts created after Summer 2022).
//
// Please note, that One Call API 3.0 is included in the "One Call by Call"
// subscription only. This separate subscription includes 1,000 calls/day for
// free and allows you to pay only for the number of API calls made to this
// product.
//
// Here’s how to subscribe and avoid any credit card changes:
// - Go to
// https://home.openweathermap.org/subscriptions/billing_info/onecall_30/base?key=base&service=onecall_30
// - Follow the instructions to complete the subscription.
// - Go to https://home.openweathermap.org/subscriptions and set the "Calls per
//   day (no more than)" to 1,000. This ensures you will never overrun the free
//   calls.
const String OWM_ONECALL_VERSION = "2.5";

// LOCATION
// Set your latitude and longitude.
// (used to get weather data as part of API requests to OpenWeatherMap)
const String LAT = "31.2304";
const String LON = "121.4737";
// City name that will be shown in the top-right corner of the display.
const String CITY_STRING = "Shanghai";

// TIME
// For list of time zones see
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char *TIMEZONE = "CST-8"; // China Standard Time (UTC+8)
// Time format used when displaying sunrise/set times. (Max 11 characters)
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
// const char *TIME_FORMAT = "%l:%M%P"; // 12-hour ex: 1:23am  11:00pm
const char *TIME_FORMAT = "%H:%M"; // 24-hour ex: 01:23   23:00
// Time format used when displaying axis labels. (Max 11 characters)
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
// const char *HOUR_FORMAT = "%l%P"; // 12-hour ex: 1am  11pm
const char *HOUR_FORMAT = "%H"; // 24-hour ex: 01   23
// Date format used when displaying date in top-right corner.
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
const char *DATE_FORMAT = "%a, %B %e"; // ex: Sat, January 1
// Date/Time format used when displaying the last refresh time along the bottom
// of the screen.
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
const char *REFRESH_TIME_FORMAT = "%x %H:%M";
// NTP_SERVER_1 is the primary time server, while NTP_SERVER_2 is a fallback.
// pool.ntp.org will find the closest available NTP server to you.
const char *NTP_SERVER_1 = "pool.ntp.org";
const char *NTP_SERVER_2 = "time.nist.gov";
// If you encounter the 'Failed To Fetch The Time' error, try increasing
// NTP_TIMEOUT or select closer/lower latency time servers.
const unsigned long NTP_TIMEOUT = 20000; // ms
// Sleep duration in minutes. (aka how often esp32 will wake for an update)
// Aligned to the nearest minute boundary.
// For example, if set to 30 (minutes) the display will update at 00 or 30
// minutes past the hour. (range: [2-1440])
// Note: The OpenWeatherMap model is updated every 10 minutes, so updating more
//       frequently than that is unnessesary.
const int SLEEP_DURATION = 30; // minutes
// Bed Time Power Savings.
// If BED_TIME == WAKE_TIME, then this battery saving feature will be disabled.
// (range: [0-23])
const int BED_TIME = 00;  // Last update at 00:00 (midnight) until WAKE_TIME.
const int WAKE_TIME = 06; // Hour of first update after BED_TIME, 06:00.
// Note that the minute alignment of SLEEP_DURATION begins at WAKE_TIME even if
// Bed Time Power Savings is disabled.
// For example, if WAKE_TIME = 00 (midnight) and SLEEP_DURATION = 120, then the
// display will update at 00:00, 02:00, 04:00... until BED_TIME.
// If you desire to have your display refresh exactly once a day, you should set
// SLEEP_DURATION = 1440, and you can set the time it should update each day by
// setting both BED_TIME and WAKE_TIME to the hour you want it to update.

// HOURLY OUTLOOK GRAPH
// Number of hours to display on the outlook graph. (range: [8-48])
const int HOURLY_GRAPH_MAX = 24;

// BATTERY
// To protect the battery upon LOW_BATTERY_VOLTAGE, the display will cease to
// update until battery is charged again. The ESP32 will deep-sleep (consuming
// < 11μA), waking briefly check the voltage at the corresponding interval (in
// minutes). Once the battery voltage has fallen to CRIT_LOW_BATTERY_VOLTAGE,
// the esp32 will hibernate and a manual press of the reset (RST) button to
// begin operating again.
const uint32_t WARN_BATTERY_VOLTAGE = 3535;                // (millivolts) ~20%
const uint32_t LOW_BATTERY_VOLTAGE = 3462;                 // (millivolts) ~10%
const uint32_t VERY_LOW_BATTERY_VOLTAGE = 3442;            // (millivolts)  ~8%
const uint32_t CRIT_LOW_BATTERY_VOLTAGE = 3404;            // (millivolts)  ~5%
const unsigned long LOW_BATTERY_SLEEP_INTERVAL = 30;       // (minutes)
const unsigned long VERY_LOW_BATTERY_SLEEP_INTERVAL = 120; // (minutes)
// Battery voltage calculations are based on a typical 3.7v LiPo.
const uint32_t MAX_BATTERY_VOLTAGE = 4200; // (millivolts)
const uint32_t MIN_BATTERY_VOLTAGE = 3000; // (millivolts)

#ifdef MQTT_OTA_UPGRADE
// MQTT OTA UPGRADE CONFIGURATION
// These settings are used when MQTT_OTA_UPGRADE is defined in platformio.ini
// MQTT服务器配置 - 从secrets.h读取敏感信息
const char *MQTT_OTA_SERVER = SECRET_MQTT_OTA_SERVER;
const int MQTT_OTA_PORT = SECRET_MQTT_OTA_PORT;
const char *MQTT_OTA_USERNAME = SECRET_MQTT_OTA_USERNAME;
const char *MQTT_OTA_PASSWORD = SECRET_MQTT_OTA_PASSWORD;
const bool MQTT_OTA_USE_SSL = SECRET_MQTT_OTA_USE_SSL;

// MQTT OTA连接配置
const int MQTT_OTA_CONNECTION_TIMEOUT = 5000; // MQTT连接超时 (ms)
const int MQTT_OTA_MESSAGE_TIMEOUT = 10000;   // 消息等待超时 (ms)
const int MQTT_OTA_MAX_RETRIES = 3;           // 最大重试次数

// MQTT OTA升级配置
const bool MQTT_OTA_ENABLE = true;           // 是否启用OTA功能
const int MQTT_OTA_MIN_BATTERY_LEVEL = 30;   // 最低电池电量要求 (%)
const bool MQTT_OTA_ALLOW_DOWNGRADE = false; // 是否允许版本降级

// 注意：设备ID和topic将在运行时根据MAC地址自动生成
// 格式：devices/weather-display-{MAC}/ota/upgrade
#endif

// See config.h for the below options
// E-PAPER PANEL
// LOCALE
// UNITS
// WIND ICON PRECISION
// FONTS
// ALERTS
// BATTERY MONITORING
