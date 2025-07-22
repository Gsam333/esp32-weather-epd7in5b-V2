/* Client side utilities for esp32-weather-epd.
 * Copyright (C) 2022-2024  Luke Marzen
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

// built-in C++ libraries
#include <cstring>
#include <vector>

// arduino/esp32 libraries
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_sntp.h>
#include <time.h>

// additional libraries
#include <Adafruit_BusIO_Register.h>
#include <ArduinoJson.h>

// header files
#include "_locale.h"
#include "api_response.h"
#include "aqi.h"
#include "client_utils.h"
#include "config.h"
#include "display_utils.h"
#include "renderer.h"
#ifndef USE_HTTP
#include <WiFiClientSecure.h>
#endif

#ifdef USE_HTTP
static const uint16_t OWM_PORT = 80;
#else
static const uint16_t OWM_PORT = 443;
#endif

/* Power-on and connect WiFi.
 * Takes int parameter to store WiFi RSSI, or “Received Signal Strength
 * Indicator"
 *
 * Returns WiFi status.
 */
wl_status_t startWiFi(int &wifiRSSI) {
  WiFi.mode(WIFI_STA);
  Serial.printf("%s '%s'", TXT_CONNECTING_TO, WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // timeout if WiFi does not connect in WIFI_TIMEOUT ms from now
  unsigned long timeout = millis() + WIFI_TIMEOUT;
  wl_status_t connection_status = WiFi.status();

  while ((connection_status != WL_CONNECTED) && (millis() < timeout)) {
    Serial.print(".");
    delay(50);
    connection_status = WiFi.status();
  }
  Serial.println();

  if (connection_status == WL_CONNECTED) {
    wifiRSSI = WiFi.RSSI(); // get WiFi signal strength now, because the WiFi
                            // will be turned off to save power!
    Serial.println("IP: " + WiFi.localIP().toString());
  } else {
    Serial.printf("%s '%s'\n", TXT_COULD_NOT_CONNECT_TO, WIFI_SSID);
  }
  return connection_status;
} // startWiFi

/* Disconnect and power-off WiFi.
 */
void killWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
} // killWiFi

/* Prints the local time to serial monitor.
 *
 * Returns true if getting local time was a success, otherwise false.
 */
bool printLocalTime(tm *timeInfo) {
  int attempts = 0;
  while (!getLocalTime(timeInfo) && attempts++ < 3) {
    Serial.println(TXT_FAILED_TO_GET_TIME);
    return false;
  }
  Serial.println(timeInfo, "%A, %B %d, %Y %H:%M:%S");
  return true;
} // printLocalTime

/* Waits for NTP server time sync, adjusted for the time zone specified in
 * config.cpp.
 *
 * Returns true if time was set successfully, otherwise false.
 *
 * Note: Must be connected to WiFi to get time from NTP server.
 */
bool waitForSNTPSync(tm *timeInfo) {
  // Wait for SNTP synchronization to complete
  unsigned long timeout = millis() + NTP_TIMEOUT;
  if ((sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) &&
      (millis() < timeout)) {
    Serial.print(TXT_WAITING_FOR_SNTP);
    delay(100); // ms
    while ((sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) &&
           (millis() < timeout)) {
      Serial.print(".");
      delay(100); // ms
    }
    Serial.println();
  }
  return printLocalTime(timeInfo);
} // waitForSNTPSync

/* Perform an HTTP GET request to OpenWeatherMap's "Current Weather" API
 * (2.5/weather) This is a simpler API that provides current weather data only.
 * If data is received, it will be used to populate the current weather section.
 *
 * Returns the HTTP Status Code.
 */
#ifdef USE_HTTP
int getOWMcurrentWeather(WiFiClient &client, owm_current_t &current)
#else
int getOWMcurrentWeather(WiFiClientSecure &client, owm_current_t &current)
#endif
{
  int attempts = 0;
  bool rxSuccess = false;
  String uri = "/data/2.5/weather?lat=" + LAT + "&lon=" + LON +
               "&units=standard&lang=" + OWM_LANG + "&appid=" + OWM_APIKEY;

  // Print the complete URL for debugging (with real API key for verification)
  String fullUrl =
      "http" + String(OWM_PORT == 443 ? "s" : "") + "://" + OWM_ENDPOINT + uri;
  Serial.println("DEBUG: Complete API Request URL:");
  Serial.println(fullUrl);

  // This string is printed to terminal to help with debugging. The API key is
  // censored to reduce the risk of users exposing their key.
  String sanitizedUri = "http" + String(OWM_PORT == 443 ? "s" : "") + "://" +
                        OWM_ENDPOINT + "/data/2.5/weather?lat=" + LAT +
                        "&lon=" + LON + "&units=standard&lang=" + OWM_LANG +
                        "&appid={API key}";

  Serial.print(TXT_ATTEMPTING_HTTP_REQ);
  Serial.println(": " + sanitizedUri);
  int httpResponse = 0;
  while (!rxSuccess && attempts < 3) {
    wl_status_t connection_status = WiFi.status();
    if (connection_status != WL_CONNECTED) {
      // -512 offset distinguishes these errors from httpClient errors
      return -512 - static_cast<int>(connection_status);
    }

    HTTPClient http;
    http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT); // default 5000ms
    http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT);        // default 5000ms
    http.begin(client, OWM_ENDPOINT, OWM_PORT, uri);
    httpResponse = http.GET();
    if (httpResponse == HTTP_CODE_OK) {
      // Parse the current weather JSON response
      String payload = http.getString();
      Serial.println("DEBUG: Current Weather API Response:");
      Serial.println(payload);

      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        // Parse current weather data
        current.dt = doc["dt"];
        current.temp = doc["main"]["temp"];
        current.feels_like = doc["main"]["feels_like"];
        current.pressure = doc["main"]["pressure"];
        current.humidity = doc["main"]["humidity"];
        current.visibility = doc["visibility"];
        current.uvi = 0; // Not available in current weather API
        current.clouds = doc["clouds"]["all"];
        current.wind_speed = doc["wind"]["speed"];
        current.wind_deg = doc["wind"]["deg"];
        current.wind_gust = doc["wind"]["gust"] | 0.0;
        current.sunrise = doc["sys"]["sunrise"];
        current.sunset = doc["sys"]["sunset"];

        // Weather description
        current.weather.id = doc["weather"][0]["id"];
        current.weather.main = doc["weather"][0]["main"].as<String>();
        current.weather.description =
            doc["weather"][0]["description"].as<String>();
        current.weather.icon = doc["weather"][0]["icon"].as<String>();

        // Rain and snow (if available)
        current.rain_1h = doc["rain"]["1h"] | 0.0;
        current.snow_1h = doc["snow"]["1h"] | 0.0;

        rxSuccess = true;
      } else {
        Serial.println("Failed to parse current weather JSON");
        httpResponse = -256 - static_cast<int>(error.code());
      }
    }
    client.stop();
    http.end();
    Serial.println("  " + String(httpResponse, DEC) + " " +
                   getHttpResponsePhrase(httpResponse));
    ++attempts;
  }

  return httpResponse;
} // getOWMcurrentWeather

/* Perform an HTTP GET request to OpenWeatherMap's "One Call" API
 * If data is received, it will be parsed and stored in the global variable
 * owm_onecall.
 *
 * Returns the HTTP Status Code.
 */
#ifdef USE_HTTP
int getOWMonecall(WiFiClient &client, owm_resp_onecall_t &r)
#else
int getOWMonecall(WiFiClientSecure &client, owm_resp_onecall_t &r)
#endif
{
  int attempts = 0;
  bool rxSuccess = false;
  DeserializationError jsonErr = {};
  String uri = "/data/2.5/forecast?lat=" + LAT + "&lon=" + LON +
               "&lang=" + OWM_LANG +
               "&units=standard&cnt=40"; // 获取完整的5天数据（40个数据点）

  // This string is printed to terminal to help with debugging. The API key is
  // censored to reduce the risk of users exposing their key.
  String sanitizedUri = OWM_ENDPOINT + uri + "&appid={API key}";

  uri += "&appid=" + OWM_APIKEY;

  Serial.print(TXT_ATTEMPTING_HTTP_REQ);
  Serial.println(": " + sanitizedUri);
  int httpResponse = 0;
  while (!rxSuccess && attempts < 3) {
    wl_status_t connection_status = WiFi.status();
    if (connection_status != WL_CONNECTED) {
      // -512 offset distinguishes these errors from httpClient errors
      return -512 - static_cast<int>(connection_status);
    }

    HTTPClient http;
    http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT); // default 5000ms
    http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT);        // default 5000ms
    http.begin(client, OWM_ENDPOINT, OWM_PORT, uri);
    httpResponse = http.GET();
    if (httpResponse == HTTP_CODE_OK) {
      jsonErr = deserializeOneCall(http.getStream(), r);
      if (jsonErr) {
        // -256 offset distinguishes these errors from httpClient errors
        httpResponse = -256 - static_cast<int>(jsonErr.code());
      }
      rxSuccess = !jsonErr;
    }
    client.stop();
    http.end();
    Serial.println("  " + String(httpResponse, DEC) + " " +
                   getHttpResponsePhrase(httpResponse));
    ++attempts;
  }

  return httpResponse;
} // getOWMonecall

/* Perform an HTTP GET request to OpenWeatherMap's "Air Pollution" API
 * If data is received, it will be parsed and stored in the global variable
 * owm_air_pollution.
 *
 * Returns the HTTP Status Code.
 */
#ifdef USE_HTTP
int getOWMairpollution(WiFiClient &client, owm_resp_air_pollution_t &r)
#else
int getOWMairpollution(WiFiClientSecure &client, owm_resp_air_pollution_t &r)
#endif
{
  int attempts = 0;
  bool rxSuccess = false;
  DeserializationError jsonErr = {};

  // set start and end to appropriate values so that the last 24 hours of air
  // pollution history is returned. Unix, UTC.
  time_t now;
  int64_t end = time(&now);
  // minus 1 is important here, otherwise we could get an extra hour of history
  int64_t start = end - ((3600 * OWM_NUM_AIR_POLLUTION) - 1);
  char endStr[22];
  char startStr[22];
  sprintf(endStr, "%lld", end);
  sprintf(startStr, "%lld", start);
  String uri = "/data/2.5/air_pollution/history?lat=" + LAT + "&lon=" + LON +
               "&start=" + startStr + "&end=" + endStr + "&appid=" + OWM_APIKEY;
  // This string is printed to terminal to help with debugging. The API key is
  // censored to reduce the risk of users exposing their key.
  String sanitizedUri = OWM_ENDPOINT +
                        "/data/2.5/air_pollution/history?lat=" + LAT +
                        "&lon=" + LON + "&start=" + startStr +
                        "&end=" + endStr + "&appid={API key}";

  Serial.print(TXT_ATTEMPTING_HTTP_REQ);
  Serial.println(": " + sanitizedUri);
  int httpResponse = 0;
  while (!rxSuccess && attempts < 3) {
    wl_status_t connection_status = WiFi.status();
    if (connection_status != WL_CONNECTED) {
      // -512 offset distinguishes these errors from httpClient errors
      return -512 - static_cast<int>(connection_status);
    }

    HTTPClient http;
    http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT); // default 5000ms
    http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT);        // default 5000ms
    http.begin(client, OWM_ENDPOINT, OWM_PORT, uri);
    httpResponse = http.GET();
    if (httpResponse == HTTP_CODE_OK) {
      jsonErr = deserializeAirQuality(http.getStream(), r);
      if (jsonErr) {
        // -256 offset to distinguishes these errors from httpClient errors
        httpResponse = -256 - static_cast<int>(jsonErr.code());
      }
      rxSuccess = !jsonErr;
    }
    client.stop();
    http.end();
    Serial.println("  " + String(httpResponse, DEC) + " " +
                   getHttpResponsePhrase(httpResponse));
    ++attempts;
  }

  return httpResponse;
} // getOWMairpollution

/* Prints debug information about heap usage.
 */
void printHeapUsage() {
  Serial.println("[debug] Heap Size       : " + String(ESP.getHeapSize()) +
                 " B");
  Serial.println("[debug] Available Heap  : " + String(ESP.getFreeHeap()) +
                 " B");
  Serial.println("[debug] Min Free Heap   : " + String(ESP.getMinFreeHeap()) +
                 " B");
  Serial.println("[debug] Max Allocatable : " + String(ESP.getMaxAllocHeap()) +
                 " B");
  return;
}
