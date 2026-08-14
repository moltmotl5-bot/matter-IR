# Phase 3 — Matter + Panasonic CW-HU 設定

## 適用機型

- **Panasonic CW-HUxxxx** 窗型冷氣
- IR 協定：`PANASONIC_AC`，model **NKE**（Phase 1/2 已確認）

## 前置條件

- Phase 1 / 2 已成功
- ESP32-S3 **N16R8**
- esp32 Arduino core **3.0.0+**
- Apple Home 中已有 Matter hub（HomePod / Apple TV）

## Arduino IDE 設定

| 選項 | 值 |
|------|-----|
| Board | ESP32S3 Dev Module |
| Flash Size | 16MB |
| PSRAM | OPI PSRAM |
| **Partition Scheme** | **16M Flash (3MB APP/9.9MB FATFS)** |
| USB CDC On Boot | 依你的 COM 埠設定 |

## 函式庫

| 函式庫 | 用途 |
|--------|------|
| IRremoteESP8266 | Panasonic IR 發射 |
| Adafruit AHTX0 | AHT20 室溫（可選） |

## WiFi 設定

```bash
cp firmware/phase3_matter_ac/secrets.h.example firmware/phase3_matter_ac/secrets.h
```

編輯 `secrets.h`：

```cpp
#define WIFI_SSID "你的WiFi名稱"
#define WIFI_PASSWORD "你的WiFi密碼"
```

## 接線

| 模組 | ESP32-S3 |
|------|----------|
| IR 發射 DAT | GPIO 15（VCC→5V） |
| AHT20 SDA | GPIO 8 |
| AHT20 SCL | GPIO 9 |

若尚未接 AHT20，在 `config.h` 註解：

```cpp
// #define USE_AHT20_SENSOR 1
```

## 烧录与配对

1. 上传 `firmware/phase3_matter_ac/phase3_matter_ac.ino`
2. Serial Monitor 115200
3. 复制 **Manual code** 或打开 **QR URL**
4. iPhone：**家庭 App → 加入配件 → Matter 配件**
5. 配对成功后，在 Home 中应看到**温控器**

## Home App 操作 → IR

| Home 操作 | 冷气 IR |
|-----------|---------|
| 关闭 | Power OFF |
| 制冷 + 温度 | Cool + 设定温度 |
| 除湿 | Dry |
| 仅送风 | Fan only |
| 制热 | Heat（若 CW-HU 支持） |

## 解除配对

按住开发板 **BOOT** 键 **5 秒** → Serial 显示 Decommissioning → 重新配对

## 常见问题

**编译失败 Matter / 容量不足**
→ 确认 Partition Scheme 为 3MB+ APP

**Home 有温控但冷气没反应**
→ 确认 IR 发射模块 5V、对准 CW-HU；Serial 应出现 `[IR] TX sent`

**没有室温**
→ 接 AHT20 并启用 `USE_AHT20_SENSOR`，或 Home 只显示目标温度

**IRremoteESP8266 与 Matter 编译冲突**
→ Phase 3 仅 IR 发射、不用 IR 接收，一般可共存；若报错请升级 IRremoteESP8266 至含 ESP32 3.x 补丁的版本
