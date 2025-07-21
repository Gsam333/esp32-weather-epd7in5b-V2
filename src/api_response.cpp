/* API response deserialization for esp32-weather-epd.
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

#include "api_response.h"
#include "config.h"
#include <ArduinoJson.h>
#include <vector>

DeserializationError deserializeOneCall(WiFiClient &json,
                                        owm_resp_onecall_t &r) {
  int i;

  // 使用更大的缓冲区来解析JSON
  DynamicJsonDocument doc(65536); // 64KB

  DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 1
  Serial.println("[debug] doc.overflowed() : " + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error) {
    return error;
  }

  // 初始化结构体
  memset(&r, 0, sizeof(r));

  // 解析城市信息
  if (doc.containsKey("city")) {
    JsonObject city = doc["city"];
    if (city.containsKey("coord")) {
      r.lat = city["coord"]["lat"].as<float>();
      r.lon = city["coord"]["lon"].as<float>();
    }
    if (city.containsKey("name")) {
      r.timezone = city["name"].as<const char *>();
    }
    if (city.containsKey("timezone")) {
      r.timezone_offset = city["timezone"].as<int>();
    }
  }

  // 解析预报列表
  if (doc.containsKey("list") && doc["list"].size() > 0) {
    // 使用第一个预报项作为当前天气
    JsonObject firstForecast = doc["list"][0];

    // 设置当前天气数据
    r.current.dt = firstForecast["dt"].as<int64_t>();

    // 设置日出日落时间（如果可用）
    if (doc["city"].containsKey("sunrise") &&
        doc["city"].containsKey("sunset")) {
      r.current.sunrise = doc["city"]["sunrise"].as<int64_t>();
      r.current.sunset = doc["city"]["sunset"].as<int64_t>();
    }

    // 设置温度和体感温度
    if (firstForecast.containsKey("main")) {
      r.current.temp = firstForecast["main"]["temp"].as<float>();
      r.current.feels_like = firstForecast["main"]["feels_like"].as<float>();
      r.current.pressure = firstForecast["main"]["pressure"].as<int>();
      r.current.humidity = firstForecast["main"]["humidity"].as<int>();
    }

    // 设置云量
    if (firstForecast.containsKey("clouds")) {
      r.current.clouds = firstForecast["clouds"]["all"].as<int>();
    }

    // 设置能见度
    if (firstForecast.containsKey("visibility")) {
      r.current.visibility = firstForecast["visibility"].as<int>();
    }

    // 设置风速和风向
    if (firstForecast.containsKey("wind")) {
      r.current.wind_speed = firstForecast["wind"]["speed"].as<float>();
      r.current.wind_deg = firstForecast["wind"]["deg"].as<int>();
      if (firstForecast["wind"].containsKey("gust")) {
        r.current.wind_gust = firstForecast["wind"]["gust"].as<float>();
      }
    }

    // 设置降水概率
    if (firstForecast.containsKey("pop")) {
      float pop = firstForecast["pop"].as<float>();
      // 在One Call API中，pop是0-1的值，而在Forecast API中，它可能是0-100的值
      if (pop > 1.0) {
        pop = pop / 100.0f;
      }
    }

    // 设置降雨量和降雪量
    if (firstForecast.containsKey("rain")) {
      if (firstForecast["rain"].containsKey("3h")) {
        r.current.rain_1h = firstForecast["rain"]["3h"].as<float>() / 3.0f;
      }
    }

    if (firstForecast.containsKey("snow")) {
      if (firstForecast["snow"].containsKey("3h")) {
        r.current.snow_1h = firstForecast["snow"]["3h"].as<float>() / 3.0f;
      }
    }

    // 设置天气描述
    if (firstForecast.containsKey("weather") &&
        firstForecast["weather"].size() > 0) {
      JsonObject weather = firstForecast["weather"][0];
      r.current.weather.id = weather["id"].as<int>();
      r.current.weather.main = weather["main"].as<const char *>();
      r.current.weather.description = weather["description"].as<const char *>();
      r.current.weather.icon = weather["icon"].as<const char *>();
    }

    // 设置默认值
    r.current.uvi = 0.0f;
    r.current.dew_point = 0.0f;

    // 解析小时预报数据
    i = 0;
    for (JsonObject forecast : doc["list"].as<JsonArray>()) {
      if (i >= OWM_NUM_HOURLY)
        break;

      r.hourly[i].dt = forecast["dt"].as<int64_t>();

      if (forecast.containsKey("main")) {
        r.hourly[i].temp = forecast["main"]["temp"].as<float>();
        r.hourly[i].feels_like = forecast["main"]["feels_like"].as<float>();
        r.hourly[i].pressure = forecast["main"]["pressure"].as<int>();
        r.hourly[i].humidity = forecast["main"]["humidity"].as<int>();
      }

      if (forecast.containsKey("clouds")) {
        r.hourly[i].clouds = forecast["clouds"]["all"].as<int>();
      }

      if (forecast.containsKey("visibility")) {
        r.hourly[i].visibility = forecast["visibility"].as<int>();
      }

      if (forecast.containsKey("wind")) {
        r.hourly[i].wind_speed = forecast["wind"]["speed"].as<float>();
        r.hourly[i].wind_deg = forecast["wind"]["deg"].as<int>();
        if (forecast["wind"].containsKey("gust")) {
          r.hourly[i].wind_gust = forecast["wind"]["gust"].as<float>();
        }
      }

      if (forecast.containsKey("pop")) {
        r.hourly[i].pop = forecast["pop"].as<float>();
      }

      if (forecast.containsKey("rain")) {
        if (forecast["rain"].containsKey("3h")) {
          r.hourly[i].rain_1h = forecast["rain"]["3h"].as<float>() / 3.0f;
        }
      }

      if (forecast.containsKey("snow")) {
        if (forecast["snow"].containsKey("3h")) {
          r.hourly[i].snow_1h = forecast["snow"]["3h"].as<float>() / 3.0f;
        }
      }

      if (forecast.containsKey("weather") && forecast["weather"].size() > 0) {
        JsonObject weather = forecast["weather"][0];
        r.hourly[i].weather.id = weather["id"].as<int>();
        r.hourly[i].weather.main = weather["main"].as<const char *>();
        r.hourly[i].weather.description =
            weather["description"].as<const char *>();
        r.hourly[i].weather.icon = weather["icon"].as<const char *>();
      }

      // 设置默认值
      r.hourly[i].dew_point = 0.0f;
      r.hourly[i].uvi = 0.0f;

      i++;
    }

    // 从3小时预报数据中提取每日预报
    // 初始化每日预报数组
    for (int j = 0; j < OWM_NUM_DAILY; j++) {
      r.daily[j] = {};
    }

    // 获取当前日期
    time_t now;
    time(&now);
    struct tm *timeinfo = localtime(&now);
    int currentDay = timeinfo->tm_mday;

    // 处理每日预报数据
    i = 0; // 每日预报索引
    int lastDay = -1;
    float minTemp = 1000.0f;
    float maxTemp = -1000.0f;

    // 遍历所有3小时预报，按天分组
    for (JsonObject forecast : doc["list"].as<JsonArray>()) {
      time_t forecastTime = forecast["dt"].as<int64_t>();
      struct tm *forecastTm = localtime(&forecastTime);
      int forecastDay = forecastTm->tm_mday;

      // 如果是新的一天
      if (forecastDay != lastDay && lastDay != -1) {
        if (i >= OWM_NUM_DAILY)
          break;

        // 完成上一天的数据
        r.daily[i].temp.min = minTemp;
        r.daily[i].temp.max = maxTemp;

        // 重置最高最低温度
        minTemp = 1000.0f;
        maxTemp = -1000.0f;
        i++;
      }

      // 记录这一天
      lastDay = forecastDay;

      // 如果是第一次遇到这一天
      if (i < OWM_NUM_DAILY) {
        float temp = forecast["main"]["temp"].as<float>();

        // 更新最高最低温度
        if (temp < minTemp)
          minTemp = temp;
        if (temp > maxTemp)
          maxTemp = temp;

        // 设置基本信息
        r.daily[i].dt = forecastTime;
        if (doc["city"].containsKey("sunrise") &&
            doc["city"].containsKey("sunset")) {
          r.daily[i].sunrise = doc["city"]["sunrise"].as<int64_t>();
          r.daily[i].sunset = doc["city"]["sunset"].as<int64_t>();
        }

        // 设置天气信息
        if (forecast.containsKey("weather") && forecast["weather"].size() > 0) {
          JsonObject weather = forecast["weather"][0];
          r.daily[i].weather.id = weather["id"].as<int>();
          r.daily[i].weather.main = weather["main"].as<const char *>();
          r.daily[i].weather.description =
              weather["description"].as<const char *>();
          r.daily[i].weather.icon = weather["icon"].as<const char *>();
        }

        // 设置其他信息
        if (forecast.containsKey("main")) {
          r.daily[i].pressure = forecast["main"]["pressure"].as<int>();
          r.daily[i].humidity = forecast["main"]["humidity"].as<int>();
        }

        if (forecast.containsKey("clouds")) {
          r.daily[i].clouds = forecast["clouds"]["all"].as<int>();
        }

        if (forecast.containsKey("wind")) {
          r.daily[i].wind_speed = forecast["wind"]["speed"].as<float>();
          r.daily[i].wind_deg = forecast["wind"]["deg"].as<int>();
          if (forecast["wind"].containsKey("gust")) {
            r.daily[i].wind_gust = forecast["wind"]["gust"].as<float>();
          }
        }

        if (forecast.containsKey("pop")) {
          r.daily[i].pop = forecast["pop"].as<float>();
        }

        // 设置默认值
        r.daily[i].moonrise = 0;
        r.daily[i].moonset = 0;
        r.daily[i].moon_phase = 0.0f;
        r.daily[i].dew_point = 0.0f;
        r.daily[i].uvi = 0.0f;
        r.daily[i].visibility = 10000;

        // 设置温度
        r.daily[i].temp.day = temp;
        r.daily[i].temp.night = temp;
        r.daily[i].temp.eve = temp;
        r.daily[i].temp.morn = temp;

        // 设置体感温度
        if (forecast.containsKey("main") &&
            forecast["main"].containsKey("feels_like")) {
          float feels_like = forecast["main"]["feels_like"].as<float>();
          r.daily[i].feels_like.day = feels_like;
          r.daily[i].feels_like.night = feels_like;
          r.daily[i].feels_like.eve = feels_like;
          r.daily[i].feels_like.morn = feels_like;
        }

        // 雨雪数据
        if (forecast.containsKey("rain")) {
          if (forecast["rain"].containsKey("3h")) {
            r.daily[i].rain = forecast["rain"]["3h"].as<float>();
          }
        }

        if (forecast.containsKey("snow")) {
          if (forecast["snow"].containsKey("3h")) {
            r.daily[i].snow = forecast["snow"]["3h"].as<float>();
          }
        }
      }
    }

    // 处理最后一天的数据
    if (i < OWM_NUM_DAILY) {
      r.daily[i].temp.min = minTemp;
      r.daily[i].temp.max = maxTemp;
    }
  }

  return error;
} // end deserializeOneCall

DeserializationError deserializeAirQuality(WiFiClient &json,
                                           owm_resp_air_pollution_t &r) {
  int i = 0;

  // 使用更大的缓冲区来解析JSON
  DynamicJsonDocument doc(16384); // 16KB

  DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 1
  Serial.println("[debug] doc.overflowed() : " + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error) {
    return error;
  }

  r.coord.lat = doc["coord"]["lat"].as<float>();
  r.coord.lon = doc["coord"]["lon"].as<float>();

  for (JsonObject list : doc["list"].as<JsonArray>()) {
    r.main_aqi[i] = list["main"]["aqi"].as<int>();

    JsonObject list_components = list["components"];
    r.components.co[i] = list_components["co"].as<float>();
    r.components.no[i] = list_components["no"].as<float>();
    r.components.no2[i] = list_components["no2"].as<float>();
    r.components.o3[i] = list_components["o3"].as<float>();
    r.components.so2[i] = list_components["so2"].as<float>();
    r.components.pm2_5[i] = list_components["pm2_5"].as<float>();
    r.components.pm10[i] = list_components["pm10"].as<float>();
    r.components.nh3[i] = list_components["nh3"].as<float>();

    r.dt[i] = list["dt"].as<int64_t>();

    if (i == OWM_NUM_AIR_POLLUTION - 1) {
      break;
    }
    ++i;
  }

  return error;
} // end deserializeAirQuality