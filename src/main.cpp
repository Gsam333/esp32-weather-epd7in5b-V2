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

#include "config.h"
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>
#include <time.h>

#include "_locale.h"
#include "api_response.h"
#include "client_utils.h"
#include "config.h"
#include "display_utils.h"
#include "icons/icons_196x196.h"
#include "renderer.h"

#if defined(SENSOR_BME280)
#include <Adafruit_BME280.h>
#endif
#if defined(SENSOR_BME680)
#include <Adafruit_BME680.h>
#endif
#if defined(USE_HTTPS_WITH_CERT_VERIF) || defined(USE_HTTPS_WITH_CERT_VERIF)
#include <WiFiClientSecure.h>
#endif
#ifdef USE_HTTPS_WITH_CERT_VERIF
#include "cert.h"
#endif

// too large to allocate locally on stack
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

  esp_sleep_enable_timer_wakeup(sleepDuration * 1000000ULL);
  Serial.print(TXT_AWAKE_FOR);
  Serial.println(" " + String((millis() - startTime) / 1000.0, 3) + "s");
  Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
  Serial.println(" " + String(sleepDuration) + "s");
  esp_deep_sleep_start();
} // end beginDeepSleep

// LED1闪烁控制变量
unsigned long previousMillis = 0;
const long blinkInterval = 1500; // 1.5秒闪烁间隔
bool ledState = false;
bool isRequestingData = false;

// LED1闪烁函数
void blinkLED1() {
  if (isRequestingData) {
    // 发送GET请求时LED常亮
    digitalWrite(PIN_LED1, HIGH);
    return;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(PIN_LED1, ledState ? HIGH : LOW);
  }
}

/* Program entry point.
 */
void setup() {
  unsigned long startTime = millis();
  Serial.begin(115200);

  // 初始化LED1
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
  uint32_t batteryVoltage = 4200; // Simulate good battery voltage (4.2V)
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
  isRequestingData = true;      // 设置为请求数据状态，LED常亮
  digitalWrite(PIN_LED1, HIGH); // 立即点亮LED
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

  // 所有API请求完成，设置LED为闪烁模式
  isRequestingData = false;

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

  // GET INDOOR TEMPERATURE AND HUMIDITY, start BMEx80...
  float inTemp = NAN;
  float inHumidity = NAN;

#if DEBUG_MODE_SKIP_HARDWARE
  // DEBUG MODE: Skip BME sensor initialization
  Serial.println(
      "DEBUG MODE: Skipping BME sensor - simulating 22.5°C, 45% humidity");
  inTemp = 22.5;     // Simulate 22.5°C
  inHumidity = 45.0; // Simulate 45% humidity
#else
  pinMode(PIN_BME_PWR, OUTPUT);
  digitalWrite(PIN_BME_PWR, HIGH);
  TwoWire I2C_bme = TwoWire(0);
  I2C_bme.begin(PIN_BME_SDA, PIN_BME_SCL, 100000); // 100kHz

#if defined(SENSOR_BME280)
  Serial.print(String(TXT_READING_FROM) + " BME280... ");
  Adafruit_BME280 bme;

  if (bme.begin(BME_ADDRESS, &I2C_bme)) {
#endif
#if defined(SENSOR_BME680)
    Serial.print(String(TXT_READING_FROM) + " BME680... ");
    Adafruit_BME680 bme(&I2C_bme);

    if (bme.begin(BME_ADDRESS)) {
#endif
      inTemp = bme.readTemperature();  // Celsius
      inHumidity = bme.readHumidity(); // %

      // check if BME readings are valid
      // note: readings are checked again before drawing to screen. If a reading
      //       is not a number (NAN) then an error occurred, a dash '-' will be
      //       displayed.
      if (std::isnan(inTemp) || std::isnan(inHumidity)) {
        statusStr = "BME " + String(TXT_READ_FAILED);
        Serial.println(statusStr);
      } else {
        Serial.println(TXT_SUCCESS);
      }
    } else {
      statusStr = "BME " + String(TXT_NOT_FOUND); // check wiring
      Serial.println(statusStr);
    }
    digitalWrite(PIN_BME_PWR, LOW);
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

  // 在进入深度睡眠前，让LED闪烁一段时间（例如10秒）
  Serial.println("LED will blink for 10 seconds before deep sleep");
  unsigned long blinkStartTime = millis();
  while (millis() - blinkStartTime < 10000) { // 闪烁10秒
    blinkLED1();
    delay(50); // 短暂延迟，不影响闪烁效果
  }

  // 关闭LED
  digitalWrite(PIN_LED1, LOW);

  // DEEP SLEEP
  beginDeepSleep(startTime, &timeInfo);
} // end setup

/* This will never run, but if it did (例如在调试模式下)，我们也会让LED闪烁
 */
void loop() {
  blinkLED1();
  delay(50); // 短暂延迟，不影响闪烁效果
} // end loop
