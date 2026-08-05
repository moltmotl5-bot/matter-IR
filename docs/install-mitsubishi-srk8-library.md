# 安裝 MitsubishiSRK8 函式庫

Arduino IDE 需先安裝此自訂函式庫才能編譯 `phase1_decode` 和 `phase2_srk_control`。

## Mac 一鍵安裝

```bash
cp -r firmware/libraries/MitsubishiSRK8 ~/Documents/Arduino/libraries/
```

若 sketchbook 路徑不同，在 Arduino IDE：**Arduino IDE → Settings → Sketchbook location** 查看實際路徑。

安裝後 **完全退出並重開 Arduino IDE**。

## 驗證

開啟 `phase1_decode.ino`，**Sketch → Include Library** 選單中應出現 **MitsubishiSRK8**。

## 依賴

- IRremoteESP8266（crankyoldgit）— 透過 Library Manager 安裝
