# 3. 自定义模型/音频导入管线：zzrtl 周边与 `z64audio` 黑箱

> 分级见 `README.md`。本节全部结论来自只读静态分析：源码阅读 + `/usr/local/bin/rea` 对二进制取证。**未执行任何第三方代码，未修改任何上游文件。**

---

## 3.1 `z64-sample-creation-tools` 仓库定位（F）

本地只读快照：`oot-peripherals/z64-sample-creation-tools/`，**GPL-3.0**。README 自述用途为
"Creates an APDCM file compatible with Zelda64 and a SEQ64 XML instrument bank"，
即它服务于 **N64 Zelda64 音频（APCM/VADPCM）** 与 **seq64 音源库** 两条链，与工单 #8 的
"自定义模型导入工具（zzrtl/zzromtool/z64audio/seq64 周边）"直接对应。

关键接口事实（源码可指行）：

| 事实 | 位置 |
|---|---|
| Python GUI 通过子进程调用外部 `z64audio` 可执行文件 | `WAV to ZSOUND.py:451-458`（`subprocess.run(['z64audio','-i',self.filepath,'-o',…], stdout=PIPE, stderr=PIPE)`） |
| 调用点在 `run_z64audio` 方法内 | `WAV to ZSOUND.py:446` |
| exe 缺失时给出用户提示（说明该二进制是**外部依赖而非仓库产物**） | `WAV to ZSOUND.py:463` |
| 配置文件三键：`zaudio_z64rom_mode=false`、`zaudio_zz_naming=false`、`zaudio_def_dnd_fmt="bin"` | `z64audio.toml` |
| `zaudio_zz_naming` 的注释原文：`more suitable for Jared's zzrtl script`，并区分 `nameBook.bin` 与 `name_predictors.bin` | `z64audio.toml` |

**结论**：`z64audio` 与 zzrtl 生态是**同一工作流的两端**（注释直接点名 zzrtl），
命名表产物（`nameBook.bin` / `name_predictors.bin`）是与 zzromtool/zzrtl 对接的实际契约。

---

## 3.2 `z64audio.exe` 黑箱身份（F，REA）

| 属性 | 值 | 证据 |
|---|---|---|
| 大小 | **2,757,038 B** | `stat` |
| SHA-256 | `3d173b43b807a7205c290149c92915fd5c6aedfbbe72ba92832c45b6df4965bf` | `sha256sum` |
| 格式 | **PE32 / machine 332 (i386) / 32-bit** | `rea inspect-artifact` |
| 导入 DLL（6 个） | `GDI32`、`KERNEL32`、`msvcrt`、`OPENGL32`、`SHELL32`、`USER32` | `rea inspect-pe-imports` |
| imphash (md5) | `0a0da823eab2ae5675eeaa71d57ae838` | 同上 |
| Authenticode | **无**：`IMAGE_DIRECTORY_ENTRY_SECURITY` 缺失，`certificates: 0`，`table: null` | `rea inspect-pe-certificates`（`ev_9384ccd5…6813`） |
| 加固/可靠性 | 无 CFG 等现代缓解项记录（32-bit MinGW 产物） | `rea inspect-binary-hardening`（`ev_cf88f1b7…11e1`） |
| 发布面对账 | 15 项检查，`javascript.source_map_sources` 等 = `not_observed` | `rea audit-release-exposure` |

### 3.2.1 构建环境足迹（F，从二进制内嵌路径恢复）

```
/home/rankaisija/mxe/tmp-gcc-i686-w64-mingw32.static/gcc-11.3.0.build_/i686-w64-mingw32.static/libgcc
/home/rankaisija/mxe/usr/i686-w64-mingw32.static/include/psdk_inc
GCC: (GNU) 11.3.0
mingw_vfprintf.c / mingw_pformat.c / mingw_helpers.c / mingw_get_invalid_parameter_handler
```

⇒ **静态判定**：该 exe 由 Linux 上的 **MXE** 交叉工具链（`i686-w64-mingw32.static`，GCC 11.3.0，
静态链接 MinGW-w64 运行时）构建，构建机用户目录名 `rankaisija`。
这解释了 32-bit、`msvcrt` 直连、无任何签名目录、以及为什么仓库里没有对应的构建脚本 —— 它是
**开发者本机产物**，非 CI 产物。

> 注：字符串里同时存在 `Microsoft Time-Stamp PCA 2010` / `Thales TSS ESN:F87A-E374-D7B9` 字样，
> 但证书目录不存在 ⇒ 这些是链接期残留/模板文本，**不能**当作"已签名"证据（记 `?`）。

### 3.2.2 数据格式接口（F，字符串恢复 + 源码交叉）

从只读字符串抽取（33,737 个可打印片段 / 789 个关键词命中）恢复出的**输入/输出契约**：

- 采样容器：`RIFF____WAVE`、`FORM____AIFF` ⇒ 接受 WAV 与 AIFF。
- 另含 `FORMAT=32-bit_rle_rgbe` ⇒ 内嵌 Radiance HDR 读取器（属更大库的一部分，说明该二进制集成了较多库代码）。
- 支持的采样数值格式（逐条原文）：
  `8-bit Unsigned Integer` / `16-bit Signed Integer` / `24-bit Signed Integer (Tightly Packed)` /
  `32-bit Signed Integer` / `32-bit IEEE Floating Point` ⇒ **解码器可接受的输入位深/编码枚举**。
- 产物命名：`.book.bin`、`.loopbook.bin`、`.vadpcm.bin`、`/Sample.wav`；
  以及模板串 `.book      = &%sBook,` 与 `.book        = {`
  ⇒ 输出既有二进制音源库，也有**文本/C 结构导出**（`%s` 格式化 + 大括号写法）。
- 配置键在二进制内可见：`zaudio_def_dnd_fmt`、`zaudio_z…`（`zaudio_z64rom_mode` / `zaudio_zz_naming` 的可打印片段）
  ⇒ **TOML 由该 exe 自身解析**（不是 Python 侧解析后传参）。
- GUI/图形侧残留：`#define USE_UNIFORMBUFFER 1`、`A requested format is not supported on this device`（GLU 错误串）
  ⇒ 与 `OPENGL32` 导入一致，程序带 OpenGL 窗口（拖放导入界面，对应 `zaudio_def_dnd_fmt` 的 "dnd" = drag-and-drop）。

### 3.2.3 未确定项（U / ?）

- **U**：命令行开关全集。字符串里形似开关的片段（`-bitstob`、`-memcpy`、`-pre_c_init`、`-float`）
  多数是链接期符号残片，**不足以**断言 CLI 语法；只有 `-i/-o` 有源码证据（`WAV to ZSOUND.py:451-458`）。
- **U**：`zaudio_z64rom_mode` 与 `zaudio_zz_naming` 的**语义分支**（打开/关闭分别产生什么字节差异）。
  要确定需要真实 ROM 输入 + 运行，或反汇编 DSP 例程 —— 均超出本轮授权。
- **U**：APCM/VADPCM 编码器的**逐函数实现**。本笔记只做接口还原（输入格式枚举、输出文件族、配置键），
  **不做 DSP 复刻**：仓库自带 Python/文档侧已给出可用契约，反汇编 32-bit 静态 GCC 产物去重建 DSP
  既不可靠（无样本验证）也不是本轮必要工作。
- **?**：二进制"开源仓库 + 无源码黑箱"这一状态本身 —— 仓库以 GPL-3 声明，但夹带一个**未提供源码/构建脚本**
  的 2.7 MB 可执行文件。按 GPL 口径这是交付缺口（缺对应源码），值得在工单里记一笔。

---

## 3.3 `zzrtl`（`audio-model/zzrtl`，只读源码还原）

### 3.3.1 项目性质（F）

`README.md:1-6`：zzrtl 是 **OoT / MM 文件系统管理工具**，"Filesystems are dumped and injected
by way of the highly customizable scripts it interprets. It has built-in compression features,
and projects built to my specification work on both original Nintendo 64 hardware and Wii Virtual Console."
⇒ 它不是"模型编辑器"，而是**文件系统级的 dump/inject 引擎**；自定义模型是通过
"注入新 DMA + 重写表项"落进 ROM 的。

`README.md` 尾段（Showcase 之后）明确指向 **`jaredemjohnson/zzrtl-audio` fork**，
"adds support for sound, music, and text editing" ⇒ 与 §3.1 里 `z64audio.toml` 注释
`more suitable for Jared's zzrtl script` **互为印证**：z64audio 输出即喂给该 fork。

**C 语言工程，仓库不含任何预编译二进制**（仅 `src/` + `script/` + `Makefile`）。
⇒ REA 的 `inspect-*` 家族无适用输入；本轮按"不编译第三方代码"的自设边界，
只做**源码级接口还原**，故本节整体记 **F（源码可指行）/ R（未与真实 ROM 样本对账）**。

### 3.3.2 脚本引擎：内嵌极简 C 解释器（F，`MANUAL.md` "rtl script specification"）

原文："zzrtl uses a very minimalistic C implementation. It is based on xc:
`https://github.com/lotabout/write-a-C-interpreter`"（本地对应 `src/xc.c`，66,121 B）。

被明确禁用的 C 特性（逐条原文）：浮点、预处理器、`for`、`goto`、`do-while`、`break`、
`+=/-=`、无符号类型、所有变量必须声明在函数开头、数组、声明时赋值、`switch-case`、
函数原型；`struct` 关键字**保留给内置类型**。

宿主暴露给脚本的函数族（即插件式 API 面）：
- libc 透传：`printf sprintf sscanf malloc free fopen fprintf fclose memset memcmp strcmp strcasecmp strcat system`（多数加了错误检查）。
- 通用：`die`、`get32/get24/get16/get8`、`put32/put24/put16/put8`、`u8/s8/u16/s16`、`u32op(a,op,b)`。
- ROM 面（`src/zzrtl.h:165-238` 逐项声明）：`rom_new`、`rom_free`、`rom_flag`、`rom_raw`、`rom_save`、
  `rom_align`、`rom_compress(rom, enc, mb)`、`rom_seek`、`rom_seek_cur`、`rom_tell`、`rom_size`、
  `rom_inject_prefix`、`rom_inject_mode`、`rom_inject(rom, fn, comp)`、`rom_inject_png(…)`、
  `rom_inject_raw`、`rom_inject_raw_dma`。
- 目录/路径抽象：`dir_exists`、`dir_enter`、`dir_enter_file`、`dir_leave`、`dir_mkname`、`dir_use_style`（`src/zzrtl.h:126-157`）。
- 数据装载与 TSV：`loadfile(fn, &sz, optional)`、`tsv_col_row(tsv, colname, row)`（`src/zzrtl.h:48-52`）
  ⇒ 名字表就是 TSV：本地 `script/oot_names.tsv`（26,332 B）、`script/mm_names.tsv`（31,603 B）。

### 3.3.3 压缩后端（F）

`src/zzrtl.c:2587-2609`（与 `:4185-4203` 为同一分支的第二处）：按字符串选择编码器 ——

| `enc` 取值 | 后端 | 位置 | 状态 |
|---|---|---|---|
| `yaz` / `slowyaz` | 自带 `src/enc/yaz.c`、`src/enc/yar.c` | `:2587` | 在用（注释称 slowyaz "no application anymore"，`:2593`） |
| `lzo` | `src/enc/lzo/`（LZO 1X，含 `lzo1x_9x.c` 高压缩档） | `:2595-2597` | 在用 |
| `ucl` | `src/enc/ucl/` | `:2599-2601` | 在用 |
| `aplib` | `src/enc/apultra/`（`shrink.c` 75 KB、`divsufsort`） | `:2603` | 在用 |
| `zx7` | `src/enc/zx7.c` | `:2605`（被注释掉） | **未启用** |

⇒ 一个 ROM 文件条目可在 4 种 N64 可行压缩方案间切换；这是"高清纹理包能否装进 ROM"的实际约束来源。

### 3.3.4 云补丁（cloudpatch）格式：完全还原（F，最高价值）

入口：`src/main_cloudpatch.c:10-18` —— `in = argv[1]`、`out = argv[2]`，其后每个参数都是一次
`rom_cloudpatch(rom, 0, argv[i])`。实现体 `src/zzrtl.c:4289+`。逐行语义（读源码即可完全确定）：

1. 校验 rom 已初始化，否则 `die_i("cannot cloudpatch uninitialized rom")`；
2. `ofs < 0 || ofs >= rom->data_sz` ⇒ `die_i("invalid cloudpatch offset %d (0x%X)")`；
3. `file_load(fn, &sz, 1)` 读补丁文本；
4. `dst = rom->data + ofs`；
5. 逐行：行首必须是 `0x`（`memcmp(str,"0x",2)`），`sscanf(str,"%X",&cloudOfs)` 取**相对偏移**；
6. 走到第一个 `,`（`strchr`），越过逗号；
7. 只要还有成对十六进制字符就 `sscanf(ok,"%02X",&x)` 并 `dst[cloudOfs] = x`，**偏移逐字节自增**。

⇒ **补丁文件格式**：`0x<相对偏移(hex)>,<连续十六进制字节串>`，一行一处，纯文本，无校验、无长度字段
（越界靠 `dst` 无边界检查 ⇒ 长补丁可越写；这是可报告的实现风险，非本轮修复项）。
云补丁的意义：**不重定位文件系统、只原地改写若干字节**即可把自定义内容"挂"进 ROM ——
这正是"自定义模型导入"在不破坏 N64 虚拟控制台兼容性的前提下能落地的机制。

### 3.3.5 纹理转换接口（F）

`src/n64texconv.c` 以 `(color family × bit depth)` 二维表组织转换函数：
`n64_colorfunc_array[]`（`:296-`）与 `n64_colorfunc_array_to[]`（`:341-`）。
已实现的格式名：`onebit`、`rgba5551`、`rgba8888`、`ci4`、`ci8`、`i4`、`i8`、`ia4`、`ia8`、`ia16`
（`N64_COLOR_FUNC_NAME*` 全集，逐一 grep 得出）。

细节（可核验的取值规则）：
- `i4`：4 位灰度直接扩到 4 通道（源码注释 `/* TODO: * 0.5 fixes kokiri path */` ⇒ **作者自认此处亮度有偏差**，记 `?`）。
- `ia4`：`x = CONV_7(*b >> 1)`，`w = ((*b) & 1) * 255` ⇒ 3 位强度 + 1 位 alpha。
- `ia8`：`x = CONV_15(*b >> 4)`，`w = CONV_15(*b & 0xF)` ⇒ 4+4。
- `ci4/ci8` 反向转换需要调色板，否则 `n64texconv.c:745` 返回 `"ci format but no palette provided"`。
- 未支持组合一律填 `0`（空槽），统一报 `invalid format`（`:659/:720`）。

### 3.3.6 zzrtl 侧未确定项

- **U**：`rom_inject_mode` 的 mode 取值全集与各自的 DMA 表重排策略（需读 `zzrtl.c` 全 106 KB 才可能定，本轮只取接口）。
- **U**：`zzrtl-audio` fork 的音频注入接口（外部仓库，**本轮未克隆**）。
- **?**：`i4` 亮度 TODO（若上游有误用，会体现在实际纹理上）。
- **R**：全部结论来自源码静态阅读，**未编译、未运行、未用真实 ROM 对账**。
