# ESP32-S3 Matter 智慧冷氣遙控器

基於 ESP32-S3 的紅外線發射/接收裝置，目標是透過 Matter 協議接入 Apple HomeKit，控制三菱重工 (Mitsubishi Heavy) 分體式冷氣 SRK53MMH1。

## 硬體清單

| 項目 | 規格 |
|------|------|
| ESP32-S3 開發板 | N16R8（16MB Flash + 8MB PSRAM） |
| 紅外線接收模組 | HX1838（帶金屬屏蔽罩） |
| 紅外線發射模組 | 帶電晶體放大 |
| 溫濕度模組 | AHT20 + BMP280（階段三起使用） |

接線詳情見 [docs/wiring.md](docs/wiring.md)。

## Mac 開發環境設定

### 1. 安裝 CH343 驅動

ESP32-S3 開發板使用 CH343 USB 轉串口晶片。若 Arduino IDE 找不到連接埠：

1. 下載 [WCH CH343 Mac 驅動](https://www.wch.cn/downloads/CH343SER_MAC_ZIP.html)
2. 安裝後重新插拔 USB-C 線
3. 在終端機確認：`ls /dev/cu.*` 應出現類似 `/dev/cu.usbmodem*` 的裝置

### 2. Arduino IDE 設定

**開發板管理員網址**（偏好設定 → 額外的開發板管理員網址）：

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

**安裝套件**（開發板管理員 → 搜尋 `esp32`）：

- `esp32` by Espressif Systems，版本 **3.0.0 或以上**（Matter 支援需要）

**函式庫**（程式庫管理員）：

| 函式庫 | 作者 | 用途 |
|--------|------|------|
| IRremoteESP8266 | crankyoldgit | 紅外線收發 |
| Adafruit AHTX0 | Adafruit | AHT20 溫濕度 |
| Adafruit BMP280 Library | Adafruit | BMP280 氣壓 |

**開發板選項**（工具選單）：

| 選項 | 值 |
|------|-----|
| Board | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| PSRAM | OPI PSRAM |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| USB CDC On Boot | Enabled（方便 Mac 序列埠監控） |

### 3. 上傳韌體

1. 用 USB-C **資料傳輸線**連接 Mac 與 ESP32 的 **COM/UART** 埠
2. 開啟對應階段的 `.ino` 檔案
3. 選擇正確的連接埠（`/dev/cu.usbmodem...`）
4. 點擊上傳
5. 開啟序列埠監控，Baud rate 設 **115200**

## 開發階段

### 階段一：訊號破解（你現在在這裡）

```
firmware/phase1_ir_learn/phase1_ir_learn.ino
```

1. 燒錄此韌體
2. 開啟序列埠監控（115200）
3. 拿三菱重工遙控器對準接收模組，按「電源」或「調溫」
4. **記下 Protocol 名稱**（預期 `MitsubishiHeavy152` 或 `MitsubishiHeavy88`）

### 階段二：紅外線發射測試

```
firmware/phase2_ir_transmit/phase2_ir_transmit.ino          — 自動每 10 秒發射
firmware/phase2_ir_transmit_interactive/                    — 序列埠互動控制
```

1. 將發射模組對準冷氣（約 3 公尺）
2. 若冷氣有反應，階段二完成
3. 若 Phase 1 顯示 `MitsubishiHeavy88`，修改 `.ino` 中的 AC 類別

### 階段三：Matter 溫控器整合（待開發）

- 使用 Arduino ESP32 Matter 範例建立 Thermostat 設備
- 整合 AHT20 讀取室溫
- Apple Home 配對

### 階段四：軟硬體結合部署（待開發）

- Matter callback 觸發 IR 發射
- 5V/1A USB 供電正式部署

## 專案結構

```
matter-IR/
├── README.md
├── docs/
│   └── wiring.md
└── firmware/
    ├── config.h                          — 共用腳位設定
    ├── phase1_ir_learn/                  — 階段一：訊號學習
    ├── phase2_ir_transmit/               — 階段二：自動發射測試
    └── phase2_ir_transmit_interactive/   — 階段二：互動發射測試
```

## 常見問題

**Arduino IDE 找不到連接埠**
→ 安裝 CH343 驅動，確認用的是 COM/UART 埠而非純 USB 埠

**序列埠監控沒有輸出**
→ 確認 Baud rate 為 115200；嘗試將 USB CDC On Boot 設為 Enabled

**編譯錯誤：找不到 IRremoteESP8266**
→ 在程式庫管理員安裝 IRremoteESP8266（作者 crankyoldgit）

**Phase 1 顯示 UNKNOWN**
→ 遙控器距離太遠或角度不對；確保接收模組 S 腳接 GPIO 4

**Phase 2 冷氣沒反應**
→ 確認發射模組 VCC 接 5V（非 3.3V）；調整發射角度對準冷氣 IR 接收窗

## 授權

Apache License 2.0
