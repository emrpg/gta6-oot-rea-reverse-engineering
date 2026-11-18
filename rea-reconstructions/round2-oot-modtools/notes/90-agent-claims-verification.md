# 90. 断言复核：子 agent 与并发提交的逐条裁决

方法：对每条**承重断言**（决定报告结论的那类）亲自打开文件读 `path:line`；
对不可复核的元数据（star 数、HEAD 日期、体积、检索 0 命中）保留为 **R（报告值）** 并在正文注明。
本轮共打开核对 **31 个文件 / 约 120 处行号**（清单见 §7.6），下面只列裁决结果。

| # | 断言（来源） | 裁决 | 核对依据 |
|---|---|---|---|
| 1 | RomPatcher.js 作者是 Marc Robledo、MIT（agent） | **F，且推翻我方工单前提** | `package.json:20-21` |
| 2 | `zzrtl` = `z64dev/zzrtl`；`NullEndr` 用户不存在；`zzromtool` 未开源（agent） | **R→F（版本号面）**：`zzrtl.c` 内版本串与 v1.0.5 release 一致；GitHub 404 与博客索引为 agent 侧观测，本报告按 R 引用 | agent + 本地 `zzrtl` 源码 |
| 3 | Byte-Forge 注册表里 BPS/APS/PPF/RUP **四行被注释**，与 README 宣称矛盾（agent） | **F，可复现缺陷** | `lib/core/rom-patcher.ts:41-45`（注释行原文已读） |
| 4 | OTR = MPQ、O2R = ZIP；O2R 用 epoch 时间戳（agent） | **F** | `OTRMod/OTR/Generate.cs:21-33,35-40`；C++ 第二源 `OTRExporter/…/ExporterArchiveO2R.cpp:16,23,28` |
| 5 | `.zpf` 头写 `'ZPFv1'`，`xor_range` 默认 `(0x00B8AD30, 0x00F029A0)`（agent） | **F** | `N64Patch.py:101,106,108-109`；判别 `:218` 非 `ZPFv` 报错 |
| 6 | OoTR 网页层 `.patch` = minibsdiff，头 8 字节 `MBSDIF43`（agent） | **F** | `MBSDIFFPatch.py:15,72,87,121,144,159` |
| 7 | N64 `.sra` = `0x20 + 3×0x1450`（+备份，共 `0x8000`）（agent + 我方） | **F，三源** | `saveslot.ts:330-339`、`savefile.ts:104-118`、`oot.js:44-51`、`ZeldaSaveTool/Save/File.cs:82-84`（含 "skip 0x4 between slot 2 and 3"） |
| 8 | OoT N64 校验和：16-bit 大端字、`0x09A9` 字、`%= 0xFFFF`、写回 `0x1352`（我方） | **F** | `saveslot.ts:1088-1096,1072-1078` |
| 9 | GC 侧塞尔达校验和：字节和 `0x0a8c` + `nsum = -(sum + 0x0a8c)` + 两个 BE32（agent） | **F** | `segher-wii-tools/zelda-cksum.c:11-22`；整包 u16 @`0x1ffc/0x1ffe` `:34-40` |
| 10 | `SIZE_SRM = 296960`、`SIZE_SRA_SRM_OFFSET = 133120`、`SIZE_MPK_SRM_OFFSET = 2048`（agent） | **F，且与 §1.2 BizHawk `0x48800/0x20800/0x800` 同值** ⇒ 容器偏移获双源印证 | `N64SaveConverter.java:42-49` |
| 11 | DexDrive `HEADER_LENGTH = 4160`、magic `'123-456-STD'`（agent）；`mempak` 用 12 字节含 NUL（agent） | **F** | `DexDrive.js:6,24,26`、`mempak/js/parser.js:630`、`mempak/js/init.js:11`（`DexDrive = 36928 = 32768 + 4160`） |
| 12 | `CTRSaveSize = 0x14DC`；导出下拉只有 `N64Emu/N64Console/PcPortSav` ⇒ N64→3DS 方向不存在（agent） | **F** | `Save/File.cs:8,87`、`ToolForm.cs:34` |
| 13 | 判别式 `input[0x87] == 0 ? PcPortSav : N64Emu`（agent） | **F（我方此前未记录，新增）** | `Save/File.cs:91-93` |
| 14 | VADPCM 帧头 `(shift<<4)|filter`、order-2 高 nibble 在前、order-4 用 2-bit（我方） | **F** | `oot_soundfont.py:199,204,208-209`；codebook `8×(order+8)` `:164`；`Codebook.py:44,53,68,72,169` |
| 15 | `z64audio.exe`：PE32/i386、`dynamic_base`+`nx_compat` enabled、`guard_cf` disabled、无证书、无 debug 目录、6 DLL/270 导入（并发提交） | **F** | `evidence/z64audio_exe__inspect-binary-hardening.json`（machine 332、2 757 038 B、sha `3d173b43…65bf`）+ `…inspect-pe-imports.json` |
| 16 | `z64audio.toml` 与 `book_order/bits/frame_size/iteration/threshold` 字符串（并发提交） | **F** | 对二进制 `grep -a` 逐词命中 ≥1 |
| 17 | 并发 `z64audio_vadpcm_reconstructed.c` 的 `codebook_row[2][8]` | **错误** | 应为 `8 × (order+8)`，见 #14 |
| 18 | 同文件在 `.c` 里用 `static_cast<int32_t>` | **不能编译** | C++ 语法 |
| 19 | 并发 `computeOotSlotChecksum` 用 `& 0xffff` | **错误** | 真值 `%= 0xFFFF`（#8）；两者在溢出点差 1 |
| 20 | 并发文件标题含 "IPS32/IPS Patcher" | **名不副实**：全文无 IPS 代码 | 文件内容核对 |
| 21 | 并发引用 `oot-save-edit/models/savefile.ts` 为槽布局/校验和来源 | **部分错位**：常量在 `saveslot.ts`/`saveheader.ts`，`savefile.ts` 只有拼装 | #7 |
| 22 | 并发断言"RomPatcher.js 与 oot-save-edit 无 Electron BrowserWindow/IPC 暴露面" | **成立但表述不足**：`oot-save-edit` 有 `ui/main.ts` **WebView 桌面壳**（其 `AGENTS.md` 自述）⇒ 非 Electron，确有原生宿主 | agent 值 + 上游 AGENTS.md |
| 23 | `gfxasm` 三个 exe = 同一源码三宏编译；stdin 逐行宏 → 8 字节字；`-b` 二进制（agent） | **F，解第一轮悬案** | `gfxasm/src/gfxasm.c:17,20-22,27,32,36,512` |
| 24 | DSP-ADPCM 帧 = `header(pred<<4|scale) + 14 nibbles`，且 JS 覆盖 Python（agent） | **F** | `web/dspadpcm.js:1-9`、`web/worker.js:15-16` |
| 25 | `oot3d-zobj-import` worker 用 Pyodide 0.29.3 CDN、`TOOLS` 28 项、`fetch('../tools/${name}.py')`（agent） | **F，且 REA 独立印证**：bundle 的 `request` 族保留 8 关系含 `cdn.jsdelivr.net/pyodide/v0.29.3/full/`、`pyodide.js`、`dspadpcm.js?v=20261013b`、`../tools/${name}.py`×2；模板串解析为 **2 个 unknown** ⇒ REA 无法静态定名 28 模块，须读 `TOOLS` 字面量 | `evidence/oot3d-zobj-import__analyze-javascript-application.bundle.json` |
| 26 | REA 统计：`relevant_files 8 / parsed_javascript_files 3 / visited_ast_nodes 3468 / text_bytes_read 34835 / findings 6 / modules 0 / parse_failures 0`；Electron 计数器全 0；语义图 `nodes 2750 / relations 2433 / unknowns 1386 / fingerprints 64` | **F（本轮一手 REA 输出）** | 同上 bundle |
| 27 | CMB magic 12 项 + `CMB_VERSION_OOT3D = 0x6` + 小端（agent） | **F** | `CMB-Exporter/cmb_internal/cmb_constants.py:4-17`、`binary.py:23-42` |
| 28 | OBJEX `vertexshading` 实现面 6 值 > 手册 4 值（agent） | **F** | `z64convert/src/objex.h:24-31` |
| 29 | play-as LUT `0x5000..0x5800`、段基 `0x06000000`、终结符 `0xDE`、成人 `0x06005380`/儿童 `0x060053A8`、年龄字节 `0x500B`（agent） | **F** | `zobj_checker/zzplayas_to_zzconvert.ts:15-41,77`、`index.ts:11` |
| 30 | Switch 2 侧只有 IPS32（`oot-mod build.py:205,212`；`ns2jb lazy-nso.c:55-56`）（agent） | **F** | 两处原文已读 |
| 31 | "不存在 OoT 相关的 Web 端音频/模型编辑器，除 `oot3d-zobj-import` 与 Ocarina-Atlas"（agent） | **R**（负断言不可自证；我方仅确认 `z64me` 博客站无应用 bundle） | agent 检索记录 |
| 32 | NOT FOUND 列表（`.fpz`、MML→`.seq`、MIDI↔`.seq`、`.zsc` 其他编辑器、`.mcf/.mcp/.pdm` 工具、N64→3DS 写入、N64→Wii-VC 打包、OoT→SW2 字段映射） | **R**（检索式负断言）⇒ 报告 §8 逐条标注"检索未命中"而非"不存在" | agent 检索记录 |
| 33 | 我方对 agent 的两处反向纠正 | **agent 无错，但我方工单前提有错**：工单 #8 原文要求 "zzrtl/zzromtool/z64audio/seq64 周边"，其中 `zzromtool` 无仓库、`z64audio` 只有预编译 exe ⇒ 该子任务只能以"接口取证 + 周边开源管线"完成 | §7.0 |

**净结论**：三个 agent 的承重断言**全部通过抽查**（0 条伪造 `path:line`）；
被推翻的是**并发提交 `7f05074` 的 4 处技术错误 + 1 处命名越界**（#17–#21），
以及**工单自身的 2 处前提**（#2、#33）。
