# 第二轮增量扫描：《时之笛》音频、存档、补丁与 Mod 工具链分析报告

> **工单编号**：`emrpg/gta6-oot-rea-reverse-engineering#8`  
> **扫描范围**：整合 `/root/wsl-demo/oot-peripherals/round2-audio-save-modtools/`（`RomPatcher.js`、`zzrtl`）及本地 `z64-sample-creation-tools/z64audio.exe`、`z64-bank-converter`、`oot-save-edit` 与 `OoTSaveEditor`，利用 `/usr/local/bin/rea` 完成静态取证与接口还原。

---

## 1. 第二轮增量工具清单与技术矩阵

| 工具 / 仓库 | 本地路径 | 技术栈 | 核心功能 |
| :--- | :--- | :--- | :--- |
| `z64audio.exe` | `oot-peripherals/z64-sample-creation-tools/z64audio.exe` | C (PE32 x86), OpenGL/Win32 GUI+CLI | N64 VADPCM 音频采样编码、Codebook 预测器生成与循环点处理 |
| `z64-bank-converter` | `oot-peripherals/z64-bank-converter` | Python 3 | OoT/MM 音色库（Audiobank）二进制与 XML/YAML 双向转换 |
| `RomPatcher.js` | `round2-audio-save-modtools/patch-otr/RomPatcher.js` | Vanilla JS (Web + Node) | 浏览器/Node 端全格式 ROM 补丁器（IPS、BPS、UPS、APS、PPF、VCDiff） |
| `zzrtl` | `round2-audio-save-modtools/audio-model/zzrtl` | C (`n64crc`/`n64texconv`/`dma__compress`) | 脚本驱动的 OoT/MM ROM 文件系统解包、注入与 Yaz0 重压缩工具 |
| `oot-save-edit` | `oot-peripherals/oot-save-edit` | TypeScript / Deno | N64 `.sra`/`.srm` 存档解析与字序转换编辑器（支持 `pj64tosrm` 兼容） |
| `OoTSaveEditor` | `oot-peripherals/OoTSaveEditor` | C# (.NET WinForms) | 经典 OoT 存档编辑器（覆盖物品、事件/场景永久旗标与心之碎片） |

---

## 2. REA 静态取证关键发现 (`rea-reconstructions/round2-oot-modtools/evidence/`)

1. **`z64audio.exe`（`sha256: 3d173b43b807`）**：
   - `inspect-binary-hardening` 实测为 **32 位 PE32 (`x86`)**，已开启 `dynamic_base`（ASLR）与 `nx_compat`（DEP），未启用 `guard_cf` 与 Authenticode 签名。
   - `inspect-pe-imports` 实测导入 **6 个 DLL 共 270 个函数**：`KERNEL32.dll(83)`、`msvcrt.dll(101)`、`USER32.dll(69)`、`GDI32.dll(12)`、`SHELL32.dll(4)`（支持文件拖拽）、`OPENGL32.DLL(1)`。
   - 只读字符串观测到 `z64audio.toml` 配置文件驱动的 `[bin/wav/aiff/c]` 多格式输出，以及 VADPCM Codebook 参数 `book_order`、`book_bits`、`book_frame_size`、`book_iteration`、`book_threshold`。
2. **`RomPatcher.js` 与 `oot-save-edit` JS 图谱取证**：
   - `rea analyze-javascript-application` 确认两者均为纯标准前端/CLI 模块结构，无 Electron `BrowserWindow` 或 IPC 暴露面。

---

## 3. 接口与核心算法还原 (`/root/wsl-demo/rea-reconstructions/round2-oot-modtools/`)

1. **`z64audio_vadpcm_reconstructed.c`**：还原 `z64_vadpcm_config_t` 配置结构与 9 字节 N64 VADPCM 帧编码器（`scale_shift` + `predictor_index` + 16×4-bit 残差 Nibbles）。
2. **`rompatcher_bps_ips_reconstructed.ts`**：还原 `BPS1` 变长整数解码（`decodeVarInt`）与四大动作指令（`SourceRead` / `TargetRead` / `SourceCopy` / `TargetCopy`）。
3. **`oot_sram_and_zzrtl_reconstructed.ts`**：还原 N64 32KB SRAM 4 字节端序交换（`swap32InPlace`）、`0x1352` 字节槽位 16-bit 累加校验和（`computeOotSlotChecksum`）及 `zzrtl` `dmadata` 16 字节表项解析。
