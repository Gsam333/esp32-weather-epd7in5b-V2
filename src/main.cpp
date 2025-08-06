/* Main program for esp32-weather-epd.
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

// Standard Arduino and ESP32 libraries
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>
#include <time.h>

// ESP32 specific headers
#include "esp32-hal-gpio.h"
#include "esp32-hal.h"

// Third-party libraries
#include <Adafruit_Sensor.h>
#if defined(SENSOR_BME280)
#include <Adafruit_BME280.h>
#endif
#if defined(SENSOR_BMP280)
#include <Adafruit_BMP280.h>
#endif
#if defined(SENSOR_AHT20)
#include <Adafruit_AHTX0.h>
#endif
#if defined(SENSOR_BME680)
#include <Adafruit_BME680.h>
#endif

// WiFi security libraries
#if defined(USE_HTTPS_WITH_CERT_VERIF) || defined(USE_HTTPS_WITH_CERT_VERIF)
#include <WiFiClientSecure.h>
#endif
#ifdef USE_HTTPS_WITH_CERT_VERIF
#include "cert.h"
#endif

// Project configuration and utilities
#include "_locale.h"
#include "api_response.h"
#include "client_utils.h"
#include "config.h"
#include "display_utils.h"
#include "renderer.h"

// Icons and assets
#include "icons/icons_196x196.h"

// Optional features
#ifdef MQTT_OTA_UPGRADE
#include "mqtt_ota_manager.h"
#endif

// Sensor management modules
#include "dual_sensor_manager.h"
#include "sensor_power_manager.h"

// Global variables - too large to allocate locally on stack
static owm_resp_onecall_t owm_onecall;
static owm_resp_air_pollution_t owm_air_pollution;

Preferences prefs;

/* Put esp32 into ultra low-power deep sleep (<11μA).
 * Aligns wake time to the minute. Sleep times defined in config.cpp.
 */
void beginDeepSleep(unsigned long startTime, tm *timeInfo) {
  if (!getLocalTime(timeInfo)) {
    Serial.println(TXT_REFERENCING_OLDER_TIME_NOTICE);
  }

  // To simplify sleep time calculations, the current time stored by timeInfo
  // will be converted to time relative to the WAKE_TIME. This way if a
  // SLEEP_DURATION is not a multiple of 60 minutes it can be more trivially,
  // aligned and it can easily be deterimined whether we must sleep for
  // additional time due to bedtime.
  // i.e. when curHour == 0, then timeInfo->tm_hour == WAKE_TIME
  int bedtimeHour = INT_MAX;
  if (BED_TIME != WAKE_TIME) {
    bedtimeHour = (BED_TIME - WAKE_TIME + 24) % 24;
  }

  // time is relative to wake time
  int curHour = (timeInfo->tm_hour - WAKE_TIME + 24) % 24;
  const int curMinute = curHour * 60 + timeInfo->tm_min;
  const int curSecond =
      curHour * 3600 + timeInfo->tm_min * 60 + timeInfo->tm_sec;
  const int desiredSleepSeconds = SLEEP_DURATION * 60;
  const int offsetMinutes = curMinute % SLEEP_DURATION;
  const int offsetSeconds = curSecond % desiredSleepSeconds;

  // align wake time to nearest multiple of SLEEP_DURATION
  int sleepMinutes = SLEEP_DURATION - offsetMinutes;
  if (desiredSleepSeconds - offsetSeconds < 120 ||
      offsetSeconds / (float)desiredSleepSeconds >
          0.95f) { // if we have a sleep time less than 2 minutes OR less 5%
    // SLEEP_DURATION,
    // skip to next alignment
    sleepMinutes += SLEEP_DURATION;
  }

  // estimated wake time, if this falls in a sleep period then sleepDuration
  // must be adjusted
  const int predictedWakeHour = ((curMinute + sleepMinutes) / 60) % 24;

  uint64_t sleepDuration;
  if (predictedWakeHour < bedtimeHour) {
    sleepDuration = sleepMinutes * 60 - timeInfo->tm_sec;
  } else {
    const int hoursUntilWake = 24 - curHour;
    sleepDuration = hoursUntilWake * 3600ULL -
                    (timeInfo->tm_min * 60ULL + timeInfo->tm_sec);
  }

  // add extra delay to compensate for esp32's with fast RTCs.
  sleepDuration += 3ULL;
  sleepDuration *= 1.0015f;

#if DEBUG_LEVEL >= 1
  printHeapUsage();
#endif

  // Force garbage collection to free unused memory
  ESP.getFreeHeap();

  // Display memory usage before entering deep sleep
  Serial.println("Preparing for deep sleep...");
#if DEBUG_LEVEL >= 1
  printHeapUsage();
#endif

  // Prepare sensor power management for deep sleep
  sensorPowerManager.prepareForDeepSleep();

  esp_sleep_enable_timer_wakeup(sleepDuration * 1000000ULL);
  Serial.print(TXT_AWAKE_FOR);
  Serial.println(" " + String((millis() - startTime) / 1000.0, 3) + "s");
  Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
  Serial.println(" " + String(sleepDuration) + "s");

  // Add delay to allow time to view serial output
  Serial.println("Waiting 10 seconds for serial output...");
  for (int i = 10; i > 0; i--) {
    Serial.print("Countdown: ");
    Serial.print(i);
    Serial.println("s");
    delay(1000);
  }
  Serial.println("Entering deep sleep mode now");

  esp_deep_sleep_start();
}

/* Program entry point.
 */
void setup() {
  unsigned long startTime = millis();
  Serial.begin(115200);

  // Wait for serial connection
  delay(2000);

  // Initialize sensor power management system
  sensorPowerManager.wakeupFromDeepSleep();

  Serial.println();
  Serial.println("===========================================");
  Serial.println("ESP32 Weather Display - BMP280+AHT20 Dual Sensor");
  Serial.println("===========================================");
  Serial.println();

  // Initialize LED1 - turn on to indicate startup
  pinMode(PIN_LED1, OUTPUT);
  digitalWrite(PIN_LED1, LOW);

#if DEBUG_LEVEL >= 1
  printHeapUsage();
#endif

  disableBuiltinLED();

  // Open namespace for read/write to non-volatile storage
  prefs.begin(NVS_NAMESPACE, false);

#if BATTERY_MONITORING && !DEBUG_MODE_SKIP_HARDWARE
  uint32_t batteryVoltage = readBatteryVoltage();
  Serial.print(TXT_BATTERY_VOLTAGE);
  Serial.println(": " + String(batteryVoltage) + "mv");

  // When the battery is low, the display should be updated to reflect that, but
  // only the first time we detect low voltage. The next time the display will
  // refresh is when voltage is no longer low. To keep track of that we will
  // make use of non-volatile storage.
  bool lowBat = prefs.getBool("lowBat", false);

  // low battery, deep sleep now
  if (batteryVoltage <= LOW_BATTERY_VOLTAGE) {
    if (lowBat == false) { // battery is now low for the first time
      prefs.putBool("lowBat", true);
      prefs.end();
      initDisplay();
      do {
        drawError(battery_alert_0deg_196x196, TXT_LOW_BATTERY);
      } while (display.nextPage());
      powerOffDisplay();
    }

    if (batteryVoltage <= CRIT_LOW_BATTERY_VOLTAGE) { // critically low battery
      // don't set esp_sleep_enable_timer_wakeup();
      // We won't wake up again until someone manually presses the RST button.
      Serial.println(TXT_CRIT_LOW_BATTERY_VOLTAGE);
      Serial.println(TXT_HIBERNATING_INDEFINITELY_NOTICE);
    } else if (batteryVoltage <= VERY_LOW_BATTERY_VOLTAGE) { // very low battery
      esp_sleep_enable_timer_wakeup(VERY_LOW_BATTERY_SLEEP_INTERVAL * 60ULL *
                                    1000000ULL);
      Serial.println(TXT_VERY_LOW_BATTERY_VOLTAGE);
      Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
      Serial.println(" " + String(VERY_LOW_BATTERY_SLEEP_INTERVAL) + "min");
    } else { // low battery
      esp_sleep_enable_timer_wakeup(LOW_BATTERY_SLEEP_INTERVAL * 60ULL *
                                    1000000ULL);
      Serial.println(TXT_LOW_BATTERY_VOLTAGE);
      Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
      Serial.println(" " + String(LOW_BATTERY_SLEEP_INTERVAL) + "min");
    }
    esp_deep_sleep_start();
  }
  // battery is no longer low, reset variable in non-volatile storage
  if (lowBat == true) {
    prefs.putBool("lowBat", false);
  }
#elif DEBUG_MODE_SKIP_HARDWARE
  // DEBUG MODE: Skip battery monitoring
  uint32_t batteryVoltage = 4200;
  Serial.println("DEBUG MODE: Skipping battery monitoring - simulating 4200mv");
#else
  uint32_t batteryVoltage = UINT32_MAX;
#endif

  // All data should have been loaded from NVS. Close filesystem.
  prefs.end();

  String statusStr = {};
  String tmpStr = {};
  tm timeInfo = {};

  // START WIFI
  int wifiRSSI = 0; // “Received Signal Strength Indicator"
  wl_status_t wifiStatus = startWiFi(wifiRSSI);
  if (wifiStatus != WL_CONNECTED) { // WiFi Connection Failed
    killWiFi();
    initDisplay();
    if (wifiStatus == WL_NO_SSID_AVAIL) {
      Serial.println(TXT_NETWORK_NOT_AVAILABLE);
      do {
        drawError(wifi_x_196x196, TXT_NETWORK_NOT_AVAILABLE);
      } while (display.nextPage());
    } else {
      Serial.println(TXT_WIFI_CONNECTION_FAILED);
      do {
        drawError(wifi_x_196x196, TXT_WIFI_CONNECTION_FAILED);
      } while (display.nextPage());
    }
    powerOffDisplay();
    beginDeepSleep(startTime, &timeInfo);
  }

  // TIME SYNCHRONIZATION
  configTzTime(TIMEZONE, NTP_SERVER_1, NTP_SERVER_2);
  bool timeConfigured = waitForSNTPSync(&timeInfo);
  if (!timeConfigured) {
    Serial.println(TXT_TIME_SYNCHRONIZATION_FAILED);
    killWiFi();
    initDisplay();
    do {
      drawError(wi_time_4_196x196, TXT_TIME_SYNCHRONIZATION_FAILED);
    } while (display.nextPage());
    powerOffDisplay();
    beginDeepSleep(startTime, &timeInfo);
  }

#ifdef MQTT_OTA_UPGRADE
  // MQTT OTA UPGRADE CHECK
  Serial.println("Checking for MQTT OTA upgrades...");

  // 配置MQTT OTA
  mqttOTAConfig.mqttServer = MQTT_OTA_SERVER;
  mqttOTAConfig.mqttPort = MQTT_OTA_PORT;
  mqttOTAConfig.mqttUsername = MQTT_OTA_USERNAME;
  mqttOTAConfig.mqttPassword = MQTT_OTA_PASSWORD;
  mqttOTAConfig.enableSSL = MQTT_OTA_USE_SSL;
  mqttOTAConfig.connectionTimeout = MQTT_OTA_CONNECTION_TIMEOUT;
  mqttOTAConfig.messageTimeout = MQTT_OTA_MESSAGE_TIMEOUT;
  mqttOTAConfig.maxRetries = MQTT_OTA_MAX_RETRIES;
  mqttOTAConfig.enableOTA = MQTT_OTA_ENABLE;
  mqttOTAConfig.minBatteryLevel = MQTT_OTA_MIN_BATTERY_LEVEL;
  mqttOTAConfig.allowDowngrade = MQTT_OTA_ALLOW_DOWNGRADE;

  // 生成设备ID和topic
  mqttOTAConfig.deviceId = "weather-display-" + WiFi.macAddress();
  mqttOTAConfig.deviceId.replace(":", ""); // 移除MAC地址中的冒号
  mqttOTAConfig.generateTopics();

  // 初始化MQTT OTA管理器
  if (mqttOTAManager.begin(&mqttOTAConfig)) {
    // 检查升级（这个函数会在10秒内完成）
    bool upgradeTriggered = mqttOTAManager.checkForUpgrade();

    if (upgradeTriggered) {
      // 如果升级被触发，设备会在升级完成后重启
      // 这里的代码不会被执行到
      Serial.println("OTA upgrade triggered, device will restart...");
    } else {
      Serial.println("No OTA upgrade available, continuing normal operation");
    }
  } else {
    Serial.println(
        "Failed to initialize MQTT OTA manager, continuing normal operation");
  }
#endif

  // MAKE API REQUESTS
#ifdef USE_HTTP
  WiFiClient client;
#elif defined(USE_HTTPS_NO_CERT_VERIF)
  WiFiClientSecure client;
  client.setInsecure();
#elif defined(USE_HTTPS_WITH_CERT_VERIF)
  WiFiClientSecure client;
  client.setCACert(cert_Sectigo_RSA_Organization_Validation_Secure_Server_CA);
#endif

  // First try the current weather API (2.5/weather) - your preferred API
  Serial.println("Trying Current Weather API (2.5/weather) first...");
  int currentWeatherStatus = getOWMcurrentWeather(client, owm_onecall.current);

  // Then try Forecast API for forecast data
  Serial.println("Trying Forecast API (2.5/forecast)...");
  int rxStatus = getOWMonecall(client, owm_onecall);

  // If current weather API failed but One Call succeeded, use One Call data
  if (currentWeatherStatus != HTTP_CODE_OK && rxStatus == HTTP_CODE_OK) {
    Serial.println("Current Weather API failed, but One Call API succeeded. "
                   "Using One Call data.");
  }
  // If current weather API succeeded but One Call failed, we still have current
  // weather
  else if (currentWeatherStatus == HTTP_CODE_OK && rxStatus != HTTP_CODE_OK) {
    Serial.println("Current Weather API succeeded, but One Call API failed. "
                   "Limited forecast data available.");
    // Clear forecast arrays to prevent displaying stale data
    for (int i = 0; i < OWM_NUM_HOURLY; i++) {
      owm_onecall.hourly[i] = {};
    }
    for (int i = 0; i < OWM_NUM_DAILY; i++) {
      owm_onecall.daily[i] = {};
    }
  }
  // If both APIs failed, show error
  else if (currentWeatherStatus != HTTP_CODE_OK && rxStatus != HTTP_CODE_OK) {
    killWiFi();
    statusStr = "Weather APIs Failed";
    tmpStr = "Current: " + String(currentWeatherStatus, DEC) +
             ", OneCall: " + String(rxStatus, DEC);
    initDisplay();
    do {
      drawError(wi_cloud_down_196x196, statusStr, tmpStr);
    } while (display.nextPage());
    powerOffDisplay();
    beginDeepSleep(startTime, &timeInfo);
  } else {
    Serial.println("Both APIs succeeded! Using current weather data from "
                   "2.5/weather API.");
  }
  rxStatus = getOWMairpollution(client, owm_air_pollution);

  if (rxStatus != HTTP_CODE_OK) {
    killWiFi();
    statusStr = "Air Pollution API";
    tmpStr = String(rxStatus, DEC) + ": " + getHttpResponsePhrase(rxStatus);
    initDisplay();
    do {
      drawError(wi_cloud_down_196x196, statusStr, tmpStr);
    } while (display.nextPage());
    powerOffDisplay();
    beginDeepSleep(startTime, &timeInfo);
  }
  killWiFi(); // WiFi no longer needed

  // GET INDOOR TEMPERATURE AND HUMIDITY
  float inTemp = NAN;
  float inHumidity = NAN;
  float inPressure = NAN;

#if DEBUG_MODE_SKIP_HARDWARE
  // DEBUG MODE: Skip sensor initialization
  Serial.println("DEBUG MODE: Skipping sensors - simulating sensor data");
  inTemp = 22.5;
  inHumidity = 45.0;
  inPressure = 101325.0;
#else
  // Use dual sensor management system
  bool sensorInitSuccess = dualSensorManager.initialize();
  bool dataReadSuccess = false;

  if (sensorInitSuccess) {
    // Try to read sensor data with up to 3 retries
    int retryCount = 0;
    const int maxRetries = 3;

    while (retryCount < maxRetries && !dataReadSuccess) {
      if (retryCount > 0) {
        Serial.printf("Retrying sensor data read (attempt %d)...\n",
                      retryCount + 1);
        delay(500); // Wait 500ms before retry
      }

      SensorData sensorData = dualSensorManager.readAllSensors();

      // Check if at least one valid data point exists
      bool hasValidData = sensorData.temperatureValid ||
                          sensorData.humidityValid || sensorData.pressureValid;

      if (hasValidData) {
        dataReadSuccess = true;

        // Map to original variables
        if (sensorData.temperatureValid) {
          inTemp = sensorData.temperature;
          Serial.printf("Temperature: %.2f°C\n", inTemp);
        } else {
          Serial.println("Temperature data invalid, will display as '--'");
        }

        if (sensorData.humidityValid) {
          inHumidity = sensorData.humidity;
          Serial.printf("Humidity: %.2f%%\n", inHumidity);
        } else {
          Serial.println("Humidity data invalid, will display as '--'");
        }

        if (sensorData.pressureValid) {
          inPressure = sensorData.pressure;
          Serial.printf("Pressure: %.2f hPa\n", inPressure / 100.0);
        } else {
          Serial.println("Pressure data invalid");
        }

        Serial.println("Dual sensor data read completed");

        // Clear any previous error status
        if (!statusStr.isEmpty() && statusStr.indexOf("传感器") >= 0) {
          statusStr = "";
        }
      } else {
        retryCount++;
        Serial.printf("Data read attempt %d failed, all sensor data invalid\n",
                      retryCount);
      }
    }

    if (!dataReadSuccess) {
      statusStr = "Sensor data read failed";
      Serial.println("Unable to read valid sensor data after multiple retries");
    }
  } else {
    statusStr = "Sensor initialization failed";
    Serial.println("Dual sensor system initialization failed");
  }

  // If dual sensor system completely fails, log the failure
  if (!sensorInitSuccess || !dataReadSuccess) {
    Serial.println(
        "Dual sensor system failed, sensor data will show as invalid");
  }

  // Final data validation and status report
  bool hasTempData = !std::isnan(inTemp);
  bool hasHumidityData = !std::isnan(inHumidity);
  bool hasPressureData = !std::isnan(inPressure);

  Serial.println("Final sensor data status:");
  Serial.printf("  Temperature: %s\n", hasTempData ? "Valid" : "Invalid");
  Serial.printf("  Humidity: %s\n", hasHumidityData ? "Valid" : "Invalid");
  Serial.printf("  Pressure: %s\n", hasPressureData ? "Valid" : "Invalid");

  if (!hasTempData && !hasHumidityData && !hasPressureData) {
    Serial.println("Warning: All sensor data invalid, display will show '--' "
                   "placeholders");
  }
#endif

  String refreshTimeStr;
  getRefreshTimeStr(refreshTimeStr, timeConfigured, &timeInfo);
  String dateStr;
  getDateStr(dateStr, &timeInfo);

  // RENDER FULL REFRESH
  initDisplay();
  do {
    drawCurrentConditions(owm_onecall.current, owm_onecall.daily[0],
                          owm_air_pollution, inTemp, inHumidity);
    drawOutlookGraph(owm_onecall.hourly, owm_onecall.daily, timeInfo);
    drawForecast(owm_onecall.daily, timeInfo);
    drawLocationDate(CITY_STRING, dateStr);
#if DISPLAY_ALERTS
    drawAlerts(owm_onecall.alerts, CITY_STRING, dateStr);
#endif
    drawStatusBar(statusStr, refreshTimeStr, wifiRSSI, batteryVoltage);
  } while (display.nextPage());
  powerOffDisplay();

  // Turn off LED
  digitalWrite(PIN_LED1, HIGH);

  // DEEP SLEEP
  beginDeepSleep(startTime, &timeInfo);
}

/* This will never run in normal operation.
 */
void loop() { delay(100); }
