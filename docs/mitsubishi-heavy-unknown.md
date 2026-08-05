# Mitsubishi Heavy SRK53MMH1 — UNKNOWN 協定說明

## 你的测试结果分析

| 按键 | Value | Bits | 说明 |
|------|-------|------|------|
| Power ON | 0xA6D40490 | 130 | 8-byte 变体，库无法识别 |
| Temp UP | 0x4176CD50 | 130 | 与 Power ON 不同，捕获有效 |
| Temp DOWN | 0xA6D40490 | 130 | 可能与 Power ON 同帧或重复码 |
| Power OFF | 0x79B7EB22 | 260 | 可能是双帧/三连发 |
| Power OFF (2nd) | 0xC2FFAE7A | 130 | 单帧版本 |

**结论：序列埠正常，IR 接收正常。** `UNKNOWN` 不代表失败——表示 IRremoteESP8266 没有内建此遥控器的解码器。

SRK53MMH1 使用的是 **undocumented 8-byte（约 64-bit payload）变体**，不是库里的 `MitsubishiHeavy152` 或 `MitsubishiHeavy88`。

参考：[Hackaday — Hacking Mitsubishi Heavy AC IR](https://hackaday.io/project/205909-hacking-mitsubishi-heavy-ac-ir-with-esp32)

## 两条路径（按顺序试）

### 路径 A：函式库发射（先试，最简单）

烧录 `firmware/phase2_ir_transmit/phase2_ir_transmit.ino`，将发射模块对准冷气。

- 若冷气有反应 → 库的发射端兼容你的型号，可直接用 `IRMitsubishiHeavy152Ac` 继续 Matter 整合
- 若无反应 → 走路径 B

也可试 `phase2_ir_transmit_interactive`，用 Serial 命令 `on` / `off` / `temp 24` 测试。

### 路径 B：Raw 重播（UNKNOWN 时的可靠方案）

1. **重新烧录** 更新后的 `phase1_ir_learn.ino`（会输出完整 raw 时序数组）
2. 按 **Power ON**，从 Serial Monitor 复制 `--- Raw send array ---` 整块
3. 粘贴到 `phase2_raw_replay.ino` 的 `POWER_ON_RAW[]` 和 `POWER_ON_LEN`
4. 对 Power OFF、Temp UP/DOWN 重复
5. 烧录 `phase2_raw_replay.ino`，Serial 输入 `1` 测试开机

**注意：**

- Power OFF 若出现 260 bits（双帧），优先用 **130 bits 的单帧** 版本
- 每次按键后等输出完成再按下一个
- 遥控器距离接收模块 5–10 cm，避免过近饱和

## 重要：UNKNOWN 的 hex 值不能用来发射

`0xA6D40490` 这类 Value 是库对 raw 的错误解读，**不能**用 `send()` 重发。必须用 Phase 1 输出的 **raw timing 数组**（微秒 mark/space 序列）。

## 下一步

1. 先试 **路径 A**（phase2_ir_transmit）
2. 把结果告诉我（冷气有没有反应）
3. 若无反应，跑更新版 Phase 1，把 **Power ON 的 Raw send array** 贴给我，我帮你填好 phase2_raw_replay
