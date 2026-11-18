# 6. 本轮新增硬事实：校验和算法族、容器身份、补丁投递层

> 分级见 `README.md`。本文件只收录**本轮亲自复核过 `path:line`** 的增量结论，
> 用于修正 `01/02/03` 的早期表述，并作为报告 16 的直接依据。

---

## 6.1 N64 `.sra` 槽位布局：算术闭合（F）

两处**互不依赖**的实现给出同一布局：

| 组成 | 大小 | 证据 |
|---|---|---|
| `SaveHeader` | `0x20` | `oot-save-edit/models/saveheader.ts:24-26` |
| 3 × 存档槽 `SaveSlot` | `0x1450` each | `oot-save-edit/models/saveslot.ts:330-331`（`requiredSize`）、`:339`（构造断言） |
| 3 × 备份槽 | `0x1450` each | `oot-save-edit/models/savefile.ts:111-118`（先 slots 后 backups 两轮 `currentOffset += SaveSlot.requiredSize`） |
| 尾部填充 | `0x600` | `OOT-Save-Converter/oot.js:44-51`（`0x1450 ×6` 后 `0x600 // Junk`） |

`0x20 + 6 × 0x1450 + 0x600 = 0x8000 = 32 768 B` ⇒ 与 §1.1 的 SRAM 256 Kbit 完全对齐（F）。
注意 C# 侧 `ZeldaSaveTool/Structs/Oot/Oot.cs:135-145` 的 `Save` 结构只到 `0x1354` 级别（单槽），
说明**"槽 0x1450" 是 oot-save-edit/OOT-Save-Converter 口径（含 `0x0E` 之后的额外字段），
而 ZeldaSaveTool 口径是"单份 SaveInfo"**：两者不矛盾，但混用会算错偏移（?**→F**，见下）。

`SaveSlot` 相对偏移速查（F）：

| 字段 | 槽内偏移 | 证据 |
|---|---|---|
| `checkPattern`（6 B） | `0x001C` | `saveslot.ts:410-412` |
| 合法值 | `"ZELDAZ"` | `saveslot.ts:334-336`（`TextEncoder` 编码）、`:1109-1114`（`isValid = isPatternValid && isChecksumValid`） |
| `eventFlags` | `0x0ED4`，28 B（u16 数组） | `saveslot.ts:1045-1052` |
| `itemFlags` | `0x0EF0`，8 B | `:1054-1061` |
| `otherFlags` | `0x0EF8`，60 B | `:1063-1070` |
| `dungeonItems` | `0x00A8`，`0x14` B | `:830` |
| `checksum` | `0x1352`，2 B | `:1072-1078` |
| `fileIndex` | `0x1354`，4 B | `:1080-1086` |

⇒ **实卡侧的 `ZELDAZ` 标记（§1.4 DreamDumper64）与软件侧的槽内模式是同一常量的两处出现**（F），
这解释了为什么 `n64-sram-converter` 与 `oot-save-edit` 都能用同一个 6 字节串判定有效性。
`Admentus64-N64-Save-Converter/"N64 Save Converter.ps1":415-425` 给出 MM/SW 系列的同族常量
`ZELDA2`=`5A 45 4C 44 41 32`、`ZELDA3`、以及 OoT 分支的 `21 5A 45 4C 44 41`（前导 `0x21`）与
抽取长度 `0x8000`/偏移 `-6`（F，同一模式家族的变体）。

---

## 6.2 校验和算法族对照（F，本轮新增的关键判据）

| 目标 | 步长 | 覆盖长度 | 归约 | 写回 | 证据 |
|---|---|---|---|---|---|
| **N64 OoT 槽（`.sra`）** | 16-bit 大端字 | `0x09A9` 字 = **`0x1352` B**（自槽首 `0x0000`） | `sum %= 0xFFFF`（**模 65535**） | u16 BE @ `0x1352` | `oot-save-edit/models/saveslot.ts:1088-1096` |
| GC/Wii 卡 OoT 槽（segher） | 字节 | `0x0a8c` B | `nsum = -(sum + 0x0a8c)` | **两个 BE32** @ `0x0a8c`/`0x0a90` | `segher-wii-tools/zelda-cksum.c:11-22` |
| BizHawk/PC 端口整包 | 16-bit 大端字 | 至 `0x1ffc` | — | u16 @ `0x1ffc`/`0x1ffe` | `zelda-cksum.c:34-40` |
| MM FlashRAM 槽 | 字节和，**排除校验字段本身** | — | 16-bit 回写 | `zs-flash/zs_data.c:825-848` |
| MM3D（3DS） | CRC-16/ARC，种子 `0x1017` | — | @ `0x1A88` | `mm3d-heart-tracker/mm3d_save.py:35-41,58`；`mm3d-save-editor/checksum/crc16.go:10-12` |
| Switch 2  remake `progress.sav` | **无校验和**，字段名 = `murmur3_32` | 容器 `MAGIC 0x01020304`、`VERSION 0x002D80EB`、头 40 B、项 8 B | — | `oot-sw2-save/ootsw2/container.py:41-44`、`hash.py:30,68` |

**最容易踩的坑（F）**：`sum % 0xFFFF` 与 `sum & 0xFFFF` **不等价**（前者模 65535，后者模 65536）。
N64 OoT 用的是 `%= 0xFFFF`。任何以 `& 0xffff` 实现的"OoT 校验和"在跨边界时会产生
每累积一次溢出即偏差 1 的系统性错误 ⇒ 存档被游戏判为损坏。见 §6.6 的并发产出核查。

另一坑：`0x1352` **既是校验和的偏移量，又恰好等于覆盖长度**（`0x09A9 × 2 = 0x1352`）——巧合，
不是同一含义；把它当"字段所在偏移"去 `slice(0x1352, 0x1352+len)` 会读到校验和本身。

---

## 6.3 DexDrive 头的两种长度写法（F，纠正 §1.1 表述）

- `save-file-converter/frontend/src/save-formats/N64/DexDrive.js:24` `HEADER_LENGTH = 4160`、`:26` `HEADER_MAGIC = '123-456-STD'`（**11 字符**）；`:6` 注释同样写 "4160 byte header"。
- `mempak/js/parser.js:630` 判定用 `arrstr(data, 0, 12) === "123-456-STD\x00"`（**12 字节含 NUL**）。
⇒ 同一魔数的两种读取宽度并存：11 B 串 + 1 B NUL（F）。§1.1 里"64 + 16×256 = 4160"的分解仍成立（`4160 = 0x1040`），
但**不要**据此认为头结构只有 64 字节注释区；两份实现对头内其余字段的处理不同：
`save-file-converter` 把 `4160` 之后的整体当 mempack（`:103`），`mempak` 则按 bank 解析。

## 6.4 容量/后缀矩阵的第二个独立来源（F）

`Ninjiteu/N64SaveConverter`（Java，GUI 版）`N64SaveConverterGui/src/N64SaveConverter.java:42-49`：

```
SIZE_EEP = 2048   SIZE_SRA = 32768   SIZE_FLA = 131072   SIZE_MPK = 131072   SIZE_SRM = 296960
SIZE_SRA_SRM_OFFSET = 133120   SIZE_MPK_SRM_OFFSET = 2048
```

- `296960 = 32 × 9280`？实为 mupen64plus/Retropal 的**整卡镜像**：`296960 − 32768×8 = 32768`??
  核对：`296960 = 0x48800`（与 §1.2 BizHawk `.SaveRAM` **同一大小**，F）⇒ `SIZE_SRM` 即 8 个 32 KiB 槽 + `0x800` 头尾的 mupen 卡；
  `SIZE_SRA_SRM_OFFSET = 133120 = 0x20800` **与 §1.2 "save 落在 0x20800" 完全一致**（F，两条独立路径互证）。
- `SIZE_MPK_SRM_OFFSET = 2048 = 0x800` 与 §1.2 "2 KiB 零头" 一致（F）。
⇒ §1.2 的 BizHawk 布局推断升级为**双源印证**，且 `0x800 / 0x20800` 两个魔数偏移不再是"从 C 代码读出"而是"生态共识"。

`ChronoAndross/n64SaveConverter`（Android）`N64SaveConverter.java:19-31` 给同一矩阵 +
`REGEX` 覆盖 `eep|sra|fla|mpk|srm`，`:65-70` `.mpk → 前 pad 2048 的 .mpk 与 .srm`，`:157` `byteSwap`（F）。

---

## 6.5 N64 VADPCM 帧头与 codebook：位约定（F，用于裁定"还原"的合法性）

一手解码实现 `ocarina-songbook/ocarina_songbook/oot_soundfont.py`：

- `:199` `scale = 1 << (fr[0] >> 4)` ⇒ **帧头高 4 位 = shift/scale 指数**；
- `:204` order-2：`ix += [c >> 4, c & 15]` ⇒ **每字节高 nibble 在前**，4-bit 有符号残差；
- `:208` order-4：`ix += [(c >> 6) & 3, (c >> 4) & 3, (c >> 2) & 3, c & 3]` ⇒ **2-bit 样/字节**，且 `:205,209` 的
  `(v - 16 if v >= 8 else v)` / `(v - 4 if v >= 2 else v)` 给出**符号偏置**（order-2 阈值 8，order-4 阈值 2）；
- `:14` 文档化 sample 头字段序：`loop count u32 (0 = 无循环, 0xFFFFFFFF = 永久)`、`n u32 + n s16` 循环 predictor、
  `book order s32`、`npredictors s32`、`n u32 + n s16` book；
- `:164` 注释："The decomp's `expand_codebook`: per predictor an **8 × (order + 8)**" 矩阵 ⇒ codebook 展开后的形状是
  `8 × (order+8)`（order=2 → `8 × 10`），**不是** `2 × 8`。

结构体侧（F）：`z64-bank-converter/utils/audiobank/structs/Codebook.py:44`
`AdpcmBook: struct size = 0x8 + (0x08 * order * num_predictors)`、`:53` `self.order = 2`、`:68/:72`
写出 order 后 `assert self.order == 2`（**该工具只支持 order 2**）、`:169` `align_to_16(8 + 8*order*npredictors)`。

⇒ 结论：任何"N64 VADPCM 编码器"接口若要正确，帧头必须是 `(shift << 4) | filter`、nibble 高位在前、
且 order 4 要走 2-bit 分支；`book_order`/`book_bits` 二词在 `z64audio.exe` 里确实存在（见 §6.7），
`book_bits` 语义即 `npredictors = 1 << book_bits`（R，未从二进制反汇编证实）。

## 6.6 并发提交 `7f05074` 的三份"还原"文件核查（逐条）

`git show --stat 7f05074` = 3 个 `*_reconstructed.*` + `reports/16`（227 行插入），**已 push 至 PUBLIC origin/main**。

| 断言/代码 | 判定 | 依据 |
|---|---|---|
| 文件名 `z64audio_vadpcm_reconstructed.c`，证据仅列 REA JSON 与字符串 | **?（命名越界）** | 未反汇编即给出函数体，属"规格推导"而非"从工件还原"；应标 `spec-derived reference` |
| `header = (scale_shift << 4) \| pred_idx` | **F（正确）** | `oot_soundfont.py:199` |
| 偶数 i 写高 nibble | **F（order-2 正确）** | `oot_soundfont.py:204` |
| `codebook_row[2][8]` | **错误（应为 `8 × (order+8)`）** | `oot_soundfont.py:164` |
| `static_cast<int32_t>(…)` 出现在 `.c` | **不能编译** | C++ 语法 |
| 固定 16 × 4-bit 残差，同时配置里允许 `book_order` | **不自洽** | order-4 为 2-bit（`:208`） |
| `sum = (sum + word) & 0xffff` | **错误**（应为 `% 0xFFFF`） | `saveslot.ts:1093` |
| `OOT_SLOT_SIZE = 0x1450` | **F** | `saveslot.ts:331`、`oot.js:44-50` |
| "checksum covers 0x1352 bytes" | **数值正确、措辞混淆** | `0x09A9×2 = 0x1352`；同一数又是 `checksum` 字段偏移 |
| 证据路径 `oot-save-edit/models/savefile.ts` | **该文件不含这些常量** | 实际为 `models/saveslot.ts` / `models/saveheader.ts` / `models/savefile.ts`（布局在 `:104-118`） |
| `rompatcher_bps_ips_reconstructed.ts` 标题含 "IPS32/IPS Patcher" | **文件内无一行 IPS 代码** | 全文只有 `BPS1` |
| `decodeVarInt` VLV 实现 | **F** | 与 `RomPatcher.format.bps.js` 的 `BPS_readVLV` 一致（见 §2.3） |
| `actionEnd = patch.length - 12`、`'BPS1'` 头校验 | **F** | 同 §2.3 页脚三 CRC 结论 |
| 未做 source/target/patch 三重 CRC 校验 | **不完整**（真实实现会抛 mismatch） | §2.3 |
| `parseDmaEntry` 4 × u32 BE | **F** | zzrtl/OoT 文件表口径（§3.3） |
| `z64audio.exe` hardening：`dynamic_base`/`nx_compat` enabled、`guard_cf` disabled、无证书、无 debug 目录 | **F** | `evidence/z64audio_exe__inspect-binary-hardening.json`（machine 332、2 757 038 B、sha `3d173b43…65bf`） |
| 导入表 "6 DLL / 270 函数" | **F** | `evidence/z64audio_exe__inspect-pe-imports.json` |
| `z64audio.toml`、`book_order/bits/frame_size/iteration/threshold` 字符串 | **F** | 直接对 `z64audio.exe` `grep -a`，每个词命中 ≥1 行 |
| "RomPatcher.js 与 oot-save-edit 均无 Electron BrowserWindow / IPC 暴露面" | **部分成立** | `oot-save-edit` 有 `ui/main.ts` **webview** 桌面壳（其 `AGENTS.md` 自述"Desktop GUI wrapper: ui/main.ts (webview + bundled web UI)"）⇒ 非 Electron，但**确有一个原生 WebView 宿主**，报告不得写成"无桌面宿主" |

⇒ 处置：不改写他人提交内容（已在 PUBLIC 历史中），改在本节留痕并在报告 16 §7 明确"公开仓库历史中存在
三处技术错误（`& 0xffff`、`2×8` codebook、C++ 语法置于 `.c`）与一处命名越界"，并给出修正实现（§6.2/§6.5）。

## 6.7 `z64audio.exe` 字符串面（F）

对 `oot-peripherals/z64-sample-creation-tools/z64audio.exe` 做 `grep -a` 词频（只读）：
`z64audio.toml`×1、`book_order`×1、`book_bits`×1、`book_frame_size`×1、`book_iteration`×1、
`book_threshold`×1、`vadpcm`×2、`loopbook`×1、`.book.bin`×1。
⇒ 与 §3.2 的 `-i/-o` + TOML 自解析 + `.book.bin/.loopbook.bin/.vadpcm.bin` 输出互证；
`book_iteration`/`book_threshold` 的存在**暗示编码器做多次迭代/阈值搜索**（而非单次 `err/scale` 贪心）——
这是 §6.6 第 5 行的关键旁证（R）。

## 6.8 OTR / O2R 容器身份（F，升级 §2.4）

`OTRMod/OTRMod/OTR/Generate.cs`：`:21` 注释 `/* OTR (MPQ-based) */`、`:22-33` `FromImage` → `MpqArchiveBuilder` →
`builder.SaveTo(otrStream, true)`；`:35` `/* O2R (ZIP-based) */`、`:36-40` `FromImageO2R` → Ionic `ZipFile` +
`CompressionLevel.BestCompression` + **`date = 1970-01-01 UTC`（epoch 时间戳，为可复现构建）**。
⇒ **OTR = MPQ 容器，O2R = ZIP 容器**（F），此前的 U 解除。
`OTRExporter/OTRExporter/ExporterArchiveO2R.cpp:16,23,28`（"Failed to open ZIP (O2R) file" / "Loaded ZIP (O2R) archive"）
为 C++ 侧**独立第二源**印证 O2R=ZIP（F）；`ExporterArchiveOTR.cpp:35` "No valid OTR file was provided."。
`OTRMod.Web/OTRMod.Web.csproj:1` `Microsoft.NET.Sdk.BlazorWebAssembly`、`:12` `net10.0`、`:20` `RunAOTCompilation=True`
⇒ 浏览器端 OTR 构建走 **Blazor WASM + AOT**；`Services/Archive/ArchiveExplorer.cs:129` `MpqArchive.Open(stream, true)`。
WASM/AOT 段级还原仍需 WABT（U，见 §5.4）。

## 6.9 OoTR 的两层补丁投递（F，本轮最大增量）

- **本地层 `.zpf`/`.zpfz`**：`OoT-Randomizer/N64Patch.py:101`
  `create_patch_file(rom, file, xor_range=(0x00B8AD30, 0x00F029A0))`、`:106` 写 `'ZPFv1'`、`:108-109` 把 `xor_range` 两端
  作 int32 入头；应用侧 `:200 apply_patch_file`、`:214 zlib.decompress`、`:218` 非 `b'ZPFv'` 报 "File is not in a Zelda Patch Format"、
  `:220-222` 版本字节检查。`.zpfz` = ZIP(`P<n>.zpf`)（`Main.py:468-474`），设置项 `SettingsList.py:418 "extensions": ["zpfz","zpf","patch"]`。
  ⇒ **XOR key 是从 ROM 自身某区间流式取出的**（`N64Patch.py:36-61 write_block/key_next`），不是外部密钥（F）。
- **网页层 `.patch` = minibsdiff**：`MBSDIFFPatch.py:15 apply_ootr_3_web_patch`、`:87` 头 8 字节必须 `b'MBSDIF43'`、
  `:44` 纯 Python 回退 `apply_minibsdiff_patch_file`、`:94-97` gzip 容器、`:121/:144` ctrl/extra 块读取、`:159` 8 字节再读；
  由 `Main.py:542`（`extension == 'patch'`）调用，`:334-345`/`:17-44` 按平台选择仓库内预编译的
  `bin/minibsdiff/`、`bin/gzinject/`（含 `ootr_usa.gzi`/`ootr_jpn.gzi`）、`bin/Compress/`、`bin/Decompress/`。
  ⇒ 这是**"GPL 仓库内夹带预编译黑箱"**的第二例（§3.2 的 `z64audio.exe` 为第一例），且 `bin/` 内二进制本轮未做 REA 取证（U）。
- **`ZPFv1` 的 JS/WASM 实现：不存在**（U，检索 `zpf extension:js`、`"ZPFv" -repo:OoTRandomizer/OoT-Randomizer` 零命中）
  ⇒ 浏览器端无法离线施加 `.zpf`，只能走 OoTR 网页的 `.patch`(minibsdiff) 或 RomPatcher 类格式。

## 6.10 RomPatcher.js 生态的真实归属与"宣称面 ≠ 实现面"（F）

- **归属更正**：`marcrobledo/RomPatcher.js`（1244★）。`package.json:20 "author": "Marc Robledo"`、`:21 "license": "MIT"`。
  ⇒ `02-rompatcher-otr-textures.md` §2.1 若写 JException 则为**错误**（本文件即更正记录）。
- **`patchmyrom`（Next.js）原样 vendor**：`public/workers/rom-patcher-worker.js:4` 注释 "vendored unmodified"、
  `:19-33 self.importScripts(...)` 载入全部 10 个格式模块；`rom-toolkit.js:586-608` 复刻同一 magic 表并注明
  "Same order and same prefixes as RomPatcher.parsePatchFile()"；`lib/format-labels.ts:28` 把 `VCDIFF` **显示为 "xdelta"**（?= 命名误导）。
  `lib/worker-urls.ts:5,7,10,12` 四个 worker URL；`lib/data/pokemonHackBaseRomTable.ts:121 commonPatchFormat: "xdelta"`（仅元数据，无 ROM）。
- **Byte-Forge（Nuxt TS 移植）有硬缺陷**：`lib/core/rom-patcher.ts:42-51` 的注册表里
  `// [BPSModule.magic…]`、`// [APSModule…]`、`// [PPFModule…]`、`// [RUPModule…]` **四行被注释掉**，
  运行时只识别 IPS/UPS/APSGBA/PMSR/VCDIFF，而 `README.md:2` 宣称支持 BPS/PPF/RUP/APS ⇒ **文档与实现矛盾（F，可复现缺陷）**；
  其 `lib/formats/aps.ts:5` 仍定义 `APS_N64_MAGIC = 'APS10'`，`:76-80` 有 N64 cart-ID/CRC 头显示逻辑（代码存在但未注册）。
- **`rom-weaver`（Rust→WASI/WASM）**：`crates/rom-weaver-patches/src/` 有 22 个格式模块
  （`ips/ups/bps/ppf/rup/pmsr/bdf/xdelta/vcdiff/hdiffpatch/gdiff/dldi/dps/pat/ninja1/solid/basis_probe/bsp`）；
  分派 `src/lib.rs:348-359` 四个 SIGNATURE 常量、`:647 probe_by_signature(...)`、`:661` APS-GBA 与 APS10 消歧、
  `:533` Vcdiff handler；`packages/rom-weaver-webapp/package.json:58` 走 `scripts/ensure-wasm-build.mjs`。
  ⇒ 覆盖面远超 RomPatcher.js（F），但其 `.wasm`/`.bsp` VM（附 `bsp_vm_runtime.js`）段级内容未取证（U）。
- 另有 `anypatch`（Python，`src/anypatch/patchio.py:115-135` 列 22 种格式名 + `MAGICS` + `APPLIERS` dict，31 个格式模块）、
  `tiny-little-patcher`、`gba-to-3ds-cia`、`SebaaMG/ns2jb-modloader`（Switch 2 modloader，
  `native/lazy-nso.c:55` `unsigned width=lfs_equal(data,"IPS32",5)?4:3;`、`:56` 非 `PATCH` 则返 `0xd401`）、
  `craftycorvid/oot-mod`（`build.py:205 blob = bytearray(b"IPS32")`、`:212 assert blob[:5]==b"IPS32" and blob[-4:]==b"EEOF"`）
  ⇒ **Switch 2 时代的 OoT 补丁实际收敛到 IPS32**（F，与 §2.6 的 `TOO_BIG_ROM_SIZE = 67108863` 64 MiB 上限形成对照：
  IPS32 的 4 字节偏移正是为超过 16 MiB/64 MiB 而存在）。

## 6.11 Switch 2 HD 纹理管线（F，替代"OTR GUI"缺位）

`Hookshot-Textures/hstex/`：`bntx.py:1` 文档字符串（Switch 2 OoT 以 **BNTX + block-linear + "Ounc" 平台标记** 发布纹理）、
`:131-132` `if raw[:4] != b"BNTX": raise ValueError("not a BNTX")`；
`rstb.py:1-3` **RESTBL**（CRC32 键的资源大小表）、`:13` `NAME = "System/Resource/ResourceSizeTable.Product.111.Nin_Ounce_NVN2.rsizetable.zs"`、
`:48 def fit_mod(self, mod)`（按 mod 体积重算堆）；`byml.py:1` BYML v2–7（`.bgyml/.byml.zs`）；
`cli.py:10` 输出为"upscaled 2x, BC7, 写入 `<out>/romfs/Tex`"、`:35`、`:119 heap_cmd`。
⇒ 高清纹理包的三条互不相交技术路线就此确定（F）：
SoH/MM3D 用 **MPQ/ZIP（.otr/.o2r）**；N64 原始用 **CI4/CI8/IA/I 与 `n64texconv`**（§3.3）；
Switch 2 用 **BNTX/BC7 + RESTBL 堆重算**。

## 6.12 N64 纹理编解码枚举的第二个独立来源（F）

`queueRAM/Texture64/Texture64/N64Graphics.cs:5`
`enum N64Codec { RGBA16, RGBA32, IA16, IA8, IA4, I8, I4, CI8, CI4, ONEBPP }`；
`Random06457/Z64Utils/README.md:23` 同一 RDP 格式列表。
⇒ §4 OBJEX 的 `newtex format ∈ rgba32/rgba16/ia16/ci4/ci8/ia4/ia8/i4/i8` 与 zzrtl `n64texconv` 之间再添一源，
枚举集**三处一致**（F），`ONEBPP` 为 Texture64 独有的第 10 项（= 1-bit 图标格式）。
