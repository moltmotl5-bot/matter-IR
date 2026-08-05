# Mitsubishi SRK 8-byte 協定解碼

## 為什麼可以直接解碼？

你的 SRK53MMH1 遙控器使用的是 **8-byte 變体**，不是 IRremoteESP8266 內建的 152/88 bit 格式，但社群已完成逆向：

| 欄位 | 意義 |
|------|------|
| B0 B1 | 固定標頭 `FF 00` |
| B2 B3 | 風速 + 校驗（B3 = 0xFF − B2） |
| B4 B5 | 溫度+模式 + 校驗（B5 = 0xFF − B4） |
| B6 B7 | 固定尾碼 `2A D5` |

**B4 編碼：**
- 高 4 bit：`(32 − 溫度) & 0xF`
- 低 4 bit：模式（6=冷氣, 5=除濕, 4=送風, 3=暖氣, 7=自動）
- 關機：低 4 bit OR `0x08`

**風速 B2：** `FF`=高, `BF`=中, `9F`=低

參考：[Hackaday — Hacking Mitsubishi Heavy AC IR](https://hackaday.io/project/205909-hacking-mitsubishi-heavy-ac-ir-with-esp32)

## 使用方式

### 1. 安裝自訂函式庫

見 [install-mitsubishi-srk8-library.md](install-mitsubishi-srk8-library.md)

### 2. 解碼測試（Phase 1）

燒錄 `firmware/phase1_decode/phase1_decode.ino`，按遙控器按鍵。

**成功輸出範例：**

```
--- Custom decode ---
Frame: FF 00 BF 40 66 99 2A D5
Validation: OK
Power : ON
Mode  : cool
Temp  : 24 C
Fan   : medium
```

### 3. 發射控制（Phase 2）

燒錄 `firmware/phase2_srk_control/phase2_srk_control.ino`，Serial 輸入 `on`、`temp 24`、`off` 等命令。

## 與 Matter 整合

解碼後的 `MitsubishiSRKState` 可直接對接 Matter Thermostat callback，比 raw 重播更適合長期使用。
