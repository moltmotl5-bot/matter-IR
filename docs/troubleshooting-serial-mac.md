# Mac 序列埠監控無輸出 — 排查指南

ESP32-S3 開發板通常有 **兩個 USB-C 孔**，這是 Mac 上看不到 Serial Monitor 輸出的頭號原因。

## 快速診斷（5 分鐘）

### 步驟 1：先燒錄診斷韌體

```
firmware/serial_test/serial_test.ino
```

這個程式每秒印一行，不涉及 IR 函式庫，專門用來確認序列埠是否正常。

### 步驟 2：確認 Mac 有看到裝置

終端機執行：

```bash
ls /dev/cu.*
```

常見名稱：

| 裝置名稱 | 代表 |
|----------|------|
| `/dev/cu.usbmodem*` | 原生 USB（CDC） |
| `/dev/cu.wchusbserial*` 或 `/dev/cu.usbserial*` | CH343 UART 轉接 |

若完全沒有新裝置 → 見下方「驅動與線材」。

### 步驟 3：USB 孔 + CDC 設定必須配對

ESP32-S3 N16R8 雙孔開發板：

```
┌─────────────────────────────┐
│  [USB]          [COM/UART] │  ← 兩個 Type-C
│                             │
│         ESP32-S3            │
└─────────────────────────────┘
```

| 你插的孔 | Arduino IDE 設定 | 說明 |
|----------|------------------|------|
| **USB**（原生） | USB CDC On Boot = **Enabled** | Serial 走 USB |
| **COM / UART** | USB CDC On Boot = **Disabled** | Serial 走 CH343 |

**改設定後必須重新上傳韌體**，只改設定不重燒是不夠的。

### 步驟 4：Serial Monitor 設定

- Baud rate：**115200**
- Line ending：Newline 或 Both NL & CR
- 上傳完成後按一下板子 **EN / RST** 鍵
- 再開啟 Serial Monitor（或關掉重開）

### 步驟 5：兩套組合都試

若 Option A 無輸出，改 Option B 後 **重新上傳** 再試：

**Option A**
1. 線插 **USB** 孔
2. USB CDC On Boot = Enabled
3. 上傳 `serial_test.ino`
4. 選 `/dev/cu.usbmodem*` 埠
5. 開 Serial Monitor

**Option B**
1. 線插 **COM/UART** 孔
2. USB CDC On Boot = Disabled
3. 上傳 `serial_test.ino`
4. 選 `/dev/cu.wchusbserial*` 或 `usbserial*` 埠
5. 開 Serial Monitor

---

## 驅動與線材

### CH343 驅動（COM/UART 孔需要）

1. 下載：https://www.wch.cn/downloads/CH343SER_MAC_ZIP.html
2. 安裝後 **登出再登入** 或重開機
3. 重新插拔 USB-C

Apple Silicon Mac 若被安全性阻擋：系統設定 → 隱私權與安全性 → 允許 WCH 驅動。

### USB-C 線必须是「資料線」

純充電線無法傳資料。換一條確認可傳資料的 Type-C 線。

---

## 常見錯誤

| 現象 | 原因 | 解法 |
|------|------|------|
| 上傳成功但完全沒字 | 孔與 CDC 設定不配 | 依上表配對後重燒 |
| IDE 找不到埠 | 缺 CH343 驅動 | 安裝驅動 |
| 上傳用 A 埠、監控用 B 埠 | 埠不一致 | 上傳與監控用同一埠 |
| 只有上傳時有乱码 | Baud 不對 | 設 115200 |
| Phase 1 只有 heartbeat | 正常，尚未按遙控器 | 對準接收模組按鍵 |
| 板子不断重启 | 接線短路或 5V/GND 反接 | 断电检查接线 |

---

## 仍不行？

请提供以下信息：

1. `ls /dev/cu.*` 完整输出
2. Arduino IDE 里看到的 Port 名称
3. 你插的是 USB 还是 COM/UART 孔
4. USB CDC On Boot 当前设置
5. 上传是否显示成功（Done uploading）
