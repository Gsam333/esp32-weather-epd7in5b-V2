// #ifndef __AMAP_CLIENT_H__
// #define __AMAP_CLIENT_H__

// #include "amap_response.h"
// #include "config.h"
// #include <Arduino.h>
// #ifdef USE_HTTP
// #include <WiFiClient.h>
// #else
// #include <WiFiClientSecure.h>
// #endif

// // 初始化高德天气API客户端
// bool initAmapClient();

// // 获取实时天气数据
// int getAmapWeatherLive(WiFiClient &client, amap_weather_live_t &weather);

// // 获取天气预报数据
// int getAmapWeatherForecast(WiFiClient &client,
//                            amap_weather_forecast_t &forecast);

// // 获取空气质量数据
// int getAmapAirQuality(WiFiClient &client, amap_air_quality_t &air);

// // 获取天气预警信息
// int getAmapWeatherAlert(WiFiClient &client,
//                         std::vector<amap_weather_alert_t> &alerts);

// // 获取生活指数数据
// int getAmapLifeIndex(WiFiClient &client, amap_life_index_t &index);

// #endif
