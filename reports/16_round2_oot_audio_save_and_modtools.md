# 16 · 第二轮 OoT 周边工具增量扫描：存档转换 / Web 补丁 / 纹理与模型导入 / 音频管线

- **工单**：`emrpg/gta6-oot-rea-reverse-engineering#8`（第二轮周边工具增量扫描）
- **执行终端**：本机 WSL2（Linux 6.18.58.1-microsoft-standard-WSL2，x64，Node v26.3.0），工作目录 `/root/wsl-demo`
- **取证工具**：`/usr/local/bin/rea`（provider `rea-artifact-graph` / `rea-binary-hardening` / `rea-javascript-application`，version 1）
- **日期**：2026-11-19
- **仓库可见性**：**PUBLIC**（`https://github.com/emrpg/gta6-oot-rea-reverse-engineering`）。本报告与全部笔记仅陈述**开源仓库的静态阅读结论与 REA 头部/导入表取证**；提交即公开，请勿在此仓库放置 ROM、密钥、真实存档或导出资产。
- **分级图例**：**F** = 已核实事实（`path:line` 或 REA 证据可指）；**R** = 参考实现可支撑但未逐字节验证；**U** = 未找到 / 当前授权下无法确定；**?** = 证据存在但解释不唯一。

---

## 0. 核心判断（先说结论）

1. **本轮采集 82 个仓库、0 个 ROM/密钥/真实存档**（`save/` 24 + `patch-otr/` 25 + `audio-model/` 33，
   含 3 个子 agent 的 payload 清理）；`du -sh oot-peripherals/round2-audio-save-modtools` ≈ **382 MB**。范围合法（工单点名对象全为开源项目）。
2. **工单前提有 2 处需更正**（F/R，§7.0/§90#2）：`zzromtool` **从未开源**（仅 z64me 博客），GitHub 用户 **`NullEndr` 不存在**；
   `z64audio` 只有**预编译 exe**（GPL 仓库内夹带黑箱），故该子任务只能以"接口面取证 + 周边开源管线还原"完成，**不做函数体复刻**。
3. **N64 存档侧的三条硬结论互相咬合成环**（F）：`.sra` 布局 `0x20 + 6×0x1450 + 0x600 = 0x8000`（三源）；
   校验和 `0x09A9` 个大端字 `% 0xFFFF` 写回 `0x1352`（`0x1352` 同时是偏移与长度，纯巧合）；
   槽内 `0x001C` 的 6 字节模式 `"ZELDAZ"` 与实卡 dump 的识别标记**是同一常量**（§6.1）。
4. **BizHawk `.SaveRAM` 与 mupen `.srm` 的容器偏移获得第二独立来源**（F）：
   `296960 = 0x48800`、存档落点 `0x20800`、前导 `0x800` 与第一轮 Java 侧常量表逐值相同（§6.4）——
   §1.2 的 C 代码推断从"单一实现"升级为"生态共识"。
5. **补丁生态的真实分层被还原出来**（F，§6.8/§6.9/§6.10）：
   RomPatcher.js（**marcrobledo**，MIT）是格式参考实现；OoTR 用**两层**投递（本地 `.zpf/.zpfz` = `ZPFv1`+ROM 自取 XOR key；
   网页 `.patch` = `MBSDIF43` minibsdiff）；Switch 2 时代收敛到 **IPS32**（4 字节偏移）；
   而 RomPatcher.js 的 `TOO_BIG_ROM_SIZE = 67108863`（64 MiB−1）正是这条收敛的动因。
6. **"高清纹理包 / OTR GUI" 的答案不在 Web**（F，§6.8/§7.5）：`.otr` = **MPQ**、`.o2r` = **ZIP**（C#/C++ 双源），
   浏览器端走 Blazor WASM AOT；Switch 2 用 **BNTX/BC7 + RESTBL 堆重算**；N64 用 CI/IA/I 与 `n64texconv`。
   **本轮唯一真正的 Web 端模型+音频应用是 `oot3d-zobj-import`**（Pyodide worker），其余"Web 端"诉求为 U。
7. **音频侧解了一桩旧案**（F，§7.4a）：`gfxasm` 的三个 exe = **同一份 C 源码 + `-DGBI_PREFIX=F3D|F3DEX|F3DEX2` 三次编译**，
   stdin 逐行宏 → 8 字节 display-list 字；这解释了 §5.2 三个镜像 imphash 各异而行为一致。
8. **N64 VADPCM 与 3DS DSP-ADPCM 的帧头 4+4 位语义恰好互换**（F，§6.5 vs §7.4e）：
   N64 `(shift<<4)|filter` + 16 样本；DSP `(pred<<4)|scale` + 14 样本。跨平台"通用 ADPCM"写法必然出错。
9. **并发提交 `7f05074`（他人终端，已 push 到 PUBLIC main）含 4 处技术错误与 1 处命名越界**（F，§6.6/§90#17–#21）：
   `& 0xffff` 应为 `% 0xFFFF`；VADPCM codebook 形状 `2×8` 应为 `8×(order+8)`；`.c` 文件里出现 C++ 的 `static_cast`；
   文件名宣称 IPS32/IPS 却无一行 IPS 代码；把"规格推导"命名为 `*_reconstructed` 越界。**本报告不改写他人历史提交**，
   只做留痕与修正实现（§6.2/§6.5）。
10. **REA 能力边界本轮再次确认**（F，§5.4/§90#25）：`inspect-wasm-artifact` 仍不可用（缺 WABT，不得隐式安装）
    ⇒ 凡 WASM/AOT 段级还原记 U；`inspect-artifact` 对小 PE/ELF 返 `architecture: null`，machine 必须从
    `inspect-pe-imports` 取；`--evidence_output` 仅 `analyze-javascript-application` 支持；
    本轮新增一手证据：`analyze-javascript-application` 对 Pyodide 应用的 `fetch(\`../tools/${name}.py\`)`
    **保留为 2 个 unknown**，模板串模块名无法静态定名。
11. **义务台账本轮不重建**（决定，F 级自我约束）：第一轮 ledger 已覆盖闭源面；本轮 82 个仓库均为开源且许可可辨，
    义务差异集中在 **GPL-3.0（zzrtl/z64ext/OoT-Randomizer/OTRMod 系）与无许可证仓库**（`zzrtl-audio`、`N64SaveConverter`、
    `zobj`、`OOT-Save-Converter` 等）⇒ 详见 §5 的处置说明，逐仓 SPDX 已在 `notes/01/02/07` 表内登记。

---

## 1. 工单核对与范围裁定

| 工单要求 | 完成情况 | 说明 |
|---|---|---|
| 增量扫描存档转换器（N64/3DS/GC/Switch 互转与编辑，`oot-save-editor`/sram 工具） | ✅ 24 仓 | §1.0/§1.0-b/§6.1–6.4 |
| Web 端 RomPatcher.js | ✅ 主目标 + 9 个同族 Web/CLI 补丁器 | §6.8–6.10、§2 |
| 高清纹理包 / OTR 打包周边 GUI | ✅ 但**答案在原生/Blazor**，Web 端为 U | §6.8、§7.5、§7.7 |
| 自定义模型导入（`zzrtl`/`zzromtool`/`z64audio`/`seq64` 周边） | ⚠️ 部分：`zzromtool` 无仓库（U）；`z64audio` 仅二进制（接口取证）；其余 ✅ 33 仓 | §7.0、§7.1–7.4 |
| `git clone --depth 1` 到 `round2-audio-save-modtools/` | ✅ 82 仓 + `binaries/` | 全部浅克隆；sparse/删 blob 处置见 §3 |
| `rea` 静态逆向与接口还原，写入 `rea-reconstructions/round2-oot-modtools/` | ✅ 证据 29 件 + 笔记 8 篇 | §4/§5 |
| 深度报告 `reports/16_…md` | ✅ 本文件 | 取代并发提交的 36 行版本（见 §6.6 说明） |
| `gh issue comment 8` + `git push` | ✅ 本次执行 | push 前 `git status` 复核、限定路径 |

**拒绝项**：无（本轮无闭源复刻诉求）。对 `z64audio.exe`/`ootr_native.dll`/`gfxasm`/`minasm`/`etc1compress`/`ETC1Lib.dll`/`anim2link.exe`
仅做**身份、导入表、加固、字符串与接口面取证**，不产出任何"函数体还原"（§5.5）。

---

## 2. 采集与出处验证

- 克隆根：`/root/wsl-demo/oot-peripherals/round2-audio-save-modtools/{save,patch-otr,audio-model}/`
- 三子任务体积：`save/` 103 MB（其中 82 MB 为 `save-file-converter`，70 MB 是上游前端单测 fixtures）、
  `patch-otr/` 197 MB（`rom-weaver` 97 MB）、`audio-model/` 78 MB（R：`du -sh` 由 agent 产出，我方复核总量 382 MB）。
- **payload 卫生处置**（F/R）：
  - `Z64-CustomPlayerModels` 571 MB 用户模型 blob → `--filter=blob:none` sparse checkout 仅 `zobj_checker/`+`guide/`（628 KB）；
  - `zzconvert-Blender-Link-Templates` 删除 458 `.tga` + 52 `.png` 导出纹理与 18 个衍生 `.zobj`（34→29 MB），保留 `.blend/.objex/.skl/.mtl/.h/.mnf`；
  - `zobj` 删除 `docs/tex/`（`object_link_boy` PNG dump）与 `docs/meme/`，保留 HTML 格式文档；
  - 未克隆：`*-Reloaded` 纹理包、`Switch-Toolbox`（770 MB）、`MeltyTool`（git 3.12 TB）、`Vi1i/OcarinaChecksumChecker`
    （含 8,388,607 B ROM 片段与真实 `.sra`）、`dlr-developer/…switch-2-save-editor`（~200 张 ripped 图标）、
    `ComplexPlane/gci2raw`（克隆后确认是 6 行 stub，已删）。
- 下载二进制 8 件（≤20 MB）：`zzrtl-v1.0.5-win32.exe`、`z64convert-cli-linux-1.1.1`、`z64playas-linux-0.9.2`、
  `N64SaveConverter-v1.0.exe`、`zs-flash-v2.0-Windows.zip`、`Texture64-v0.2.zip`、`OTRMod.CLI.exe`、`ns2jb-modloader.zip`；
  另有第一轮既有 `gfxasm.*`、`minasm.exe`、`etc1compress(.exe/linux)`、`ETC1Lib.dll`、`anim2link` Debug/Release、`z64audio.exe`。
- `.gitignore` 追加：`oot-peripherals/`、`**/evidence/`（大 bundle 不入版本库）；`git ls-files | grep -c round2-audio-save-modtools` = **0**（F）。

---

## 3. REA 取证（一手）

命令面与产物见 `notes/05-evidence-manifest.md`；要点：

| 工件 | REA 操作 | 关键观测 |
|---|---|---|
| `z64audio.exe` | `inspect-artifact` / `inspect-pe-imports` / `inspect-binary-hardening` / `inspect-pe-resources` / `audit-release-exposure` | PE32 **machine 332 (i386)**、2 757 038 B、sha256 `3d173b43…65bf`；导入 6 DLL/270 函数（含 `OPENGL32.DLL` 1 个）；`dynamic_base`+`nx_compat` enabled、`guard_cf` disabled、无证书表、**无 debug 目录**；字符串 `z64audio.toml`/`book_*`/`.book.bin`/`loopbook`/`vadpcm` |
| `gfxasm.f3dex2.exe` | `inspect-artifact` + `inspect-pe-imports` + `inspect-binary-hardening` | 65 536 B、332、imphash `17326f9b13060ea6dd2eccac9708e87c`、**ASLR 与 DEP 均关闭**、无签名 |
| `minasm.exe` | 同上 | 78 336 B、332、imphash `836e423864b7d5f109a372e6e87e1c69`、同样无 ASLR/DEP |
| `etc1compress.exe` / `ETC1Lib.dll` | 同上 + `inspect-pe-certs` | 60 416 B/34404，ASLR+HEAP+DEP+CFG **全开**、debug 存在；DLL 17 408 B/332、导入 `MSVCR100` |
| `etc1compress`(ELF) / `minasm`(ELF) | `inspect-artifact` | full RELRO + `__stack_chk_fail` vs **partial RELRO、无 canary**（同作者两平台加固不一致） |
| `anim2link.exe`（Debug/Release × obj/bin） | `inspect-managed-members` | 4 镜像逐字节两两相同；metadata `v4.0.30319`，类型/方法/字段计数全表入 `notes/05`；`rankaisija` 构建者目录与 §7 的 anim2link README 致谢**同一 handle**（两条独立证据指向同一维护者） |
| `RomPatcher.js`（未打包源码） | `analyze-javascript-application` | 与第一轮"出厂 bundle"对比：AST 节点 **49 307 / findings 123**，而第一轮 bundle 为 337 426/980 ⇒ **同一软件，源码面信噪比高一个量级**（F，方法学结论） |
| `oot3d-zobj-import`（Pyodide Web app） | `analyze-javascript-application`（`--detail summary --evidence_output`） | `relevant_files 8 / parsed_javascript_files 3 / visited_ast_nodes 3468 / text_bytes_read 34 835 / findings 6 / modules 0 / parse_failures 0`；Electron 计数器全 0；语义图 `nodes 2750 / relations 2433 / unknowns 1386 / fingerprints 64 / root_node_ids 3`；`request` 族 8 关系含 `cdn.jsdelivr.net/pyodide/v0.29.3/full/`、`pyodide.js`、`dspadpcm.js?v=20261013b`、`../tools/${name}.py`×2；**inventory_graph_sha256 `84dbc582…`，root `b2801227…`** |
| WASM 目标（`psp-encryption.wasm` 70 000 B、Blazor AOT、rom-weaver WASM） | `inspect-wasm-artifact` **不可用** | 缺 `REA_WABT_BIN_DIRECTORY`（wasm-validate/objdump/2wat，版本 ∈[1.0.42,1.1.0)）；不得隐式安装 ⇒ 段级还原统一记 **U** |

---

## 4. 接口还原产物（`rea-reconstructions/round2-oot-modtools/`）

`notes/` 8 篇（本轮 6 篇新增/大改）：

| 文件 | 内容 | 体量 |
|---|---|---|
| `README.md` | 索引 + F/R/U/? 图例（本轮修正：`91-evidence-manifest` → 实际 `05`，补 `04/06/07/90`） | — |
| `01-save-converters.md` | 24 仓清单（§1.0 + §1.0-b）、N64 容量表、`.SaveRAM` 逐字节还原与 3 处缺陷、三个 swap 脚本同构、DreamDumper64→SummerCart64、ZeldaSaveTool 枢纽、ByteOrder 四族置换、N64↔3D 字段级对照、`0x20+3×0x1450+0x4 skip` | 22 KB |
| `02-rompatcher-otr-textures.md` | APS(N64) 逐字节（头 61/78 B、记录流、**APS 不能扩展 ROM**）、BPS/UPS VLV 位算术、magic 全集、CLI 可写面 ⊂ 可读面、`FILTER_PATCHES` 白名单宽于实现、PMSR/Yay0/XZ、嵌入式 `fetch` | 15.6 KB |
| `03-zzrtl-import-pipeline.md` | zzrtl 内嵌 C 解释器的禁用面、rom_* API、codec 分派、**cloudpatch 文本格式逐行施加且无长度校验**、`n64texconv` 矩阵、z64audio 黑箱 | 12.4 KB |
| `04-objex-model-pipeline.md` | OBJEX 版本握手 `2.major`、`exportid`、纹理/材质/`gbi` 宏、`.skel`/`.anim`、碰撞组名约定、anim2link | 10.4 KB |
| `05-evidence-manifest.md` | 逐工件身份表、加固矩阵（PE 9 属性×4 + ELF 5×2）、托管计数、WASM 边界、**第一/第二轮方法学对照** | 9.2 KB |
| **`06-checksums-and-container-identities.md`** | 本轮硬事实汇总：`.sra` 槽布局与偏移速查、**校验和算法族对照表**、DexDrive 两种读取宽度、容量矩阵第二源、**VADPCM 位约定**、`7f05074` 逐条核查、`z64audio.exe` 字符串面、**OTR=MPQ/O2R=ZIP**、`.zpf`+`.patch` 两层投递、RomPatcher 生态归属与"宣称面≠实现面"、Switch 2 BNTX/RESTBL、N64 纹理枚举第三源 | 12 KB |
| **`07-audio-model-pipeline.md`** | 归属/命名更正、OBJEX 枚举 6 值、`.zobj` unpack 串、play-as LUT/manifest、**gfxasm 三 exe 之谜**、音频表基址（debug vs 零售）、bank 签名与地址、DSP-ADPCM 帧与 `self.dspEncode` 覆盖、CSAR/`QueenSound.bcsar`、**CMB magic 常量表**、`oot3d-zobj-import` Web 架构、OOTR_Blender 的 `ootr_native.dll` 黑箱 | 13 KB |
| **`90-agent-claims-verification.md`** | 33 条承重断言逐条裁决（含 4 处推翻并发提交） | 9 KB |

**未产出的东西（有意为之）**：没有 `*_reconstructed.c` 形式的"z64audio 算法复刻"，没有 CMB/Grezzo 解析器实现体。
可读接口一律以**开源源码的常量表 + `struct.unpack` 串 + CLI 参数表**形式给出（§7.1–7.4），这是可核验的还原；
其余以规格推导写出的代码若存在，必须标"spec-derived"而非"reconstructed"（§6.6 教训）。

---

## 5. 义务台账的处置说明

- 本轮**不重建** REA obligation ledger（决定依据：第一轮 ledger 覆盖闭源面；本轮 82 仓全开源，义务差异可用许可矩阵表达）。
- 许可分布（F：源码/README；R：GitHub API `license` 字段）：
  GPL-3.0（zzrtl、z64convert、z64playas、z64yartool、z64font、zztexview、z64viewer、zroomutil、gfxasm、z64ext、z64enc、
  yaydump、io_export_objex_2.5、objex_for_2.79、Dragorn421/io_export_objex、Z64-CustomPlayerModels、CMB-Exporter、
  ns2jb-modloader、OoT-Randomizer、save-file-converter、imaandrew/N64-Save-Converter、Admentus64）、
  GPL-2.0（segher-wii-tools 文件级）、MIT（RomPatcher.js、patchmyrom、Byte-Forge、OTRExporter、n64SaveConverter、saveswap、
  tiny-little-patcher、Hookshot-Textures、Darkaxt converter、N3DSCmbViewer、OOTR_Blender、mm-recomp-save-edit(CC0)、zs-flash(CC0)）、
  OSL-3.0（ZeldaSaveTool、OTRMod）、Apache-2.0（xdelta-wasm）、CC0（z64ext? 见上）、LGPL-3.0（z64me/zelda64-import-blender）、
  **无许可证**（`zzrtl-audio`、`N64SaveConverter`、`mempak`、`OOT-Save-Converter`、`n64-sram-converter`、`7ank0v1c`、
  `zelda64`、`zobj`、`z64scene`、`anim2link`、`oot-sw2-save`、`oot-editor`? 等）⇒ **无许可证仓库不得直接并入本仓库产物**，
  仅可作事实来源；这是本轮最大的合规观察。
- **GPL 仓库内夹带无源码预编译二进制**共 3 例（F）：`z64audio.exe`（`z64-sample-creation-tools`）、
  `OoT-Randomizer/bin/{minibsdiff,gzinject,Compress,Decompress}/`、`zzrtl-audio` 自带 `zzrtl-r5b-4.exe`（且仓库无许可证）。
- **闭源黑箱新增 1 例**（F）：`OOTR_Blender/core/native_bridge.py:87 → "ootr_native.dll"`（`ctypes.CDLL`），
  该 DLL 不随仓库发布 ⇒ 记 U，未取证。

---

## 6. 与并发工作的对账（`7f05074`）

本轮另一终端已提交并推送 `7f05074`（3 个 `*_reconstructed.*` + 36 行 `reports/16`）与 `dfdcfad`(#7)、`e42c53c`(#6)、`dc42fbd`(#5)。
`origin/main..HEAD` 为空 ⇒ **这些内容已在 PUBLIC 历史中**。处置：

- 不回滚、不改写他人历史（避免破坏共享状态）；
- 本 `reports/16` 作为**替代正文**提交（工作树修改，新提交），并在 §0.9 与 §6.6 留痕其 4 处技术错误 + 1 处命名越界；
- 三份 `*_reconstructed.*` 文件**暂留原位**（其 BPS/UPS/VLV 部分正确，已被 §2/§6 引用），但按其真实来源应重命名为 `*_spec_derived.*`；
  若后续要改，需先与本会话作者确认，因为它已公开。
- **待用户决定的暴露问题（沿用第一轮）**：`rea-reconstructions/closed-source-code/` 与 `rea-reconstructions/oot-binaries/` 内
  若干"闭源接口还原"文件仍在 PUBLIC 历史中，是否需要清理/脱敏，请明示；我不会自动删除。

---

## 7. 关键增量点（相对报告 08 与 12）

| 主题 | 第一轮（08/12） | 本轮新增 |
|---|---|---|
| 存档 | 只有 `Oot3DS-Save-Editor-For-Azahar` 与 OoT3D `program id 0400486024DEC000` | `.sra` 算术闭合、**校验和算法族 6 行对照**、DexDrive 双读取宽度、BizHawk/`.srm` 偏移双源、ByteOrder 四族置换向量、N64↔3DS 字段级差异表、实卡链路（DreamDumper64→SummerCart64，`ZELDAZ`）、Switch 2 murmur3 容器（同 id 三源） |
| 补丁 | RomPatcher 仅作"网页补丁器"提及 | APS(N64) 完整格式与"APS 不能扩 ROM"的推论、BPS VLV 位算术、`TOO_BIG_ROM_SIZE` 与 IPS32 关系、`.zpf/.zpfz` vs `.patch(MBSDIF43)` 两层、9 个同族 Web/CLI 工具、Byte-Forge 注册表缺陷 |
| 纹理/模型 | 第一轮只覆盖 OoT3D-Editor/noclip.website | **OTR=MPQ、O2R=ZIP（双源）**、Blazor WASM AOT、BNTX/RESTBL/BC7（Switch 2）、`newtex format` 三源一致、Texture64 `ONEBPP` |
| 音频 | 只有 `z64audio.exe` 存在性 | 导入表/加固/字符串三面取证、`book_*` 参数面、TOML 驱动、**gfxasm 三 exe 解释**、debug vs 零售表基址两套、bank 签名与数量、**DSP vs N64 ADPCM 帧头互换**、CSAR/`QueenSound.bcsar` 语音管线 |
| 方法学 | ledger + bundle 取证 | **未打包源码 vs 出厂 bundle 的信噪比量化（49 307/123 vs 337 426/980）**、Pyodide 应用的 dynamic `fetch` 记 unknown、`inspect-artifact` 的 `architecture: null` 需以 `inspect-pe-imports` 补 machine |

---

## 8. 未找到清单（均为"检索未命中"，非"不存在"）

- **N64 → 3DS 写入方向**：无任何工具把 `.srm/.eep/.fla` 转成 OoT3D/MM3D 3DS 存档；`ZeldaSaveTool` 只解析 `CTRSave`（`File.cs:87,379`），导出列表止于 `PcPortSav`（`ToolForm.cs:34`，F）。
- **N64 → Switch 2 remake `progress.sav`**：无字段映射工具（remake 侧全是 murmur3 哈希容器）。
- **N64 → Wii/Wii-U VC 打包器（OoT 特化）**：只有反方向 + 通用 GCI 写（`twintig.c`）。
- `.mcf/.mcp/.pdm/.gcs/.dsp` 控制卡镜像工具；mupen64plus `.mcf`；3DS SD 目录布局编辑器（OoT3D）。
- **JS/WASM 实现的 `.zpf` 应用器**（浏览器无法离线施加 `.zpf`）；`.fpz`；`otr.json` schema；`jpcxr`。
- MML→`.seq`、MIDI↔`.seq`、`.x3d/.fx` 混响表、`.mov64`、`.bfgrp/.awb`（OoT 无）。
- `zzromtool`、`mashtool`、`zscrenderer`、`ootr-yz`、`NullEndr`（**账号不存在**）、"Gooee 作为 OTR 构建器"。
- 除 `oot3d-zobj-import` 与 `Ocarina-Atlas` 之外的 Web 端音频/模型编辑器；`z64me` Web ROM 查看器源码未公开。

---

## 9. 结论

第二轮增量把"周边工具"从**存在性调查**推进到**可核验的接口还原**：82 个仓库、29 件 REA 证据、8 篇笔记，
得到 5 条可跨工具复用的硬知识（`.sra` 布局算式、校验和算法族、OTR/O2R 容器身份、补丁两层投递、ADPCM 帧头互换），
并纠正了工单与并发提交合计 6 处错误前提/实现。同时确认三条无法在本地闭合的边界：
**WASM/AOT 段级取证缺 WABT（U）**、**无许可证仓库只能作事实源不能作代码源**、
**N64→3DS 与 N64→Switch 2 的存档写入方向在生态中确实缺位**。

## 附录 A · 证据与复现

```bash
# 采集（浅克隆；示例）
git clone --depth 1 https://github.com/marcrobledo/RomPatcher.js
git clone --depth 1 https://github.com/euan-forrester/save-file-converter
git clone --depth 1 --filter=blob:none --sparse https://github.com/hylian-modding/Z64-CustomPlayerModels

# REA 取证（本轮实际使用）
rea inspect-artifact <pe|elf>
rea inspect-pe-imports <pe>            # machine 取这里（inspect-artifact 对小吃回 architecture:null）
rea inspect-binary-hardening <pe|elf>
rea inspect-pe-certs <pe>
rea inspect-managed-members <net-asm>  # 不支持 --evidence_output，用 stdout 重定向
rea analyze-javascript-application <dir-or-asar> --detail summary \
    --evidence_output <bundle.json>    # 唯一支持 --evidence_output 的命令
rea inspect-wasm-artifact <wasm>       # 本轮仍不可用：需 REA_WABT_BIN_DIRECTORY

# 一手源码定位（示例，均可直接跳转）
oot-peripherals/oot-save-edit/models/saveslot.ts:330-339,410-412,1072-1114
oot-peripherals/oot-save-edit/models/savefile.ts:104-118
oot-peripherals/round2-audio-save-modtools/save/segher-wii-tools/zelda-cksum.c:11-22,34-40
oot-peripherals/round2-audio-save-modtools/save/ZeldaSaveTool/ZeldaSaveTool/Save/File.cs:79-95
oot-peripherals/round2-audio-save-modtools/patch-otr/OTRMod/OTRMod/OTR/Generate.cs:21-40
oot-peripherals/round2-audio-save-modtools/patch-otr/OoT-Randomizer/{N64Patch.py:98-109,MBSDIFFPatch.py:87}
oot-peripherals/round2-audio-save-modtools/audio-model/oot3d-zobj-import/web/{worker.js:7-31,dspadpcm.js:1-12}
oot-peripherals/round2-audio-save-modtools/audio-model/CMB-Exporter/cmb_internal/cmb_constants.py:4-17
oot-peripherals/ocarina-songbook/ocarina_songbook/oot_soundfont.py:199-209
```

证据 bundle（本地，未入库）：`rea-reconstructions/round2-oot-modtools/evidence/`
（`oot3d-zobj-import__analyze-javascript-application.bundle.json` 4.9 MB、
`RomPatcher-js__…bundle.json` 73.9 MB、`z64audio_exe__*.json`、`anim2link-*__managed-members.txt` 等 29 件）。
