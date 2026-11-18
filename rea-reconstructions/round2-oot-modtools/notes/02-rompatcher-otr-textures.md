# 2. Web 端 RomPatcher.js、OTR/纹理打包周边

> 分级见 `README.md`。目标：`oot-peripherals/round2-audio-save-modtools/patch-otr/`。
> 全部为**开源仓库静态阅读 + REA 取证**；未下载 ROM/补丁样本，未运行上游代码。

---

## 2.1 RomPatcher.js 身份（F）

本地只读快照：`patch-otr/RomPatcher.js/`，**MIT**，作者 Marc Robledo。

| 字段 | 值 | 位置 |
|---|---|---|
| 包名 / 版本 | `rom-patcher` / **3.0.0** | `package.json` |
| bin | `RomPatcher` → `./index.js` | `package.json` |
| 运行时依赖 | 仅 `chalk@4.1.2`、`commander@^11` | `package.json` |
| 自述 | "A ROM patcher made in Javascript. Compatible with IPS, BPS, APS, UPS and more formats." | `package.json.description` |
| 模块数 | 13 个格式模块 + `BinFile.js` + `HashCalculator.js` + `bz2` + `zip` | `rom-patcher-js/modules/` |
| Web Worker 入口 | `RomPatcher.webworker.apply.js` / `.crc.js` / `.create.js` | `rom-patcher-js/` |
| ES 入口 | `RomPatcher.js`（库）、`RomPatcher.webapp.js`（界面） | 同上 |
| Service Worker | `_cache_service_worker.js`（离线缓存） | 仓库根 |

格式模块 → 支持的补丁类型全集（F，目录列举）：
`ips`、`bps`、`ups`、`aps_gba`、**`aps_n64`**、`ppf`、`rup`、`pmsr`、`vcdiff`、`bdf`。
其中 **`aps_n64` 就是 OoT 补丁圈实际使用的格式**（见 §2.3）。

与 OoT 的直接关联（F，`test.js:20-22`）：测试矩阵里 APS 用例明确要求
- Patch 来源：`http://dorando.emuverse.com/projects/eduardo_a2j/zelda-ocarina-of-time.html`（UniPatcher 作者主页，即 APS 格式规范来源）
- ROM：`Legend of Zelda, The - Ocarina of Time (USA).z64 [CRC32=7e107c35]`

⇒ **`7e107c35`（OoT US ROM CRC32）与 APS-N64 校验路径在本仓库内被写死为测试用例**，
这是"Web 端 RomPatcher.js 属 OoT 周边工具"这一工单断言的最硬证据。

`index.js` 的 CLI（Node）与 `webapp/`（浏览器）共用同一 `rom-patcher-js/` 内核；
浏览器侧通过 3 个 Web Worker 分离 apply / crc / create，避免主线程阻塞 ——
这是工单所说"Web 端"的实现方式。

---

## 2.2 APS(N64) 容器：逐字节还原（F，`rom-patcher-js/modules/RomPatcher.format.aps_n64.js`，210 行）

规范出处：文件头注释 `:2` 指向 `https://github.com/btimofeev/UniPatcher/wiki/APS-(N64)`。

### 头部（`APS.fromFile`，`:131-159`）

| 偏移 | 长度 | 字段 | 取值/说明 | 源码 |
|---|---|---|---|---|
| 0 | 5 | magic | 字符串 **`APS10`** | `:4`、`:135`（`seek(5)`） |
| 5 | 1 | headerType | `0x01` = N64 模式（`APS_N64_MODE`） | `:7`、`:136` |
| 6 | 1 | encodingMethod | 原样保存/写出，**本模块不解释其含义** | `:137`、`:79` |
| 7 | 50 | description | 定长字符串，缺省 `'no description'` | `:15`、`:138`、`:80` |
| — | — | **以下仅当 headerType===1** | | `:141-146` |
| 57 | 1 | header.originalN64Format | `.v64` 文件名 ⇒ `0`，否则 `1` | `:169` |
| 58 | 3 | header.cartId | ROM 偏移 **0x3c** 处 3 字节卡带 ID | `:170-171`、校验 `:38-40` |
| 61 | 8 | header.crc | ROM 偏移 **0x10** 起 8 字节（两段 CIC CRC） | `:172-173`、校验 `:42-47` |
| 69 | 5 | header.pad | 写死 `[0,0,0,0,0]` | `:174` |
| 74 | 4 | **header.sizeOutput** | 输出（目标）大小 | `:147` |

- 所有多字节整数按 `patchFile.littleEndian=true` 读取（`:133`）。
- 无 N64 块时头部总长 = `5+1+1+50+4 = 61`；有 N64 块 = `61+17 = 78`（`:63-65` 的算法与此完全一致，可自洽验证）。

### 记录流（`:149-157`，直到 EOF）

每条记录 = `u32 offset` + `u8 length`，其后二选一：

| length | 含义 | 后续字节 | 常量 |
|---|---|---|---|
| `0x00` | **RLE 记录** | `u8 byte` + `u8 count` | `APS_RECORD_RLE=0x0000`（`:5`） |
| `1..0xff` | 简单记录 | `length` 个原始字节 | `APS_RECORD_SIMPLE=0x01`（`:6`） |

⇒ **单条记录载荷上限 255 字节**；RLE 段上限同样 255（生成端 `differentBytes.length<0xff`，`:190-195`）。

### 应用语义（`apply`，`:106-125`）

1. `validate && !validateSource()` ⇒ `throw new Error('Source ROM checksum mismatch')`（`:107-109`）；
   注意 APS **只校验源 CRC，从不校验目标 CRC** —— 与 BPS/UPS 不同（后者有 `Target ROM checksum mismatch`）。
2. 新建 `BinFile(header.sizeOutput)`，把源 ROM 前 `sizeOutput` 字节整体复制过去（`:111-112`）；
3. 逐记录 `seek(offset)` 后写 RLE 重复字节或直接写数据（`:114-122`）。

⇒ 输出大小**固定等于 `sizeOutput`**，因此 APS 无法把补丁结果增长到超过头里记录的目标大小；
这也是 OoT 社区"扩展 ROM 不能用 APS、要用 BPS/UPS"的真实技术原因（F：由 `:111` 直接可推）。

### 生成端与实现瑕疵（F，值得回报上游）

- **RLE 判定**（`:186-206`）：只在差异段内所有字节相同且 **长度 > 2** 时才用 RLE，
  否则退化为简单记录；差异扫描在 EOF 与 0xff 处终止。
- **死代码 / 潜在误读**（`:140`、`:154`）：`var seek;` **从未赋值**，却在
  `patch.addRLERecord(offset, patchFile.readU8(seek), patchFile.readU8(seek+1))` 中作为实参传入。
  核对 `BinFile.js:274`：`readU8 = function () { … }` **不接收任何参数** ⇒ 结果正确纯属"参数被忽略"，
  `seek+1` 是 `NaN` 也无害。**结论：功能正确但语义误导，属可清理缺陷（非安全缺陷）。**
- `encodingMethod` 字段被完整读写却无任何解释分支（`:137`、`:79`）⇒ **U**：APS 规范里的编码方法取值语义本模块未实现。
- `header.pad` 恒为 5 个 0（`:174`）；读入时保存但从不使用（`:145`）⇒ 记 **U**（用途未知，非本轮必需）。

---

## 2.3 BPS / UPS：变长整数与页脚校验（F）

| 项 | BPS（`RomPatcher.format.bps.js`, 469 行） | UPS（`RomPatcher.format.ups.js`, 223 行） |
|---|---|---|
| magic | `BPS1`（`:96` `seek(4)` 跳过） | `UPS1`（`UPS.MAGIC`） |
| 头字段 | `sourceSize`、`targetSize`、`metaDataLength`(+可选元数据)（`:98-104`） | 源大小、目标大小（VLV） |
| VLV 编码 | `BPS_readVLV` `:142-154`：`data += (x&0x7f)*shift; if(x&0x80) break; shift<<=7; data += shift;` | `UPS_readVLV` `:122-:138`：同构，另在 `x==-1` 时 `throw Error('Can\'t read UPS VLV at 0x…')` |
| VLV 写 | `BPS_writeVLV`：`data = Math.floor(data/128)`（`:161`） | `UPS_writeVLV`：`data = data>>7`（`:112`）⇒ **JS 位运算截为 int32，> 2 GiB 长度编码不可靠**（ROM 场景不触发，记为已知边界） |
| 动作类型 | source-read / source-write 交替（`while` + VLV 长度） | 每条记录 = VLV 相对偏移 + 终止 `0x00` + VLV 长度 + XOR 数据 |
| 页脚 | **最后 12 字节 = 3 个 CRC32**：源、目标、补丁（由 `:107` `endActionsOffset=fileSize-12` 直接确定） | 尾部 checksumInput / checksumTarget / patchData |
| 校验错误 | `'Source ROM checksum mismatch'`（`:42`）、`'Target ROM checksum mismatch'`（`:80`）、`'Patch checksum mismatch'`（`:133`） | 同名三类（`:65`、`:100`、`:182`） |
| 源校验实现 | `validateSource` 用 `romFile.hashCRC32(headerSize)`（`:33`） | 同样用 `hashCRC32(headerSize)`（`:56`） |

⇒ BPS/UPS **双向 + 补丁自身三重校验**，APS 只校验源 —— 三者强弱关系是接口级事实，可直接用于
"该用哪种格式发 OoT 模组"的决策建议（R：真实样本字节级对账未做）。

---

## 2.4 OTR / .o2r 与纹理包 GUI

见 `01/02` 交叉与工单回报正文；本地当前状态：

- **U**：本轮尚未在 `patch-otr/` 取得 OTR 写入端（OTRGUI / OTR 打包器）二进制或源码；
  已知第一轮克隆的 `oot-remake-modding`（含 QuestFramework/SoH 上游）是主要线索，
  等待增量克隆结果后再定 F/R。
- **R**：`.otr` 容器在第一轮 `reports/08` 已按 Ship/Menu 文档记录（`OTR` 头 + gzip 分块 + 文件表）；
  本轮只做**增量核对**，不重复结论。

## 2.6 格式注册表、检测链与容量上限（F）

### 2.6.1 magic 全集（`rom-patcher-js/modules/*.js` 顶部常量，逐个 grep 得出）

| 模块 | 行数 | magic 常量 | 值 |
|---|---|---|---|
| `RomPatcher.format.ips.js` | 282 | `IPS_MAGIC` | `PATCH` |
| `RomPatcher.format.ups.js` | 223 | `UPS_MAGIC` | `UPS1` |
| **`RomPatcher.format.aps_n64.js`** | **210** | `APS_N64_MAGIC` | **`APS10`** |
| `RomPatcher.format.aps_gba.js` | 114 | `APS_GBA_MAGIC` | `APS1` |
| `RomPatcher.format.bps.js` | 469 | `BPS_MAGIC` | `BPS1` |
| `RomPatcher.format.rup.js` | 396 | `RUP_MAGIC` | `NINJA2` |
| `RomPatcher.format.ppf.js` | 269 | `PPF_MAGIC` / `PPF_BEGIN_FILE_ID_DIZ_MAGIC` | `PPF` / `@BEG` |
| `RomPatcher.format.bdf.js` | 90 | `BDF_MAGIC` | `BSDIFF40` |
| `RomPatcher.format.pmsr.js` | 97 | `PMSR_MAGIC`（+ `YAY0_MAGIC='Yay0'`） | `PMSR` |
| `RomPatcher.format.vcdiff.js` | 1016 | `VCDIFF_MAGIC` | 3 原始字节 `\xd6\xc3\xc4`；内部另含 `XZ_MAGIC=[0xfd,0x37,0x7a,0x58,0x5a,0x00]`（`:680`、用于 `:945`） |

检测方式（`rom-patcher-js/RomPatcher.js:100-121`）：`patchFile.readString(8)` 取**前 8 字节**，
再按 `header.startsWith(<MODULE>.MAGIC)` 依次分派到 10 个 `fromFile`。
⇒ 顺序敏感且**只比较前缀**；`VCDIFF` 的 3 个高位字节能否与 `readString` 的解码一致属实现细节（记 `?`）。

### 2.6.2 Node CLI 面（`index.js`，F）

| 位置 | 内容 |
|---|---|
| `index.js:31` | 子命令 `patches a ROM` |
| `:34` | `-v, --validate-checksum` |
| `:35` | `-h1, --add-header`（临时加 header 以适配需要 headered ROM 的补丁） |
| `:36` | `-h0, --remove-header` |
| `:37` | `-f, --fix-checksum`（修补已知 ROM header 校验和） |
| `:38` | `-s, --output-suffix`（输出加 `(patched)` 后缀） |
| `:58` | 子命令 `creates a patch based on two ROMs` |
| `:61` | `-f, --format <format>`：**ips [default], bps, ppf, ups, aps, rup, ebp** |

**发现（F）**：`ebp` 没有独立模块文件，它由 `RomPatcher.js:259`（创建）与
`RomPatcher.webapp.js:1611`（界面）分支处理 —— 即 EBP 是**内建而非模块化**的格式；
而 `bdf / pmsr / vcdiff` 三个**可读**格式**不在 CLI 创建列表**里 ⇒ CLI 可写 7 种、库可读 10 种。
浏览器侧扩展名白名单更宽：`RomPatcher.webapp.js:1224`
`/\.(ips|ups|bps|aps|rup|ppf|ebp|bdf|bspatch|mod|xdelta|xdelta3|vcdiff)$/i`
⇒ 其中 **`bspatch`、`mod`、`xdelta`、`xdelta3` 在本仓库没有对应解析模块**（`modules/` 里无此文件）
——属"白名单宽于实现"的真实覆盖缺口（F：目录列举 + 正则原文对照）。
另有 `FILTER_NON_ROMS`（`:1226`）用扩展名黑名单排除 `txt/diz/rtf/doc/xls/html/pdf/图片/压缩包`，
且注释（`:1225`）说明为何不用 `(?<!…)` 负向后顾（Safari 不兼容）。

### 2.6.3 容量上限（F）

`RomPatcher.js:32`：`const TOO_BIG_ROM_SIZE = 67108863;`（= 64 MiB − 1），
经 `isRomTooBig(romFile)`（`:390-392`）使用 ⇒ **≥ 64 MiB 的 ROM 一律拒处理**。
对 OoT 语境直接相关：N64 原版 8/12/32 MB 无碍，但**高清纹理/扩展工程包**（以及 3DS/Switch 版镜像）
很容易越过该线 ⇒ 这是 Web 端 RomPatcher.js 的现实边界，不是补丁格式的限制。

### 2.6.4 PMSR（Paper Mario）与压缩残留（F）

`RomPatcher.format.pmsr.js:680 附近`：`patch.targetSize = PAPER_MARIO_USA10_FILE_SIZE`（`:7` = **41,943,040** = 40 MiB），
`seek(4)` 读 `u32 nRecords`，其后每条 `u32 offset + u32 length + data`，
并在 `offset+length > targetSize` 时把 targetSize 抬高。
文件顶部保留 `YAY0_MAGIC='Yay0'` 与一段 **被注释掉的 Yay0 解码调用**（`:5`、`:64`）
⇒ 说明 PMSR 在实际分发中可能整体套了 Nintendo `Yay0` 压缩，而本库**未启用**该路径。

### 2.6.5 嵌入式补丁拉取（`?`，接口事实 + 风险说明）

`RomPatcher.webapp.js:165-200`：URL 携带的 "embeded patch" 信息会走
`fetch(decodeURI(embededPatchInfo.file))`（`:182`），随后
- 用 `ZIPManager.isZipFile()` 判断是否 zip，若是则按 `embededPatchInfo.patches`（数组或单对象）批量解包（`:186-198`）；
- 若 `patches` 不是数组只 `console.warn`（`:191`）而不中断；
- 有 `isFetching` 互斥（`:165` 附近，"already fetching another file"）。

⇒ **接口结论**：Web 端支持"一条 URL 指定远端 zip + 内部多补丁"的下发方式，
`file` 字段**没有同源/白名单校验**（仅 `decodeURI`），实际防线只有浏览器同源策略与 CORS 配置。
记 **`?`**：这是可被上游利用的"任意 URL 取字节"入口（浏览器沙箱内、且取到的字节只当补丁解析），
不构成 SSRF（无服务端），但**在自托管站点上会诱导用户下载未知来源补丁**。本轮仅记录，不改动。

## 2.7 REA 取证计数（F，`rea analyze-javascript-application`）

| 项 | 值 |
|---|---|
| root artifact sha256（目录） | `43e96a4b801b9fa74112a109d2db15b74bcd0c08a9da0f4f4c04d52d94b17d2c` |
| relevant_files | **29** |
| parsed_javascript_files | **25** |
| visited_ast_nodes | **49,307** |
| findings | **123** |
| modules / preload_entrypoints / resolved_utility_entrypoints | 0 / 0 / 0（非 Electron，符合预期） |
| static_sources | 25（unknown_sources 9，unexamined 0） |
| application_unknowns | 1；语义图 unknown **23,685** |
| graph.coverage.status | `partial`；semantic_graph.coverage.status = `unknown` |
| authority | `shipped-artifact` |
| 证据 bundle | `../evidence/RomPatcher-js__analyze-javascript-application.bundle.json`，**73,860,123 B** |
| REA 自报遗漏 | `index_template.html` 第 16 行起有 **1 个内联 JavaScript 体未被分析** ⇒ 其 endpoint/storage/import 事实缺失（记为已知盲区） |

⇒ 与第一轮 Ocarina-Atlas 的对照（报告 12 §3.3）：同一工具在**未打包、无混淆**的 928 KB 源码树上
得到 49,307 AST 节点 / 123 findings；在第一轮 114 MB 出厂 bundle 上得到 337,426 节点 / 980 findings。
即**信噪比随是否被 minify 而剧烈变化**，这是本轮方法学上的增量结论（F：两组数字均出自 REA 自身输出）。

## 2.8 zip/bz2 内嵌库（F）

`modules/zip.js/zip.min.js` 与 `modules/zip.js/z-worker.js` 是 **zip.js** 的压缩副本
（`Deflater/Inflater/NOOP` + Web Worker 流水线 + CRC32 表 `3988292384` = `0xEDB88320`）。
`modules/bz2/` 提供 bzip2 解码。⇒ "Web 端"必须自带这两个解码器才能吃 `.zip/.aps` 之类分发容器；
`RomPatcher.format.ppf.js` 的 `@BEG`（`@BEGIN_FILE_ID.DIZ`）与 bdf 的 `BSDIFF40` 同属此生态。
本轮**未**对 minified JS 做逐行还原（`zip.js` 是第三方 MIT 库，接口即其公开 API），记 **U（不需要）**。


## 2.9 字节序抽象（F，`modules/BinFile.js`）

- `this.littleEndian = false`（`:35`）是**每个 BinFile 实例的显式属性**，默认大端；
- 平台原生字节序探测：`new DataView(buffer).setInt16(0, 256, true)`（`:156`）；
- `readU16`（`:279-`）/`readU32`（`:297-`）/`readU64` 各自按该标志分支；
  `readU32` 末尾 `>>> 0`（`:305`）保证返回无符号；
- `copyTo`（`:200`）把 `littleEndian` **传播给目标文件**。

⇒ 这套设计解释了为什么同一个 `APS` 类能同时服务大端 N64 ROM 与小端补丁字段：
调用方按对象分别设置标志（`aps_n64.js:133` 对补丁文件设 `littleEndian=true`，
而 ROM 侧由上游读取器保持大端）。对还原者的意义：**任何"某字段是 LE 还是 BE"的争论都必须回到赋值点**，
不能按平台直觉判断。
