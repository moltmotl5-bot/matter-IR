# ESP32-S3 Matter 智慧冷氣遙控器（Panasonic）

基於 ESP32-S3 的紅外線收發裝置，透過 **IRremoteESP8266** 學習與控制 **Panasonic** 冷氣，最終接入 Apple HomeKit（Matter）。

> 本專案已改為 Panasonic 路線；三菱重工相關韌體已移除。

## 硬體清單

| 項目 | 規格 |
|------|------|
| ESP32-S3 開發板 | N16R8（16MB Flash + 8MB PSRAM） |
| 紅外線接收模組 | HX1838 |
| 紅外線發射模組 | 帶電晶體放大（VCC 接 **5V**） |
| 溫濕度模組 | AHT20 + BMP280（階段三） |

接線見 [docs/wiring.md](docs/wiring.md)（GPIO 4 = 接收，GPIO 15 = 發射）。

## Mac 開發環境

1. **CH343 驅動**（COM/UART 孔）：[WCH 下載](https://www.wch.cn/downloads/CH343SER_MAC_ZIP.html)
2. **Arduino IDE**：esp32 **3.0+**，Board = ESP32S3 Dev Module
3. **函式庫**：IRremoteESP8266（crankyoldgit）
4. **Serial 問題**：见 [docs/troubleshooting-serial-mac.md](docs/troubleshooting-serial-mac.md)

| USB 孔 | USB CDC On Boot |
|--------|-----------------|
| USB（原生） | Enabled |
| COM/UART | Disabled |

## 開發階段

### 階段一：學習 Panasonic 遙控器

```
firmware/phase1_panasonic_learn/phase1_panasonic_learn.ino
```

1. 燒錄後開 Serial Monitor（115200）
2. 按 Panasonic 遙控器按鍵
3. 確認 Protocol 為 `PANASONIC_AC` 或 `PANASONIC_AC32`
4. 協定說明：[docs/panasonic-protocol.md](docs/panasonic-protocol.md)

### 階段二：發射控制

```
firmware/phase2_panasonic_control/phase2_panasonic_control.ino
```

Serial 命令：`on` · `off` · `temp 24` · `cool` · `model nke` · `help`

### 階段三：Matter + Apple Home

```
firmware/phase3_matter_ac/phase3_matter_ac.ino
```

1. 复制 `secrets.h.example` → `secrets.h`，填入 WiFi
2. Partition：**16M Flash (3MB APP/9.9MB FATFS)**
3. Serial 显示 QR / Manual code → Apple Home 配对
4. 详见 [docs/phase3-matter-setup.md](docs/phase3-matter-setup.md)

适用：**Panasonic CW-HUxxxx**，IR 默认 `PANASONIC_AC` + **NKE**

## 專案結構

```
matter-IR/
├── README.md
├── docs/
│   ├── wiring.md
│   ├── panasonic-protocol.md
│   ├── phase3-matter-setup.md
│   └── troubleshooting-serial-mac.md
└── firmware/
    ├── serial_test/
    ├── phase1_panasonic_learn/
    ├── phase2_panasonic_control/
    └── phase3_matter_ac/              — Matter + IR
```

## 常見問題

**Phase 1 顯示 UNKNOWN**
→ 仍會印 raw dump；試 Phase 2 並更換 `model` / `protocol ac32`

**Phase 2 冷氣沒反應**
→ 發射模組接 5V；對準 IR 接收窗；依 Phase 1 換 `model nke` 等

**Serial Monitor 無輸出**
→ 先燒錄 `firmware/serial_test/serial_test.ino`

## 授權

Apache License 2.0
