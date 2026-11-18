# 1. N64 / 3DS / GC / Switch 存档转换器与 SRAM 工具

> 分级见 `README.md`。目标目录：`oot-peripherals/round2-audio-save-modtools/save/`（**24 个仓库 + `binaries/`**，
> 其中 8 个为本笔记 §1.0 首批深挖对象，其余 16 个见 §1.0-b；跨仓库的校验和/容器结论汇总在 §6）。
> 全部结论来自**开源源码静态阅读**（`path:line` 可指）；未下载任何真实存档样本，未运行上游代码。

---

## 1.0 本轮采集清单（F）

| 仓库 | 语言/形态 | 最新提交 | 定位 |
|---|---|---|---|
| `ZeldaSaveTool` | C#（WinForms，多目标框架 .NET 2.0→） | `0b8e398` 2026-07-24 "Try to fix GCI conversion again" | **OoT PC Port 存档转换器**，全格式枢纽 |
| `save-file-converter` | JavaScript（浏览器端，euan-forrester） | `0a9786f` 2026-06-11 | 多平台 Web 转换器（含 N64 DexDrive / MemPak） |
| `N64-Save-Converter` | C（单文件 `convertsave.c`，224 行） | `f4a9921` 2022-04-30 | **BizHawk `.SaveRAM` 容器生成器** |
| `n64-sram-converter` | Python | `8686fb1` 2025-12-14 | DreamDumper64 → SummerCart64 实卡互转 |
| `N64-Ocarina-of-Time-Save-Converter-Script-for-Mac-and-Windows` | Python ×3 | `faf875e` 2025-10-12 | `swap_eep/sra/srm.py` 三型字节序互换 |
| `OOT-Save-Converter` | 纯客户端 JS（`index.html` + `oot.js`） | `37a56d1` 2018-08-03 | **可直接吃原始 N64 RAM dump** 的网页编辑器 |
| `n64SaveConverter` | Android/Kotlin（Gradle） | `4c79fa1` 2023-01-08 | **已废弃**，README 指向 `Ninjiteu/N64SaveConverter` |
| `N64SaveConverter` | （`N64SaveConverterGui` + Readme.txt） | `165f977` 2023-02-20 | 同上作者的 GUI 版 |

---

## 1.0-b 首批之外的 16 个仓库（F，逐仓一句话定位 + 关键接口）

| 仓库 | 语言/许可 | 定位与一手接口 |
|---|---|---|
| `save-file-converter` | JS / GPL-3.0（243★） | 本轮**最有价值**的多平台 Web 转换器；N64 侧见 §1.1、§6.3；`Wii/ConvertFrom/ConvertFromN64.js:36,38` 按大小分派 `.sra`/`.fla`，`:46` 对 Wii-VC 载荷做 `N64Util.endianSwap`；`NintendoSwitchOnline/N64.js:1,8` 注释明确 **NSO 的 N64 SRAM/FlashRAM 字节序与常规相反，EEPROM 不交换**（F，独立第三条字节序证据） |
| `N64SaveConverter`（Ninjiteu，Java） | 无许可证 | 容量矩阵第二来源（§6.4） |
| `n64SaveConverter`（ChronoAndross，Android） | MIT | 同矩阵 + 目录批处理（`N64SaveConverter.java:19-31,33,65-70,80,157`） |
| `saveswap`（imaandrew） | MIT | `saveswap.py:10-11` 复刻 saturnu 的 ED64-Saveswap；`:70` `VALID_SIZES=[512,2048,32768,131072]`、`:123` `calculate_padding`、`:150` `assert_stride`、`:157` `byteswap(path, swap_bytes, swap_words, pad_to)` + `.bak`、`:328-336` `--swap-mode/--force-padding/--no-backup` ⇒ **`swap_bytes`/`swap_words` 双开关正是 §1.5.1 的 V64/U64 两种置换**（F） |
| `mempak`（bryc，JS，92★） | 无许可证 | `js/init.js:11` `RawMPK=32768, DexDrive=36928, MaxMPKCmt=98144, M64PNX=296960, MultiMax=2097152`；`js/parser.js:166,169,212` `initMPK(banks)` + IndexTable/备份校验和；`:267-278` `.note` 导入按游戏 serial + 起始索引匹配、`0x100` 块拷贝；`js/codedb.js:7,523` N64 游戏码表；`js/app.js:194` 空闲块算术 `banks*0x200-0x200` ⇒ **DexDrive = 32768 + 4160 = 36928**，与 §6.3 吻合（F） |
| `segher-wii-tools` | 文件级 GPL-2.0 | `tachtig.c:69,109,158` GCI 头解码 / `###banner###.ppm`(192×64) 导出；`twintig.c:108,25` 写 GCI 头（`wbe64(header, title_id)`）；`zelda-cksum.c:11-22,34-40` ⇒ **GC 侧塞尔达校验和算法唯一一手来源**（§6.2） |
| `Admentus64-N64-Save-Converter` | GPL-3.0（PowerShell） | `:415-416` MM：`ZELDA2`(`5A 45 4C 44 41 32`) → 在 `0x6000` 后找 `ZELDA3`，偏移 `-36`、长度 `0x20300`；`:422-423` OoT：先找 `ZELDA`(`5A 45 4C 44 41`)，再找 `21 5A 45 4C 44 41`，偏移 `-6`、长度 `0x8000`；`:431-436` 按长度推 `.eep/.fla/.sra`；`:438-445` 4 字节字交换 |
| `N64-OoT-Save-Converter-Script-for-Mac-and-Windows` | 无 | §1.3 三脚本同源 |
| `n64-sram-converter` | 无 | §1.4 实卡链路 |
| `OOT-Save-Converter`（bkacjios） | 无 | 见 §1.6 + §6.1；额外：`:33-37` 原始 RAM dump 的**按版本 ROM 地址表**（NTSC1.0 `0x11A5D0` … PAL1.1 `0x118410`）、`:174 importSaveFromRAM(data, version, …)`、`:125-130,179-186` 16-bit 校验和 @`0x1352` + 槽字节 `0x1354` |
| `oot-sw2-save`（ZhaowTao）/ `oot26-save-editor` / `ezdalzotto…All-Save-Editor` / `OoT-Notas` / `oot-editor` | Python / Python / HTML / Python / Swift | Switch 2 remake 容器族（murmur3 字段名，§6.2 末行）；`oot-editor/OoTEditor.swift:182-186` **写前抢占 `TOTK/game-slot/lock`**（`O_RDWR|O_CREAT|O_NOFOLLOW`）⇒ 唯一带并发写保护的编辑器；`SaveApply.swift:9-15` 要求 `slot_0[0-5]/progress.sav` 并配对 `caption.sav` |
| `mm3d-save-editor` / `mm3d-heart-tracker` | Go / Python | 3DS MM3D：`save/save.go:25-53` rupees `0x140`、bank `0x1230`、name `0x128`、statue `0x14C`；`:138-170` `fixChecksum` 先清 `0x1A88/0x1A89` 再写 `0D 44`（?= 硬编码常量，语义 U）；`mm3d_save.py:32` `SAVE_SIZE = 0x13618`、`:35-41` name UTF-16LE `0x128`、心之上限 `0x13A`、collect `0x1F8`、场景旗标 `0x250` 步长 `0x20`、`:58` `crc16(seed 0x1017)` ⇒ **MM3D 存档 0x13618 B，与 OoT3D 的 `CTRSaveSize=0x14DC` 不同族**（F） |
| `mm-recomp-save-editor` | C / CC0-1.0 | 游戏内改档 mod（非文件工具）：`src/save_editor.c:415-421` 直接读 `gSaveContext.save.saveInfo.playerData.magicLevel/isMagicAcquired` |
| `zelda64`（jgazeley） | Python | `zelda64.py:8-12` `Save(file, name, endian, game, address, valid, slot, bound)`；`:39-52,60-65` PAL/NTSC 字符集 ↔ ASCII 双向表；`:121-136` **小端字母重排 + 非对称校验地址**；`:181-201` 16-bit BE 校验和 ⇒ 改文件名必须同步改校验和，这是 §6.2 之外的第二条"改档即坏档"路径（F） |
| `zs-flash`（cxd4） | C / CC0-1.0 | MM FlashRAM：`main.c:19,32,39` `load_flash/save_flash`、section = `(file - flash_RAM)/FILE_SIZE`；`zs_data.c:825-848` `fix_checksum`（字节和、**排除校验字段自身**、16-bit 回写）；`flash_io.h:24-27` `FLASH_SIZE/FILE_SIZE/NUMBER_OF_DATA_FILES` |
| `oot_bitset`（jb55） | C + Rust | `oot_bitset.h:78,126,136` `bitset_word/bitset_set/bitset_clear` 与 `src/lib.rs:12-59` 对偶实现；README 引 `zeldaret/oot include/save.h` ⇒ 16-bit 旗标语义的**最小可复用参考实现**（F） |
| `2Ship2PJ64-SaveConverter` | Python | MM JSON→Project64 FlashRAM：`convert.py:120-126` 16-bit 字节和、`:128-130` `word_swap`、`:189-198` 用 `OPTIONS_ID`/`ZELDA3` 自动判别大端/PC 端、`:224-248 build_slot`、`:251-271 write_flash(..., bigendian_copy)` |

**`ZeldaSaveTool` 补两条硬事实**（F，`ZeldaSaveTool/ZeldaSaveTool/Save/File.cs`）：

- `:8` `enum Format { N64Emu, N64Console, PcPortSav, CTRSave, SohJson }`；`:87` `CTRSaveSize = 0x14DC`（5340 B）；
  `:91-93` 自动判别的**最后一级**是 `input[0x87] == 0 ? PcPortSav : N64Emu` —— 单字节判别，易被误改（?）。
- `:79-84` GCI 常量：目录头 `0x40`、块 `0x2000`、载荷偏移 `0x6044`，且注释原文
  `0x20 (global header) + 3 * 0x1450 (slots) + 0x4 (skip between slot 2 and 3)` ⇒
  **第三个独立来源**确认 `0x20 + 3×0x1450` 布局（§6.1），并新出一条"N64→Wii-VC GCI 打包时槽 2/3 之间多插 4 字节"的规则（F）。
- `ToolForm.cs:34` 导出格式下拉**只列** `N64Emu / N64Console / PcPortSav` ⇒ **N64→3DS 写入方向不存在**（U，已检索确认）。

---

## 1.1 N64 存档介质容量表（F，两处独立印证）

`save-file-converter/frontend/src/save-formats/PlatformSaveSizes.js`（`n64:` 段）：

```
n64: [ 512, 2048, 32768, 131072, 786432 ]   // 注释：From http://micro-64.com/database/gamesave.shtml
                                             // 786432 行尾注：Dezaemon 3D
```

- 512 = EEPROM 4 Kbit；2048 = EEPROM 16 Kbit；**32768 = SRAM 256 Kbit（OoT 用的就是这一型）**；
  131072 = FlashRAM 1 Mbit；786432 = 768 KiB（Dezaemon 3D 特例）。
- `N64/Mempack.js:49` `CART_SAVE_SIZES = [512, 2048]`，注释（`:42-48`）给出**为何只有这两种**：
  作弊设备把卡带存档拷进 Controller Pak 时，**控制卡前 5 页被系统信息占用**，
  32768 放不下 ⇒ 只 pad 到 512/2048。这条注释是"为什么 OoT SRAM 不能进 Memory Pack"的硬答案（F）。

`N64/Mempack.js:33-38` 页结构常量（`NUM_*` 取自同目录 `Components/Basics.js`）：

| 常量 | 值 | 位置 |
|---|---|---|
| `NUM_NOTES` | **16**（注释："A 'note' is a save slot. It consists of >= 1 pages"） | `Basics.js:6` |
| `NUM_PAGES` | **128** | `Basics.js:8` |
| `PAGE_SIZE` | **256** | `Basics.js:10` |
| `TOTAL_MEMPACK_SIZE` | `NUM_PAGES * PAGE_SIZE` = **32,768 B**（256 Kbit，即标准 Controller Pak 容量） | `Basics.js:12` |
| `FIRST_SAVE_DATA_PAGE` | **5** | `Basics.js:14` |
| `MAX_DATA_SIZE` | `(128 - 5) * 256` = **31,488 B** | `Mempack.js:38` |

| 页 | 用途（`Mempack.js:33-36`） |
|---|---|
| 0 | `ID_AREA_PAGE` |
| 1 | `INODE_TABLE_PAGE` |
| 2 | `INODE_TABLE_BACKUP_PAGE`（**page 1 的完整副本**，注释明说用于 page 1 损坏时校验） |
| 3、4 | `NOTE_TABLE_PAGES` |
| 5..127 | 存档数据（`InodeTable.js:43/57/88` 三处循环都从 `FIRST_SAVE_DATA_PAGE` 起、到 `NUM_PAGES` 止） |

⇒ §1.1 里 `CART_SAVE_SIZES = [512, 2048]` 的**算术理由至此闭合**（F）：
可用数据区只有 **31,488 B**，而 32,768 B 的 SRAM 卡带存档**装不进** `31,488`；
512 与 2048 能装 ⇒ 只有这两种会被 pad。这与 `:42-48` 的注释完全一致，不再是推测。

`N64/DexDrive.js:24-31`（GC 记忆棒互转，即"GC 侧"接口）：

| 常量 | 值 |
|---|---|
| `HEADER_LENGTH` | **4160** |
| `HEADER_MAGIC` | **`123-456-STD`**（US-ASCII） |
| `FIRST_COMMENT_OFFSET` | 64 |
| `COMMENT_LENGTH` | 256，`getCommentStartOffset(i) = 64 + i*256`（`:33-35`），共 `NUM_NOTES = 16` 条 |
| 空头部填充 | `Util.getFilledArrayBuffer(HEADER_LENGTH, 0x00)`（`:112`） |

⇒ **头部等式（F）**：`4160 = FIRST_COMMENT_OFFSET 64 + NUM_NOTES 16 × COMMENT_LENGTH 256`，
即 DexDrive 头 = 64 B magic/杂项 + 16 条 256 B 槽位注释 —— 与 Controller Pak 的 16 个 note 一一对应，
所以"DexDrive → Mempack"转换时每条注释就是一个存档槽的用户说明（F）。
这一路径让 OoT 存档能经 GC Memory Dump 与 N64 Controller Pak 三方互转。

---

## 1.2 BizHawk `.SaveRAM` 容器：**逐字节完全还原**（F，`N64-Save-Converter/convertsave.c`）

工具语义（`:174-224`）：`convertsave -i <save> -o <dir>`，或 `-d -i <indir>` 走 `ftw()` 全目录；
`-h` 打印用法。每个输入产出 **3 份**（`:106-123`）：

| 输出 | 内容 | 大小 |
|---|---|---|
| `Project64/<name>.fla` | 字节交换后的原始存档 | `fsize` |
| `m64p/<name>` | 同上 | `fsize` |
| `BizHawk/<name>.SaveRAM` | 下面这个固定容器 | **恒定 `0x48800` = 294,912 B** |

容器构造（`create_save_bizhawk`，`:29-61`）：

1. `memset(bizhawk, 0, 0x800)` ⇒ **前 2 KiB 全 0 头部**；
2. 对 `i ∈ {0x800, 0x8800, 0x10800, 0x18800}`（`for (i=0x800; i<0x20000; i+=0x8000)`）的 **4 个 32 KiB 块**：
   - 先把块内**每个偶数字节写 0、每个奇数字节写 0x03**（`:52-55`）；
   - 再用 `init[]` 的**前 272 字节**覆盖块首（`:56` `memcpy(&bizhawk[i], init, 272)`）；
3. `memset(bizhawk + 0x20800, 0xff, 0x20000)` ⇒ **0x20800 起 128 KiB 全 0xFF**（FlashRAM 区）；
4. `memset(bizhawk + 0x40800, 0, 0x8000)` ⇒ 尾部 **32 KiB 全 0**；
5. `memcpy(bizhawk + (0x800 + 4*0x8000), buf, len)` ⇒ **真实存档落在偏移 `0x20800`**（第 3 步的 0xFF 区内）。

⇒ 结构式：`2 KiB 头 + 4×(32 KiB「00/03 交错 + 272 B init」) + 128 KiB 数据区 + 32 KiB 零 = 0x48800`。

`init[]`（272 B，`:30-48`）逐字面量：

```
0x81,0x01..0x1f            // 前 32 字节：0x81 后跟 0x01..0x1f 递增序列
随后 4 组 40 字节记录，每组形如：
  ff ff ff ff | 05 1a 5f 13 | 00 00 00 00 | 00 00 00 00
  ff ff ff ff ff ff ff ff | ff ff 01 ff | 66 25 99 cd
  （其中两组后接 32 字节 00）
尾 16 字节：00 71 00 03 | 00 03 ×7
```

- **F**：`0x051A5F13` 与 `0x662599CD` 在 272 B 内各出现 4 次，`0x0003` 填充与第 2 步的"奇数字节=0x03"呼应
  ⇒ 这是 **EEPROM 8 字节记录行（4 B 数据 + 4 B 控制）在容器里的镜像写法**。
- **U**：仓库与 README 均未解释这两个常量的来源（BizHawk 侧的 EEPROM/FlashRAM 索引头），
  本笔记**不**为它们编造语义；需读 BizHawk 的 N64 存档实现才能定（本轮未做）。

**输入前置变换**（`:17-27`、`:160`）：`byteswap()` 对整文件做 **4 字节全逆序**
（`buf[x]<->buf[x+3]`、`buf[x+1]<->buf[x+2]`），即大端存档 → 小端。
⇒ 该工具的**输入假设**是"原始 N64（大端）SRAM dump"；对长度非 4 倍数的输入会漏尾字节（无校验）。

**三处实现缺陷（F，源码可指）**：
1. `:106-107` 先对 `name` 做 `strcat(name,".fla")`，随后 `sprintf(m64p_file,…,name)` **复用已被改写的 `name`**
   ⇒ **m64p 输出文件名带上了 `.fla`**（与 Project64 同名），非有意行为。
2. `:138-139` `dot = strchr(fname,'.')` 后直接 `fname[dot-fname]='\0'` ⇒ **文件名无 `.` 时 NULL 参与运算**（崩溃）。
3. `:60` 的 `memcpy` 不校验 `len ≤ 0x20000`（亦不校验 ≤ 容器余量）⇒ **输入 >128 KiB 会越界写全局数组 `bizhawk`**（BSS 溢出）。
   本轮按防御性视角记录该缺陷，**不**给出利用路径。

**交叉印证（F）**：`ZeldaSaveTool/ZeldaSaveTool/ToolForm.cs:33` 的支持列表里同样出现 `.SaveRAM`
⇒ 两个互不依赖的项目使用同一容器名，说明 **BizHawk `.SaveRAM`（294,912 B）是社区通行格式**，
而非该单文件 C 工具的私有发明。

---

## 1.3 三种 N64 存档字节序互换脚本（F）

`N64-Ocarina-of-Time-Save-Converter-Script-for-Mac-and-Windows/Python Script/`
下 `swap_eep.py` / `swap_sra.py` / `swap_srm.py`：

- 三者**算法完全一致**，只有默认输出文件名不同（`save_swapped.eep/.sra/.srm`）；
- 核心（各文件 `:8-13`）：`for i in range(0, len(data), 4): word = data[i:i+4]; swapped.extend(word[::-1]) if len(word)==4 else swapped.extend(word)`
  ⇒ **一律 32 位全逆序**，尾部不足 4 字节原样保留。

⇒ 结论（F）：该仓库把 EEPROM/SRAM/FlashRAM **当作同一种"按字翻转"问题**处理；
而 EEPROM 在卡带上是 8 字节记录（见 §1.2 的 `0x03` 控制字节），
"4 字节全逆序"对 8 字节记录**恰好是每记录两字各自逆序**，是否等价于模拟器期望的 EEPROM 布局，
本仓库未论证 ⇒ 记 **U**（正确性未证实），但其与 `convertsave.c:17-27` 的变换**完全相同**（F）。

---

## 1.4 实卡链路：DreamDumper64 → SummerCart64（F，`n64-sram-converter`）

`n64_sram_converter.py`（文件头 docstring `:3-4`、`:27-33`）+ README：

| 事实 | 值 | 位置 |
|---|---|---|
| 适用存档 | 256 Kbit（**32,768 B**）SRAM 游戏 | `:4` |
| DreamDumper64 输出 | **恒 131,072 B（128 KB）**，不论实际存档型 | `:29`、`:53-55`（不等于 131072 就警告"可能不是 DreamDumper64 dump"） |
| 取用规则 | **取前 32 KB**（`sram_size = 0x8000`，`save_data = data[0:0x8000]`） | `:61-65` |
| 是否换序 | **不做任何字节交换**（"Preserves raw cartridge format (no byte swapping)"） | `:162` |
| SummerCart64 命名要求 | 存档名必须 = ROM 文件名 + `.sav`（例：`Legend of Zelda, The - Ocarina of Time (USA).sav`） | README `:79-80` |
| 游戏识别 | `detect_game_from_save(data)`：在前 1 KB 内找标记 | `:11-23` |
| **OoT 标记** | **`b'ZELDAZ'` → "The Legend of Zelda: Ocarina of Time"** | `:15` |
| Mario Golf 64 标记 | `b'\x12\x34\x56\x78'`（注释 "Confirmed: starts with these bytes"） | `:16` |
| 实测范围 | 仅 2 张自购卡：**OoT (PAL) ✓**、Mario Golf (JP) ✓；在 Analogue3D 上验证 | README `:5`、`:21-28` |
| 作者自述工具链 | README `:11`：**"This script was created with Claude (Anthropic) based on reverse-engineering DreamDumper64 save file formats and testing with actual N64 cartridges."** | README `:11` |
| 已知限制 | 作者的 DreamDumper64 **只暴露 SRAM 型**，其它存档型未实现（疑固件/硬件问题） | README `:38` |

⇒ **`ZELDAZ` 是 OoT 存档里可直接用于识别的 ASCII 标记，出现在前 1 KB 内**（F：源码字面量）。
⇒ 与 §1.2/§1.3 的对照结论很清晰：**"要不要换序"取决于目标是模拟器（小端）还是实卡/烧录卡（原始大端）**，
这四份工具恰好覆盖了两条互斥的选择，且**没有任何一份同时做两件事**。

---

## 1.5 枢纽：ZeldaSaveTool = OoT PC Port 转换器（F，C#）

- 支持输入（`ToolForm.cs:33`，逐字）：
  **`.sra, .sav, .gci, .srm, .SaveRAM, .v2.sav, .st*, .m64p, .zip, .pj.zip, .pj, .bin`** ⇒ 13 类
  （`.v2.sav` = PC Port 第二版；`.st*` = Switch Online 的 STG/STAT 存档槽；`.gci` = 3DS/GC 图形容器接口格式；
  `.bin` = 其它原始导出）
- 3DS 侧内部存档文件名常量（`ToolForm.cs:170`）：`const string sraFn = "THE LEGEND OF ZELDA";`
  ⇒ 与第一轮 Ocarina-Atlas 的 OoT3D 标题 `program id 0400486024DEC000`（`OcarinaAtlas.cs:1222`）同一条链路：
  3DS 存档在系统内以该名称为键（F，跨轮印证）。
- 多语言资源（`Properties/Resources.Designer.cs`）：德/西/法/**中**（`:251-255` 提供中文筛选器文案）
  ⇒ 中文界面已内置，本项目对中文化可直接复用其资源串。
- 兼容层 `Compat.cs`：为 **.NET 2.0/3.0/3.5** 手工补 `ExtensionAttribute`、`IsNullOrWhiteSpace`
  与 `SplitContainer` 子类 ⇒ 该 GUI 追求"最老框架也能跑"，说明它是**离线单机小工具**、无 NuGet 依赖。

### 1.5.1 四族 ROM/存档字节序的形式定义（F，最高复用价值）

`ID/ByteOrder.cs:4-45`：

```
enum Type { Unknown = -1, BigEndian /*Z64*/, LittleEndian /*N64*/, ByteSwapped /*V64*/, WordSwapped /*U64*/ }
MoveBytes(data, order):  若 data.Length % 4 != 0 抛 ArgumentException
                         每个 4 字节块按 order 置换
ToBigEndian(from):
  LittleEndian → order {3,2,1,0}   // 整字逆序
  ByteSwapped  → order {1,0,3,2}   // 字内两两交换
  WordSwapped  → order {2,3,0,1}   // 字的前后半交换
  BigEndian    → 不动
```

⇒ **这是 Z64/N64/V64/U64 四种字节序最紧凑的机器可读定义**（三组置换向量 + 长度必须是 4 的倍数）。
它同时解释了 §1.2、§1.3 为什么都写 `{3,2,1,0}`：它们的输入是 "LittleEndian"。
（注：这里注释把 "LittleEndian" 标为 `N64`，即 `.n64`/小端序转储族，与卡带原生 `.z64` 大端相对。）

### 1.5.2 存档结构对照（F）—— N64 与 3D 的 `SaveContext` 字段级差异

两份 C# 结构体（`Pack = 1, LayoutKind.Sequential`）互为镜像，可直接推出转换器的映射表：

| 组件 | N64（`Structs/Oot/Oot.cs`） | OoT3D（`Structs/Oot3D/Oot3D.cs`） | 结论 |
|---|---|---|---|
| `ItemEquips` | `buttonItems[4] + cButtonSlots[3] + padding_07 + ushort equipment`（`:6-11`） | `buttonItems[5] + buttonSlots[4] + padding_09 + ushort equipment`（`:6-11`） | **3D 各多 1 槽**（多了 L 槽/自定义 C 槽） |
| 时间/入口字段位置 | 在 **`Save`** 顶层：`entranceIndex, linkAge, cutsceneIndex, dayTime, padding_0E, nightFlag, totalDays, bgsDayCount`，其后才是 `info`（`:134-144`） | 在 **`SavePlayerData`** 内部：`entranceIndex, linkAge, cutsceneIndex, dayTime, masterQuestFlag, padding_0F, nightFlag, unk_14, unk_18`（`:15-23`） | **重排是转换器的核心工作**（不是简单字节序） |
| `playerName` | `byte[8]` | **`byte[16]` + 注释 "UTF-16, 8 chars"**，另有 `playerNameLength`、`zTargetingSetting`（`:24-26`） | 3D 用 **UTF-16** 名字 ⇒ 转换需字符集重编码（另见 `Charset.cs`） |
| `deaths` / 存档计数 | `ushort deaths`（`:18`） | `ushort deaths; // saveCount in z3Dsave`（`:29`） | 交叉引用第三方 `z3dsave` 命名，**字段语义随平台改名** |
| 生命/魔法 | `healthCapacity(short) … rupees(short) swordHealth(ushort) naviTimer(ushort)`（`:21-27`） | `healthCapacity(ushort) health(short) … rupees bgsHitsLeft(ushort) naviTimer`（`:31-37`） | 3D 用 **`bgsHitsLeft`（ Bombchu  Bowling 剩余命中数）** 取代 N64 的 `ocarinaGameRoundNum`；N64 独有 `n64ddFlag`、`swordHealth` |
| `Inventory.items` | `byte[24]` + `padding_2A(2 B)` + `upgrades/questItems` | **`byte[26]`**，**无 padding**（`:52-56`） | 3D 多 2 个物品槽；`equipment` 两侧都有 |
| `SavedSceneFlags` | 7 个 uint：`chest swch clear collect unk rooms floors`（`:53-62`） | 7 个 uint：`chest swch clear collect unk **rooms1 rooms2**`（`:64-72`） | 同为 7 字，但**第 6/7 字含义不同**（3D 把 rooms 拆两半）⇒ 不能盲拷 |
| `sceneFlags` 数组 | `uint[868]`（**注释：`SavedSceneFlags is 7 uints. 124 * 7 = 868 uints`**，`:110-111`） | `SavedSceneFlags[124]`（`:92`） | 同一 124 场景 × 7 字，表达方式不同（扁平 vs 结构体数组） |
| `FaroresWindData` | `Vec3i pos(3×int) yaw playerParams entranceIndex roomIndex set tempSwchFlags tempCollectFlags`（`:72-81`） | `int[3] pos` + 同后续字段（`:75-84`） | 等价 |
| 金币钩爪/高分 | `gsFlags int[6]`、`highScores int[7]`（`:115-117`） | `gsFlags byte[22]`、`unk_ECA[6]`、**具名** `horsebackArcheryHighscore / horseRaceRecordTime / marathonRaceRecordTime`（各带 8 B 空洞）（`:95-101`） | 3D 把 7 个高分**拆成 3 个具名字段 + 填充**，N64 是紧凑 int[7] ⇒ 必改 |
| 事件表 | `eventChkInf ushort[14]`、`itemGetInf ushort[4]`、`infTable ushort[30]`、`worldMapAreaData uint` | **同名同长度同序**（`:102-106`） | 可直接对拷（仅需换序） |
| 尾部专有 | `scarecrowLongSongSet + scarecrowLongSong[0x360] + unk_12A1[0x24] + scarecrowSpawnSongSet + scarecrowSpawnSong[0x80] + unk_1346[2] + HorseData + Checksum{ushort value}`（`:124-131`） | **`SaveInfo` 到 `worldMapAreaData` 结束**（`:107`） | 稻草人歌、马匹数据、校验和 **在 3D 结构里不存在** ⇒ 该工具不转换它们（记 U：3DS 侧这些字段在别处） |
| 偏移标注 | `unk_E8C`、`unk_EB4`、`unk_F34`、`unk_F3C`、`unk_12A1`、`unk_1346`、`padding_56`、`padding_B6` | `unk_6C`、`unk_EA4`、`unk_ECA`、`unk_ED4`、`unk_EE4`、`unk_F4C`、`padding_EA`、`padding_0F/53` | **字段名里带绝对偏移**（如 `unk_E8C` ⇒ 0xE8C）⇒ 可作为还原时的"官方锚点"，F |
| `Checksum` | 仅 `ushort value`（`:98-100`）——**没有第二块的校验** | 无该结构 | N64 存档校验和只覆盖 `0x00..0xE5` 与 `0x100..`？本轮**未**验证覆盖范围（U） |

⇒ **本节的实际价值**：这是本轮唯一一份"把 N64↔3DS 存档差异写成字段级对照表"的材料，
且全部来自 GPL/开源 C# 结构体 ⇒ 后续任何转换器（含第一轮关注的 3DS/Switch 链）都能以此为准。
仍需真实样本对账（R）。

---

## 1.6 纯客户端网页编辑器 `OOT-Save-Converter`（F）

README：`A simple, pure client based, JavaScript website that will allow you to manage your
Ocarina of Time save files and even import raw N64 ram dumps.`
⇒ 关键接口点是**吃"原始 N64 RAM dump"**（不只吃 `.sra`）——即它能从模拟器内存全量转储里定位存档；
本地文件：`index.html`、`oot.js`、`chiaro.otf`（字体）、`images/`；无任何后端/网络依赖；
最新提交 2018-08-03（"Fixed blurry images in Firefox"）。

## 1.7 未找到 / 边界（U）

- **U**：GC〈→〉N64 的 **.gci 头**逐字节定义在 `ZeldaSaveTool` 里只有筛选器与文件名常量，未见完整头结构体。
- **U**：Switch Online（`.st*`）存档的**加解密**：本仓库只做容器/命名层面（工单里的 "Switch 互转" 到此为边界）。
- **U**：`save-file-converter` 的 3DS/Switch 段（`frontend/src/save-formats/NintendoSwitchOnline/`）与
  `Wii/`、`GameCube/` 的 OoT 相关分支，本轮未展开逐文件核对。
- **?**：`save-file-converter/frontend/src/save-formats/PSP/psp-encryption/psp-encryption.wasm`
  是本轮发现的**唯一 Web 端原生模块**（70,000 B），属 PSP 存档加密，与 OoT 无直接关系；
  `rea inspect-wasm-artifact` 仍缺 WABT（见报告 §方法学）⇒ 段级还原记 U。
