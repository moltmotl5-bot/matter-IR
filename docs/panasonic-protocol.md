# Panasonic 冷氣 IR 協定

## IRremoteESP8266 支援的 Panasonic 協定

| Protocol 名稱 | 常見機型 | Phase 2 用法 |
|---------------|----------|--------------|
| **PANASONIC_AC** | 大部分分體式、窗型（NKE/LKE/DKE/JKE 等） | 預設 `IRPanasonicAc` |
| **PANASONIC_AC32** | CS-E9CKP 等 CKP 系列 | Serial 輸入 `protocol ac32` |
| **UNKNOWN** | 較新或特殊機型 | 貼 raw dump；可能需 raw 重播或自訂解碼 |

Phase 1 會自動嘗試解碼並顯示 Power / Mode / Temp / Fan。

## 階段一：學習遙控器

燒錄 `firmware/phase1_panasonic_learn/phase1_panasonic_learn.ino`

1. 對準接收模組，按 **電源**
2. 再按 **調溫 +/-**
3. 記下 Serial 顯示的 **Protocol** 名稱

**成功範例：**

```
Protocol : PANASONIC_AC
--- Panasonic AC decode ---
Power : ON
Mode  : 3
Temp  : 24 C
Fan   : 5
Model : 1
```

## 階段二：發射測試

燒錄 `firmware/phase2_panasonic_control/phase2_panasonic_control.ino`

```
on
temp 24
cool
off
```

若冷氣無反應，依 Phase 1 結果調整：

```
model nke      # 或 lke / dke / jke / ckp
protocol ac32  # 若 Phase 1 是 PANASONIC_AC32
on
```

## Model 對照（IRremoteESP8266）

| Serial 命令 | 說明 |
|-------------|------|
| `model unknown` | 自動偵測（預設） |
| `model nke` | NKE 系列遙控 |
| `model lke` | LKE 系列 |
| `model dke` | DKE 系列 |
| `model jke` | JKE 系列 |
| `model ckp` | CKP 系列（常配合 ac32） |
| `model rkr` | RKR 系列 |

## 若仍是 UNKNOWN

Panasonic 窗型或 nanoe 等進階機種有時使用較長幀，IRremoteESP8266 可能無法自動命名協定，但 raw 資料仍有效。請把 Phase 1 的 **Raw send array** 貼上來，再決定要 raw 重播或進一步逆向。

參考：[Panasonic window AC + HASS vibe coding](https://it9gamelog.medium.com/panasonic-window-ac-hass-vibe-coding-4da44183fe1e)

## 與 Matter 整合（階段三）

`IRPanasonicAc` 提供完整 state API（`setTemp`、`setMode`、`send`），可直接接入 Matter Thermostat callback，無需自訂 byte 解碼。
