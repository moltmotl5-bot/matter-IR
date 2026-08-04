# 接線指南

## 腳位對照

| 模組 | 腳位 | ESP32-S3 |
|------|------|----------|
| IR 接收 (HX1838) | VCC (+) | 3V3 |
| IR 接收 (HX1838) | GND (-) | GND |
| IR 接收 (HX1838) | DAT (S) | **GPIO 4** |
| IR 發射模組 | VCC | **5V / VIN** |
| IR 發射模組 | GND | GND |
| IR 發射模組 | DAT | **GPIO 15** |
| AHT20+BMP280 | SDA | GPIO 8 |
| AHT20+BMP280 | SCL | GPIO 9 |
| AHT20+BMP280 | VCC | 3V3 |
| AHT20+BMP280 | GND | GND |

## 接線示意

```
                    ESP32-S3 N16R8
                 ┌─────────────────┐
    IR Receiver  │  3V3 ─── VCC    │
    (HX1838)     │  GND ─── GND    │
                 │  GPIO4 ─ DAT    │
                 │                 │
    IR TX Module │  5V  ─── VCC    │
                 │  GND ─── GND    │
                 │  GPIO15─ DAT    │
                 │                 │
    USB-C ───────┤  COM/UART       │──── Mac
                 └─────────────────┘
```

## 注意事項

1. **發射模組接 5V**：確保 3 公尺發射距離。
2. **發射模組有方向性**：用母對母杜邦線延長，方便對準冷氣。
3. **VCC 勿接 GND**：通電前確認正負極。
4. **建議線色**：紅=VCC、黑=GND、黃/藍=訊號。
