# 调试日志和故障排除指南

## 🎯 核心问题：-258错误调试

### 问题定义
**错误代码**：-258（反序列化输入不完整）
**API**：`api.openweathermap.org/data/2.5/forecast`
**根本原因**：ESP32内存不足以解析25KB的JSON响应

### 1. -258错误的直接调试

**症状：**
- API调用返回-258错误
- JSON解析失败
- 连续出现相同错误直到重启
- 内存碎片化严重

**关键调试代码：**
```cpp
// 在getOWMonecall函数中添加详细调试
int getOWMonecallWithDebug(WiFiClientSecure &client, owm_resp_onecall_t &r) {
    // 调用前内存状态
    size_t freeHeapBefore = ESP.getFreeHeap();
    size_t maxAllocBefore = ESP.getMaxAllocHeap();
    float fragmentationBefore = 1.0 - (float)maxAllocBefore / freeHeapBefore;
    
    LOG_DEBUG("=== API调用前内存状态 ===");
    LOG_DEBUG("可用堆内存: " + String(freeHeapBefore) + " 字节");
    LOG_DEBUG("最大可分配: " + String(maxAllocBefore) + " 字节");
    LOG_DEBUG("内存碎片化: " + String(fragmentationBefore * 100, 1) + "%");
    
    // 内存预检查
    if (freeHeapBefore < 60000) {
        LOG_ERROR("内存不足，无法执行API调用");
        return -1002; // 自定义：内存不足
    }
    
    HTTPClient http;
    http.setTimeout(60000);
    http.begin(client, OWM_ENDPOINT, OWM_PORT, uri);
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        int responseSize = http.getSize();
        LOG_DEBUG("响应大小: " + String(responseSize) + " 字节");
        
        // 响应大小检查
        if (responseSize > 25000) {
            LOG_ERROR("响应过大: " + String(responseSize) + " 字节");
            http.end();
            return -1001; // 自定义：响应过大
        }
        
        // 解析前再次检查内存
        size_t freeHeapBeforeParse = ESP.getFreeHeap();
        if (freeHeapBeforeParse < 60000) {
            LOG_ERROR("解析前内存不足: " + String(freeHeapBeforeParse) + " 字节");
            http.end();
            return -1002; // 自定义：内存不足
        }
        
        // 尝试解析
        DeserializationError error = deserializeOneCall(http.getStream(), r);
        
        if (error) {
            LOG_ERROR("JSON解析失败: " + String(error.c_str()));
            LOG_ERROR("错误代码: " + String(error.code()));
            
            // 解析失败后的内存状态
            size_t freeHeapAfterFail = ESP.getFreeHeap();
            LOG_ERROR("解析失败后可用内存: " + String(freeHeapAfterFail) + " 字节");
            
            http.end();
            return -258; // 反序列化失败
        }
        
        LOG_INFO("JSON解析成功");
    }
    
    http.end();
    return httpCode;
}
```

### 2. 深度睡眠模式下的连续错误调试

**症状：**
- 连续多个睡眠周期都出现-258错误
- 错误状态在深度睡眠后仍然持续
- 需要手动重启才能恢复

**深度睡眠错误跟踪代码：**
```cpp
// DeepSleepErrorManager调试代码
void DeepSleepErrorManager::debugErrorState() {
    LOG_DEBUG("=== 深度睡眠错误状态 ===");
    LOG_DEBUG("连续错误次数: " + String(errorState.consecutiveErrors));
    LOG_DEBUG("最后错误时间: " + String(errorState.lastErrorTime));
    LOG_DEBUG("最后重启时间: " + String(errorState.lastRestartTime));
    LOG_DEBUG("总重启次数: " + String(errorState.totalRestarts));
    
    // 检查是否需要重启
    bool shouldRestart = shouldTriggerRestart();
    LOG_DEBUG("是否应该重启: " + String(shouldRestart ? "是" : "否"));
    
    if (shouldRestart) {
        time_t now = time(nullptr);
        LOG_DEBUG("距上次重启: " + String((now - errorState.lastRestartTime) / 60) + " 分钟");
    }
}

// 在主循环中添加
void loop() {
    // 启动时调试错误状态
    errorManager.debugErrorState();
    
    // ... 其他代码 ...
    
    // API调用后
    int apiResult = getOWMonecallWithDebug(client, owm_onecall);
    LOG_DEBUG("API调用结果: " + String(apiResult));
    
    bool willRestart = errorManager.handleApiResult(apiResult);
    if (willRestart) {
        LOG_ERROR("即将执行软重启");
        // 不会到达这里，因为会重启
    } else {
        LOG_DEBUG("继续正常流程，准备深度睡眠");
    }
}
```

**NVS状态持久化调试：**
```cpp
// 验证NVS存储是否正常工作
void testNVSPersistence() {
    LOG_DEBUG("=== 测试NVS持久化 ===");
    
    // 写入测试数据
    ErrorState testState = {2, time(nullptr), 0, 1};
    saveErrorState(testState);
    LOG_DEBUG("已保存测试状态: 错误次数=" + String(testState.consecutiveErrors));
    
    // 读取数据
    ErrorState loadedState = {};
    loadErrorState(loadedState);
    LOG_DEBUG("加载的状态: 错误次数=" + String(loadedState.consecutiveErrors));
    
    // 验证一致性
    if (loadedState.consecutiveErrors == testState.consecutiveErrors) {
        LOG_DEBUG("NVS持久化测试通过");
    } else {
        LOG_ERROR("NVS持久化测试失败");
    }
}
```

### 3. 软重启机制调试

**症状：**
- 软重启触发但问题仍存在
- 重启频率过高或过低
- 重启后状态恢复异常

**软重启调试代码：**
```cpp
// 软重启决策调试
void DeepSleepErrorManager::debugRestartDecision() {
    LOG_DEBUG("=== 软重启决策分析 ===");
    LOG_DEBUG("连续错误次数: " + String(errorState.consecutiveErrors));
    LOG_DEBUG("最大允许错误: " + String(MAX_CONSECUTIVE_ERRORS));
    
    time_t now = time(nullptr);
    int minutesSinceLastRestart = (now - errorState.lastRestartTime) / 60;
    LOG_DEBUG("距上次重启: " + String(minutesSinceLastRestart) + " 分钟");
    LOG_DEBUG("最小重启间隔: " + String(MIN_RESTART_INTERVAL / 60) + " 分钟");
    
    bool shouldRestart = shouldTriggerRestart();
    LOG_DEBUG("是否应该重启: " + String(shouldRestart ? "是" : "否"));
}

// 重启前状态保存
void DeepSleepErrorManager::performSoftRestart() {
    LOG_ERROR("=== 执行软重启 ===");
    LOG_ERROR("重启原因: 连续" + String(errorState.consecutiveErrors) + "次-258错误");
    
    // 保存重启信息
    errorState.lastRestartTime = time(nullptr);
    errorState.totalRestarts++;
    saveErrorState();
    
    // 保存重启原因到单独的NVS区域
    nvs_handle_t handle;
    nvs_open("restart_info", NVS_READWRITE, &handle);
    nvs_set_str(handle, "reason", "consecutive_258_errors");
    nvs_set_i32(handle, "error_count", errorState.consecutiveErrors);
    nvs_set_i64(handle, "restart_time", errorState.lastRestartTime);
    nvs_commit(handle);
    nvs_close(handle);
    
    // 更新显示
    updateDisplay("连续API错误，系统重启中...");
    
    LOG_ERROR("即将重启设备...");
    delay(2000); // 确保日志输出
    
    ESP.restart();
}
```

### 4. 缓存系统调试

**症状：**
- 缓存存储失败
- 缓存数据损坏
- 无法加载缓存数据

**缓存调试代码：**
```cpp
// SimpleForecastCache调试方法
void SimpleForecastCache::debugCacheState() {
    LOG_DEBUG("=== 缓存状态调试 ===");
    
    // 检查缓存是否存在
    bool hasCache = hasCachedData();
    LOG_DEBUG("缓存存在: " + String(hasCache ? "是" : "否"));
    
    if (hasCache) {
        time_t cacheAge = getCacheAge();
        int ageMinutes = (time(nullptr) - cacheAge) / 60;
        LOG_DEBUG("缓存年龄: " + String(ageMinutes) + " 分钟");
        
        // 检查缓存有效性
        CacheMetadata meta;
        nvs_handle_t handle;
        if (nvs_open(CACHE_NAMESPACE, NVS_READONLY, &handle) == ESP_OK) {
            size_t required_size = sizeof(meta);
            if (nvs_get_blob(handle, META_KEY, &meta, &required_size) == ESP_OK) {
                LOG_DEBUG("缓存数据大小: " + String(meta.dataSize) + " 字节");
                LOG_DEBUG("缓存校验和: " + String(meta.checksum));
                LOG_DEBUG("缓存有效性: " + String(meta.isValid ? "有效" : "无效"));
            }
            nvs_close(handle);
        }
    }
}

// 缓存操作日志
bool SimpleForecastCache::storeForecastData(const owm_resp_onecall_t& data) {
    LOG_DEBUG("=== 存储预报数据到缓存 ===");
    
    size_t dataSize = sizeof(data);
    LOG_DEBUG("数据大小: " + String(dataSize) + " 字节");
    
    // 计算校验和
    uint32_t checksum = calculateChecksum(&data, dataSize);
    LOG_DEBUG("数据校验和: " + String(checksum));
    
    // 存储到NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(CACHE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        LOG_ERROR("无法打开NVS命名空间: " + String(esp_err_to_name(err)));
        return false;
    }
    
    // 存储数据
    err = nvs_set_blob(handle, DATA_KEY, &data, dataSize);
    if (err != ESP_OK) {
        LOG_ERROR("存储数据失败: " + String(esp_err_to_name(err)));
        nvs_close(handle);
        return false;
    }
    
    // 存储元数据
    CacheMetadata meta = {time(nullptr), true, dataSize, checksum};
    err = nvs_set_blob(handle, META_KEY, &meta, sizeof(meta));
    if (err != ESP_OK) {
        LOG_ERROR("存储元数据失败: " + String(esp_err_to_name(err)));
        nvs_close(handle);
        return false;
    }
    
    nvs_commit(handle);
    nvs_close(handle);
    
    LOG_DEBUG("缓存存储成功");
    return true;
}
```

## 调试配置

### 简化的日志宏

```cpp
// 添加到config.h或新建debug.h
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
  #define LOG_ERROR(msg)   Serial.println("[错误] " + String(msg))
  #define LOG_INFO(msg)    Serial.println("[信息] " + String(msg))
  #define LOG_DEBUG(msg)   Serial.println("[调试] " + String(msg))
#else
  #define LOG_ERROR(msg)
  #define LOG_INFO(msg)
  #define LOG_DEBUG(msg)
#endif

// 专用日志宏
#define LOG_API(msg)      LOG_INFO("[API] " + String(msg))
#define LOG_MEMORY(msg)   LOG_DEBUG("[内存] " + String(msg))
#define LOG_CACHE(msg)    LOG_INFO("[缓存] " + String(msg))
#define LOG_RESTART(msg)  LOG_ERROR("[重启] " + String(msg))
```

## 关键监控函数

### 内存状态监控

```cpp
void logMemoryState(const char* operation) {
    size_t freeHeap = ESP.getFreeHeap();
    size_t maxAlloc = ESP.getMaxAllocHeap();
    float fragmentation = 1.0 - (float)maxAlloc / freeHeap;
    
    LOG_MEMORY("=== " + String(operation) + " 内存状态 ===");
    LOG_MEMORY("可用: " + String(freeHeap) + "B, 最大分配: " + String(maxAlloc) + "B");
    LOG_MEMORY("碎片化: " + String(fragmentation * 100, 1) + "%");
    
    if (freeHeap < 60000) {
        LOG_ERROR("内存不足警告: " + String(freeHeap) + "B < 60KB");
    }
}
```

### API调用监控

```cpp
void logApiCall(int result, size_t responseSize, unsigned long duration) {
    LOG_API("结果: " + String(result) + ", 大小: " + String(responseSize) + "B, 耗时: " + String(duration) + "ms");
    
    if (result == -258) {
        LOG_ERROR("检测到-258错误（反序列化失败）");
    } else if (result == -1001) {
        LOG_ERROR("响应过大: " + String(responseSize) + "B > 25KB");
    } else if (result == -1002) {
        LOG_ERROR("内存不足，无法解析响应");
    }
}
```

### 错误状态监控

```cpp
void logErrorState(const ErrorState& state) {
    LOG_DEBUG("=== 错误状态 ===");
    LOG_DEBUG("连续错误: " + String(state.consecutiveErrors) + "/3");
    LOG_DEBUG("最后错误: " + String((time(nullptr) - state.lastErrorTime) / 60) + "分钟前");
    LOG_DEBUG("总重启次数: " + String(state.totalRestarts));
    
    if (state.consecutiveErrors >= 3) {
        LOG_ERROR("达到软重启阈值");
    }
}
```

## 错误代码快速参考

### 核心错误代码

| 代码 | 描述 | 立即处理 |
|------|------|----------|
| **-258** | **反序列化输入不完整** | **检查内存，使用缓存，记录连续错误** |
| -1001 | 响应过大(>25KB) | 使用缓存数据 |
| -1002 | 内存不足(<60KB) | 内存清理，使用缓存 |
| -11   | HTTP超时 | 检查网络，使用缓存 |
| 200   | 成功 | 重置错误计数，更新缓存 |

### 自定义错误处理流程

```cpp
void handleApiError(int errorCode) {
    switch (errorCode) {
        case -258:
            LOG_ERROR("核心问题：-258反序列化失败");
            errorManager.recordError();
            if (cache.hasCachedData()) {
                cache.loadForecastData(owm_onecall);
                updateDisplay("使用缓存数据");
            }
            break;
            
        case -1001:
            LOG_ERROR("响应过大，跳过此次更新");
            // 不记录为连续错误，因为这是服务器问题
            break;
            
        case -1002:
            LOG_ERROR("内存不足，需要清理");
            memoryChecker.performMemoryCleanup();
            errorManager.recordError(); // 记录为内存相关错误
            break;
            
        default:
            LOG_ERROR("其他API错误: " + String(errorCode));
            // 网络错误不记录为连续错误
            break;
    }
}
```

## 测试场景

### 核心测试：模拟-258错误

```cpp
// 在getOWMonecall函数中添加测试开关
#ifdef TEST_258_ERROR
static int test258Count = 0;

int getOWMonecallWithTest(WiFiClientSecure &client, owm_resp_onecall_t &r) {
    // 模拟连续3次-258错误来测试软重启
    if (test258Count < 3) {
        test258Count++;
        LOG_DEBUG("模拟-258错误，第" + String(test258Count) + "次");
        return -258;
    }
    
    // 第4次调用返回成功，测试错误重置
    if (test258Count == 3) {
        test258Count++;
        LOG_DEBUG("模拟API成功，测试错误重置");
        return 200; // 成功
    }
    
    // 正常API调用
    return getOWMonecallOriginal(client, r);
}
#endif
```

### 深度睡眠周期测试

```cpp
// 测试跨深度睡眠的错误状态持久化
void testDeepSleepErrorPersistence() {
    LOG_DEBUG("=== 深度睡眠错误持久化测试 ===");
    
    // 模拟错误状态
    ErrorState testState = {2, time(nullptr), 0, 1};
    
    // 保存状态
    errorManager.saveErrorState(testState);
    LOG_DEBUG("保存测试状态：连续错误=" + String(testState.consecutiveErrors));
    
    // 模拟深度睡眠（清除内存状态）
    LOG_DEBUG("模拟深度睡眠，清除内存状态");
    
    // 重新加载状态
    errorManager.loadErrorState();
    LOG_DEBUG("重新加载状态");
    
    // 验证状态是否正确恢复
    errorManager.debugErrorState();
}
```

### 内存不足测试

```cpp
// 模拟内存不足场景
void testMemoryInsufficient() {
    LOG_DEBUG("=== 内存不足测试 ===");
    
    // 分配大量内存模拟不足
    size_t allocSize = ESP.getFreeHeap() - 50000; // 留下50KB
    void* testAlloc = malloc(allocSize);
    
    LOG_DEBUG("分配了" + String(allocSize) + "字节，剩余" + String(ESP.getFreeHeap()) + "字节");
    
    // 测试内存检查
    bool hasEnoughMemory = memoryChecker.checkMemoryBeforeApiCall();
    LOG_DEBUG("内存检查结果: " + String(hasEnoughMemory ? "充足" : "不足"));
    
    // 清理测试内存
    free(testAlloc);
    LOG_DEBUG("测试完成，释放测试内存");
}
```

### 快速验证脚本

```cpp
// 完整的功能验证
void runQuickTest() {
    LOG_INFO("=== 快速功能验证 ===");
    
    // 1. 测试错误管理器初始化
    errorManager.initialize();
    LOG_INFO("✓ 错误管理器初始化");
    
    // 2. 测试缓存系统
    if (cache.hasCachedData()) {
        LOG_INFO("✓ 缓存系统正常");
    } else {
        LOG_INFO("! 无缓存数据");
    }
    
    // 3. 测试内存检查
    bool memOk = memoryChecker.checkMemoryBeforeApiCall();
    LOG_INFO(String(memOk ? "✓" : "✗") + " 内存检查: " + String(ESP.getFreeHeap()) + "B");
    
    // 4. 测试NVS持久化
    testNVSPersistence();
    
    LOG_INFO("快速验证完成");
}
```

## 故障排除检查清单

### 🎯 -258错误专项排查

**当出现-258错误时：**
1. ✓ 检查API调用前可用内存（应≥60KB）
2. ✓ 检查响应大小（应≤25KB）
3. ✓ 验证HTTP超时设置（应为60秒）
4. ✓ 查看内存碎片化程度（应<70%）
5. ✓ 检查连续错误计数（≥3次触发重启）
6. ✓ 验证NVS错误状态持久化
7. ✓ 确认深度睡眠周期正常

**当连续出现-258错误时：**
1. ✓ 验证错误计数是否正确递增
2. ✓ 检查软重启触发条件（3次连续错误）
3. ✓ 确认重启间隔限制（30分钟）
4. ✓ 验证重启后错误计数重置
5. ✓ 检查缓存回退是否正常工作

### 📊 系统状态监控

**关键指标：**
- **-258错误率**：应<5%
- **连续错误恢复率**：应>95%
- **缓存命中率**：应>80%
- **软重启成功率**：应>95%
- **内存碎片化**：应<70%

**监控输出示例：**
```
=== -258错误解决方案状态 ===
运行时间: 2小时34分钟
-258错误次数: 0/3 (连续)
最后成功API: 15分钟前
当前数据源: 缓存 (年龄: 15分钟)
可用内存: 65,432字节 (充足)
内存碎片化: 23% (正常)
软重启次数: 1 (本次启动)

=== 最近事件 ===
[14:30] -258错误，使用缓存 (1/3)
[14:15] API成功，重置错误计数
[14:00] API成功
[13:45] 连续3次-258错误，软重启
[13:45] 软重启完成，内存已清理
```

### 🔧 快速诊断命令

**内存状态检查：**
```cpp
void quickDiagnosis() {
    LOG_INFO("=== 快速诊断 ===");
    LOG_INFO("可用内存: " + String(ESP.getFreeHeap()) + "B");
    LOG_INFO("最大分配: " + String(ESP.getMaxAllocHeap()) + "B");
    
    float frag = 1.0 - (float)ESP.getMaxAllocHeap() / ESP.getFreeHeap();
    LOG_INFO("内存碎片化: " + String(frag * 100, 1) + "%");
    
    errorManager.debugErrorState();
    cache.debugCacheState();
}
```

**问题解决优先级：**
1. **立即处理**：连续3次-258错误 → 检查软重启
2. **高优先级**：内存<60KB → 检查内存清理
3. **中优先级**：缓存失效 → 检查NVS存储
4. **低优先级**：网络错误 → 检查WiFi连接