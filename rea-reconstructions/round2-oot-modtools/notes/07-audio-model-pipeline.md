# 7. 音频与自定义模型导入管线（第二轮全量）

> 分级见 `README.md`。目标目录：`oot-peripherals/round2-audio-save-modtools/audio-model/`（**33 仓库 + `binaries/` 1.3 MB**）。
> 本文件所有 `path:line` 均经本轮逐条打开核对（核对方式见 §7.6）。

---

## 7.0 归属与命名更正（F，影响第一轮若干表述）

| 常见误称 | 真实身份 | 证据 |
|---|---|---|
| `NullEndr/zzrtl` | **`z64dev/zzrtl`**（GPL-3.0，C，10★，HEAD 2024-09-30），前身 `z64me/zzrtl`；GitHub 用户 **`NullEndr` 不存在**（`gh api users/NullEndr` → 404） | 仓库 API + `zzrtl/src/zzrtl.c` 版本号 |
| `zzromtool` | **从未开源**，只存在于 z64me 博客（`post/zzromtool-1.0.0…1.0.4`），功能被 `zzrtl` 取代 | 博客索引 + 无仓库 |
| `zzconvert` | 更名 **`z64dev/z64convert`**（GPL-3.0，HEAD 2025-11-13，`src/z64convert.h:2 #define VERSION "1.1.1"`） | 源码 |
| `zzplayas` | 更名 **`z64dev/z64playas`**（GPL-3.0，v0.9.2） | `src/main.c` 参数表 |
| `mashtool` / `zscrenderer` / `ootr-yz` | **不存在**（U，检索无仓库） | — |
| "Gooee 是 OTR 构建器" | **错误**：唯一同名仓库 `Cities2Modding/Gooee` 是《都市：天际线 II》UI 框架（U） | repo API description |
| `jpcxr` / `otr.json` schema / `.fpz` 格式 | **均无 OoT 相关仓库**（U） | 检索 0 命中 |

⇒ 模型导入的**真实工具链命名空间是 `z64dev/*` + `z64me/*` + `hylian-modding/*`**，
"zz\*" 前缀只是历史遗留；第一轮若按 `NullEndr`/`zzromtool` 引用，需按本表替换。

## 7.1 OBJEX 顶点着色枚举（F，升级 §4 的文档面）

`z64convert/src/objex.h:24-31`：

```c
enum objex_vertexshading {
    OBJEX_VTXSHADE_NORMAL, OBJEX_VTXSHADE_SCALED /* 缩放法线 */,
    OBJEX_VTXSHADE_COLOR,  OBJEX_VTXSHADE_ALPHA,
    OBJEX_VTXSHADE_DYNAMIC, OBJEX_VTXSHADE_NONE };
```

⇒ OBJEX 手册面只写了 `color|normal|dynamic|none` **4 种**，实现面有 **6 种**（`SCALED`、`ALPHA` 未写进手册）（F，
"文档宣称面 ⊂ 实现面"的又一例，与 §6.10 Byte-Forge 同类）。
`zobj.h:47 uint32_t dlistOffset; /* offset of display list in file */` 给出 `.zobj` 的显示列表定位字段。

CLI 面（`z64convert/src/cli.c`）：`:12 --in 'in.objex'`、`:13 --out 'out.zobj'`、`:16 --playas`（内嵌 play-as 数据）、
`:24 --world-header 'x,y,z'`、`:26 --binary-header fndcs`（**f=footer / n=names / d=displaylists / c=collisions / s=skeletons**，F：
一条命令行即可开关五段结构）、`:32 --header 'out.h'`、`:53 if (argc < 5 …)`。

OBJEX 本身 = **OBJ 超集**：`io_export_objex_2.5/export_objex.py:111 fw('mtllib %s')`、
`:114-115 fw('skellib %s')`、`:117-118 fw('animlib %s')`（F）⇒ 三条 `lib` 引用把 `.mtl/.skel/.anim` 绑成一个工程。

## 7.2 `.zobj` 二进制记录布局（F，`oot3d-zobj-import/tools/zobj.py`）

- `:101` `struct.unpack_from('>IB', d, p)` ⇒ 骨架遍历：4 字节偏移 + 1 字节标志；
- `:119` `'>hhhBBI'` ⇒ 肢体记录（x,y,z 各 int16 + 2×u8 + u32）；
- `:216` `'>hhhHhhBBBB'` ⇒ 顶点记录。
⇒ 全大端；这三条 unpack 串就是 `.zobj` 的最小可读接口（F），无需反汇编。

## 7.3 zzplayas → zzconvert 迁移：LUT 与 manifest 的硬编码地址（F）

`Z64-CustomPlayerModels/zobj_checker/`（该仓 571 MB 为用户提交模型 blob，
本轮改为 `--filter=blob:none` **sparse checkout 仅 `zobj_checker/` + `guide/`**，628 KB）：

- `zobj_checker.ts:4 buf.indexOf("MODLOADER64")`、`:6 buf.indexOf("!PlayAsManifest")`、
  `:12 byte aligned: (buf.byteLength % 0x10 === 0)`；
- `zzplayas_to_zzconvert.ts:15 n.indexOf("!PlayAsManifest0")`；`:17-18` 遍历
  `cur = readUInt32BE(start+0x4) - 0x06000000; while (cur >= 0x5000 && cur <= 0x5800) …`
  ⇒ **LUT 区位于 `0x5000..0x5800`，指针以 `0x06xxxxxx` 段基址编码**；`:26/:32` 以 `0xDE` 作 LUT 终结符；
  `:41 n.writeUInt32BE(0x06005380, 0x500C)`（成人）/ `:77 0x060053A8`（儿童）；
- `index.ts:8 readFileSync(process.argv[2])`、`:11 buf.readUInt8(0x500B)` ⇒ **`0x500B` 单字节 = 年龄选择器**。

manifest（`.mnf`）文本格式一手样例（`zzconvert-Blender-Link-Templates/OoT/manifest-adult-10u.txt`）：
`:1 DICTIONARY`、`:5 VROM_CODE 0x00A87000`、`:6 TABLE_OBJECT 0xE7F58`、`:17 SEG 0x06000000`、
`:22 DL_WAIST 0x00EBE8 "Limb 1"`、`:177 GoTo( VROM_CODE+0xE6718 );`
⇒ manifest 是**符号化补丁脚本**（可算术表达式），不是二进制（F）。
纹理文件名即接口：`eyes_ZZBANK0x08000000_ZZFORCE0x0_ZZPAL0.png`、`04001F40_ZZTUNIC.tga`
⇒ **`ZZBANK<hex> / ZZFORCE<n> / ZZPAL<n> / ZZTUNIC` 是文件名 DSL**，与 §4 的 `newtex forcewrite/forcenowrite/priority` 同一语义两套载体（F）。

`z64playas` 侧（`src/main.c`）：`:12-17 --script(.mnf)/--input/--bank/--output/--patch/--header`、
`:74` 强制输出扩展名 `.zobj`、`:89-92` `--segment`（缺省 **6** ⇒ `0x06000000`，与 §4 `.skel` 的 `segment 0x0D000000` 默认值不同族）、
`:96 print-vars`、`:136 print-lut`、`:139 print-patch`；`src/methods.c:16 dataNode->type = TYPE_DICTIONARY;`。

ModLoader64 封装侧：`zzplayas_modloader_template/package.json:9-13 "core": ["OcarinaofTime","MajorasMask"]`、
`:39 "oot_script": "ootscript.js"`；`src/zzplayas_template/zzdata.ts:7-12`（`adult_model/child_model/anim_file/oot_script/options/colors`）；
`OotoAPI.ts:1 import { Age, Tunic } from "modloader64_api/OOT/OOTAPI"`、`:55 Z64Online_ModelAllocation`、`:90 Z64_AnimationBank`。

## 7.4 音频侧（本轮最大增量）

**a) `gfxasm` 为什么有三个 exe（F，解第一轮悬案）**
`gfxasm/src/gfxasm.c:17 #error "GBI_PREFIX not defined; please compile with -DGBI_PREFIX=F3D, F3DEX, or F3DEX2"`、
`:20-22` 定义 `F3D 1 / F3DEX 2 / F3DEX2 3`、`:27/:32/:36` 把 `GBI_PREFIX` 重定义为 `f3d|f3dex|f3dex2`；
`:512 if (!strcmp(argv[i], "-b")) binary = 1;`。
⇒ `gfxasm.f3d.exe / gfxasm.f3dex.exe / gfxasm.f3dex2.exe` **同一份源码的三次编译**，差别只在宏；
输入是 **stdin 每行一条宏**，输出 **8 字节 display-list 字**（`-b` 切二进制）。§5.2 里三个镜像 imphash 不同正由此解释（F）。

**b) `.seq`/音色库表地址（F，`zzrtl-audio/oot_dump.rtl`，脚本型仓库、仅自带 `zzrtl-r5b-4.exe`）**
`:44 TBL_AUDIOSAMPLE = 0x00BCCDA0`、`:47 TBL_AUDIOBANK = 0x00BCC270`、
`:49 TBL_SEQUENCEINSTRUMENTSET = 0x00BCC4E0`、`:51 TBL_AUDIOSEQ = 0x00BCC6A0`（**debug ROM 地址集**）；
`:137-143` 给 1.0 零售版：`TBL_AUDIOTABLE_FILE = 0x00079470`、`TBL_SEQUENCE_FILE = 0x00029DE0`。
⇒ 同一工具在 debug/零售两种 ROM 下**表基址完全不同**，这解释了 §3.2 里 "需要 `oot-debug.z64`" 的原因（F）。
`:3` 注释声明该 fork 的四个内建命令 `audiobank(), audioseq(), audiotable(), restrictionflag()`；
`config.tsv:6 "build/dump audio true (optional if you don't plan on doing audio stuff)"`。

**c) ROM 内音色库定位常量（F，`isghj5/z64-bank-ripper/"Zelda64 Bank Ripper.py"`）**
`:26 ROM_FILE = sys.argv[1]`、`:27 ROM_SIZE = 67108864`（**64 MiB，与 §2.6 `TOO_BIG_ROM_SIZE` 同界**）、
`:56` OoT 签名字符串 `b"HT EELEGDNO  FEZDL A…"`（**空格交错的 "THE LEGEND OF ZELDA" 与 "MAJORA" 变体 `:61`**）、
`:70 'Audiobank': (0x0000D390, 0x0001CA50)`、`:71 'Audiobank_index': (0x00B896A0, 0x00000270)`、
`:178` 期望 bank 数 `0x26`(OoT)/`0x29`(MM)、`:182 extract_banks(...)`。

**d) `z64audio.exe` 的下游封装（F，§3.2 的黑箱终于有了调用面）**
`isghj5/z64-audiobin-generator/"Zelda64 Audiobin Generator.py"`：`:276 extract_and_write_audiofile(rom, offset, size, filename, tempfolder)`、
`:284 write_audiofile_indices`、`:290 generate_audiobin(game, file_dir, tempfolder)`、`:306 main(game)`、
`:65-99` **硬编码的零售 OoT 乐器表字节序列（16 字节/项，如 `0x00,0x00,0x3A,0xA0,0x02,0x00,0xFF…`）**。
⇒ 该脚本以"调 z64audio + 补索引 + 贴乐器表"的方式生成 audiobin ⇒ `z64audio.exe` 的 `-i/-o` 与 `.book.bin/.vadpcm.bin`
产物在真实管线中的位置被外部脚本固定下来（F，接口面不再依赖反汇编）。

**e) DSP-ADPCM（3DS/Switch 侧）与 N64 VADPCM 的头字节相反（F，关键对照）**
`oot3d-zobj-import/web/dspadpcm.js:1-3` 注释原文：
*"DSP-ADPCM encoder: a line-for-line port of tools/dspadpcm.py encode() … Returns {data: Uint8Array} of **8-byte frames: header (pred<<4 | scale) + 14 nibbles**"*；
`:4` `nframes = Math.ceil(n / 14)`、`:9` `roundEven`（**复刻 Python `round()` 的"半值取偶"**）、`:10` 历史寄存器 `h1,h2`。
⇒ 对照 §6.5：N64 VADPCM 帧 = 16 样本 + `(shift<<4)|filter`；3DS DSP-ADPCM 帧 = **14 样本** + `(pred<<4)|scale`
——**同一段高 4 位/低 4 位的语义在两个平台上恰好互换**。任何"通用 ADPCM 封装"若不分表处理必然产生噪声（F）。
工具侧 `tools/dspadpcm.py:11 decode(...)`、`:37 encode(...)`；浏览器里由 `web/worker.js:15` 的
`self.dspEncode = dspEncode` **覆盖纯 Python 实现**（F，见 §7.5）。

**f) 3DS 音频容器 CSAR（F，`oot3d-zobj-import/tools/`）**
`csar.py:49 assert d[:4] == b'CSAR'`、`:27 kind: int # 0x2201 stream, 0x2202 wave, 0x2203 sequence`、
`:55 strg, info, fileb = blocks[0x2000], blocks[0x2001], blocks[0x2002]`；
`voice.py:1-3` 文档字符串给出完整命令行：
`python tools/voice.py <pack.pak|sounds folder> <romfs>/sound/QueenSound.bcsar <out_dir> [--rate 22050]`；
`voicemap.py:19-21 return 'NA_SE_VO_LI_' + N64_ADULT[i] for 0x6800+i`（`:24` 儿童用 `_KID`）
⇒ **OoT3D 语音包 = 重写 `QueenSound.bcsar` 的 CSAR 块 `0x2000/0x2001/0x2002`**，采样率参数 `--rate 22050`（F）。
⇒ 工单里 ".bfgrp/.awb 音频工具" 对 OoT **不存在**（U；`dixonjon0965/bfgrp` 是 SEO 垃圾文本，非代码）。

**g) 3DS CMB 结构 = 开源 Python 常量表（F，绕开 Grezzo 黑箱）**
`CMB-Exporter/cmb_internal/cmb_constants.py:4-17`：
`CMB_VERSION_OOT3D = 0x6`；magic 全表 `cmb `/`skl `/`mats`/`tex `/`sklm`/`mshs`/`shp `/`sepd`/`prms`/`prm `/`luts`/`vatr`；
`binary.py:23-42` `write_u8/u16/u32/f32` 全部 `struct.pack("<…")` ⇒ **CMB 是小端**（与 `.zobj` 大端相反，F）；
`operators.py:381 bl_idname = "export_scene.cmb"` + `cmb.convert_material` / `cmb.add_cmab_texture` / `cmb.visualize_cmab_texture`（`:207-359`）；
`__init__.py:2-5 "name": "CMB Exporter", "version": (1,7), "blender": (3,5,0)`。
⇒ 第一轮"拒绝复刻 Grezzo 闭源 CMB 解析"的结论**现在有了合法替代**：接口由 GPL-3.0 开源 Blender 插件的常量表直接给出，
无需逆向闭源二进制（F）。同族参考：`N3DSCmbViewer`（C#，MIT，解析 `Csab/`、`Shaders/DVLB|DVLE|DVLP.cs`）+ 其 `etc1compress`/`ETC1Lib.dll`（§5.1）。

## 7.5 唯一真正的"Web 端模型/音频应用"：`oot3d-zobj-import`（F）

`web/worker.js`（30 行）把整个工具链搬到浏览器，结构极其透明：

- `:7` `const PYODIDE = 'https://cdn.jsdelivr.net/pyodide/v0.29.3/full/'` ⇒ **运行时依赖 CDN 上的 Pyodide 0.29.3**；
- `:8-11` `TOOLS = [build, voice, pack, ml64pak, zobj, cmb, cmbskel, pose, cmbwrite, cmabwrite, texenc, csab, zar,
  pica, fit, tunics, csar, voicemap, dspadpcm, equippak, animconv, n64anim, anim_tables, animview, cmbview,
  render, seedmerge]` ⇒ **28 个模块**；
- `:13-14` `SEED_ASSETS = ['nightly-6bd4b80','v4.0.2'].flatMap(b => ['usa','eur'].flatMap(r =>
  ['extra-buttons-…ips','…json']))` ⇒ "Extra Buttons" 补丁按 **构建 × 区域** 组合；
- `:15` `importScripts(PYODIDE + 'pyodide.js', 'dspadpcm.js?v=20261013b')`（**带查询串做缓存版本号**）；
- `:16` `self.dspEncode = dspEncode; // tools/dspadpcm.py picks this up instead of its pure-Python loop`
  ⇒ **JS 覆盖 Python 的运行时注入**（性能补丁式架构，F）；
- `:24-25` `loadPyodide(...); py.loadPackage(['numpy','pillow'])`；
- `:31` `fetch(\`../tools/${name}.py\`)` ⇒ **Python 源码不打包进 bundle，运行时抓取**；
- `:2-5` IPC 契约：`{type:'scan'|'convert'|'log'|'ready'|'scanned'|'done'|'error'}`；
  `web/app.js:106 worker.postMessage({type:'scan', inputs})`、`:321` convert。

⇒ 三点判断（F）：
1. **没有自有 WASM**（Pyodide 的 WASM 来自 CDN）；§5.4 的"WASM 段级不可还原"在此表现为
   "本轮无新 WASM 工件需要 WABT"，而非取证缺口。
2. **离线不可用 / 供应链面**：应用正确性依赖 `cdn.jsdelivr.net` 上的 Pyodide 版本与 `numpy/pillow` 包；
   `tools/*.py` 走相对路径 `../tools/`，同源 ⇒ 未发现 `fetch(decodeURI(...))` 式任意 URL（对照 §2.6 的 RomPatcher 嵌入式 fetch，那一处仍是 ?**）。
3. 3DS 本体也有原生 app（`app/source/main.c:472 romfsInit()`，发布 `OoTModelManager.3dsx` ARM 二进制）⇒
   **同一套 Python 工具同时服务浏览器 worker 与 3DS 本地执行**（F）。

**同族 Web 目标（F）**：`OOTR_Blender`（Switch  remake → Blender）
`core/byml.py:20 BYML_VERSION = 7`、`:53 raise BymlError("expected little-endian BYML magic")`、
`core/native_bridge.py:87 return "ootr_native.dll"` + `:112 ctypes.CDLL` ⇒ **Blender 插件里夹带一个未公开源码的原生 DLL 桥**（本轮唯一新增的"闭源黑箱"观察点，接口仅到 `ctypes` 加载面；深逆向记 U）；
`core/converter.py:23 def convert_bfres(`、`addon.py:149 NupError("…no importable Model/*.bfres.zs resources")`、
`:389 bpy.ops.import_scene.gltf(`、`:656/:1105 output = work / "model.glb"` ⇒ **BFRES→glTF 两段式**；
`__init__.py:6-8 version (0,6,0) / blender (5,2,0)`；`blender_manifest.toml platforms=["windows-x64"]`；
README 声明"不含游戏数据/密钥/导出资产"，要求用户提供自己的已解密 NUP（title `0400486024DEC000`，
与第一轮 OoT3D program id 及 §1.5 `sraFn` 同一 id — F，三源互证）。

## 7.6 抽查记录（哪些是我亲眼读到的）

本轮亲自 `sed/cat` 打开核对过的行：`z64convert/src/objex.h:24-31`、`CMB-Exporter/cmb_internal/cmb_constants.py:4-17`、
`oot3d-zobj-import/web/worker.js:5-16`、`web/dspadpcm.js:1-12`、`gfxasm/src/gfxasm.c:15-38`、
`Z64-CustomPlayerModels/zobj_checker/zzplayas_to_zzconvert.ts:14-42`、
`RomPatcher.js/package.json:20-21`、`Byte-Forge/lib/core/rom-patcher.ts:38-51`、
`OTRMod/OTRMod/OTR/Generate.cs:18-40`、`OoT-Randomizer/MBSDIFFPatch.py:15,72,87,121,144,159`、
`OoT-Randomizer/N64Patch.py:36,48,61,98-109`、`oot-save-edit/models/{saveslot.ts:330-339,410-412,1072-1114, saveheader.ts:24-26, savefile.ts:104-118}`、
`OOT-Save-Converter/oot.js:33,44-51`、`N64SaveConverter/…/N64SaveConverter.java:42-49`、
`segher-wii-tools/zelda-cksum.c:8-24`、`oot-sw2-save/ootsw2/container.py:38-46`、
`Admentus64-…/N64 Save Converter.ps1:415-423`、`ocarina-songbook/…/oot_soundfont.py:14,164,199-209`、
`z64-bank-converter/…/Codebook.py:44,53,68,72,169`、`save-file-converter/…/N64/DexDrive.js:6,24,26`、
`mempak/js/parser.js:630`、`ZeldaSaveTool/Save/File.cs:6-12,79-95`、`ZeldaSaveTool/ToolForm.cs:30-38`、
`z64audio.exe` 的 9 个字符串 `grep -a` 与 `evidence/z64audio_exe__inspect-binary-hardening.json` 全表。
其余（star 数、HEAD 日期、`du -sh`、release 资产大小、检索 0 命中的 NOT FOUND 列表）为 agent 报告值，标 **R**。

## 7.7 未找到 / 边界（U）

- **U**：`cmixer`（MIT）虽名含 mixer，**仓库内不含任何 z64 序列/ADPCM 解码器**（`src/cmixer.h:56-71` 只有
  `cm_init/cm_new_source_from_file/cm_set_pitch/cm_set_loop`）⇒ 不能当"OoT 音频后端"引用。
- **U**：`z64me/z64me.github.io` 的 **Web ROM 查看器源码未公开**（agent 克隆 65 MB 确认仅 Hugo 博客后删除）
  ⇒ §2/§6 里"Web 端 rom 查看"这条线在 OoT 生态**确实缺位**。
- **U**：MML→`.seq`、MIDI↔`.seq`、`.x3d`/`.fx` 混响表、`.mov64` 转换、`.zsc` 编辑器（SceneNavi/z64yartool/z64scene 之外）——
  均无仓库。
- **U**：`OoT-Randomizer/bin/{minibsdiff,gzinject,Compress,Decompress}/` 的**预编译二进制**本轮未做 REA 取证；
  `OoTR_Blender/core/ootr_native.dll`（未随仓库发布，用户本机）也无法取证。
- **U**：`oot3d-texkit.exe`（31 977 109 B，.NET）超出 20 MB 采集上限 ⇒ 未下载未取证（其 CLI 面见 agent 值，R）。
- **?**：`z64me/z64scene` 无许可证声明；`zzrtl-audio` 无许可证但自带 `zzrtl-r5b-4.exe`（GPL-3.0 衍生体的二进制再分发合规性存疑）。
