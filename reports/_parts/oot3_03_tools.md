
---

## 4. 资产工具链与容器格式族

> 取证对象：`$P/ZAPDTR`、`$P/OTRExporter`、`$P/Torch`、`$P/retro`、`$P/neoretro`，以及镜像 `$P/../oot-ports-repos/ZAPD`（zeldaret 上游）、`oot-ports-repos/fast64`。
> 快照声明：所有仓库都是浅克隆（1 commit），**提交日期只代表 HEAD 快照日期，不代表真实历史**；§4.6 的时间线因此在方法上属于"由代码依赖关系推断 + 快照日期佐证"。

### 4.1 ZAPDTR：ROM → XML + C + `.bin`

`ZAPDTR/README.md:1` 自称 "ZAPD: Zelda Asset Processor for Decomp"。注意源码不在 `ZAPDTR/src/` 而在 **`ZAPDTR/ZAPD/`**。

* **类型注册是自注册工厂，不是 switch**：`ZResource.h:246` 定义 `REGISTER_ZFILENODE(nodeName, zResClass)`，构造静态对象时调 `ZFile::RegisterNode`（`ZFile.cpp:1086-1089`，map 在 `:1082`）；解析按 XML 节点名查表：

```cpp
/* ZAPD/ZFile.cpp:280-290（节选） */
std::string nodeName = std::string(child->Name());
if (nodeMap.find(nodeName) != nodeMap.end()) {
    ZResource* nRes = nodeMap[nodeName](this);
    if (mode == ZFileMode::Extract || mode == ZFileMode::ExternalFile ||
        mode == ZFileMode::ExtractDirectory) {
        if (!isCompilable) nRes->ExtractWithXML(child, rawDataIndex);
```

  真实注册点：`ZBlob.cpp:10`、`ZDisplayList.cpp:137`、`ZVtx.cpp:7`、`ZTexture.cpp:19`、`ZMtx.cpp:7`、`ZBackground.cpp:11`、`ZAnimation.cpp:12-15`（Animation/PlayerAnimation/CurveAnimation/LegacyAnimation）、`ZPlayerAnimationData.cpp:8`、`ZCollision.cpp:12`、`ZRoom/ZRoom.cpp:50-52`（Room/Scene/AltHeader）、`ZSkeleton.cpp:10-11`、`ZLimb.cpp:11`、`ZCutscene.cpp:11`、`ZText.cpp:10`、`ZTextMM.cpp:10`、`ZAudio.cpp:10`、`ZWaterbox.cpp:7`、`ZSurfaceType.cpp:7`、`ZArray.cpp:10`；枚举 `ZResource.h:25-68`。**`MTex` / `MaterialList` 作为独立节点在 HEAD 不存在**（只有 room 命令 `ZRoom/Commands/SetAnimatedMaterialList.cpp`）——`MTex` 属早期版本。
* **fork 独有能力：直接在内存里切 ROM**。`ZRom.cpp` 内置 OoT/MM 全部版本的 DMA 表偏移与 CRC 表：

```c
/* ZAPD/ZRom.cpp:1-40（节选） */
#define OOT_OFF_NTSC_10 0x7430
#define OOT_OFF_NTSC_12 0x7960
#define OOT_OFF_PAL_GC  0x7170
#define MM_OFF_US_10    0x1A500
```
* **`XmlWriter` / `BoundsChecker` 在 HEAD 无同名文件**（全库零命中）；职能由 `OutputFormatter.h:6`（C 代码格式化）与 `ZFile::HandleUnaccountedData()` 承担，且后者在 OTR 模式下直接短路：`ZFile.cpp:1332-1340 if (Globals::Instance->otrMode) return;`
* **模式与开关是真实存在的那几个**：`ZFileMode` 在 `ZFile.h:12-24`（含 fork 独有的 `ExtractDirectory` 与 `Custom = 1000`），`Main.cpp:469-490 ParseFileMode` 把 `btex/bren/bsf/bblb/e/ed` 映射过去；`Globals.h:51 bool otrMode = true`、`:53 onlyGenCustomOtr`、`:54 xmlExtractModes`；CLI `-se/--set-exporter`（`Main.cpp:432-433`）、`--norom`→`onlyGenCustomOtr`（`:585-588`）、`-brt`→`buildRawTexture`（**唯一**的 raw 纹理开关）、`-fl` 文件列表、`-oxml asf,asp,asq`。**`--otr`/`SoHExport`/`RawDump`/`n64pngs` 这些网传参数名不存在**。
* **插件机制**：`ExporterSet.h:11-28`（11 个钩子）+ `ZResource.h:262 REGISTER_EXPORTER`；`Main.cpp:568-571 ImportExporters()`；`ZFile.cpp:389-440` 依次触发 `beginFileFunc`/`exporter->Save`/`resSaveFunc`/`endFileFunc`，`:428-434` 把带自定义导出器的资源写成 `<File>.bin`；样例 `ExporterTest/Main.cpp:59-79`。
* **与上游 ZAPD 的差异**（`oot-ports-repos/ZAPD`）：ZAPDTR 独有 `ZRom`、`ZText/ZTextMM/ZAudio/ZAudioDecode`、`FileWorker`（多线程）、`Overlays`、`yaz0`、内联 `Utils/`；上游把 Utils 独立成 `ZAPDUtils/`，`ZFileMode` 没有 `ExtractDirectory`，flag 表有 `--cs-float/--base-address/--start-offset/--end-offset` 但没有 `-fl/-brt/--norom/-oxml`。两版都有 `REGISTER_ZFILENODE` 与 `ExporterSet.h`——**注册/插件骨架已回流上游，"从 ROM 直抽 + OTR bin 输出"仍是 fork 独有**。

### 4.2 OTRExporter：ZAPD 的导出插件（不是替代品）

硬证据：`OTRExporter/OTRExporter/TextureExporter.cpp:2` `#include "../ZAPD/ZFile.h"`；`Main.cpp:437` `Globals::AddExporter("OTR", …)`；驱动脚本 `extract_assets.py:12-36` 展示的实际命令行：

```
ZAPD ed -i <xml> -b <rom> -fl assets/extractor/filelists -gsf 0 -se OTR \
       --customAssetsPath ... --otrfile oot.o2r --portVer ...
```

自定义资产走另一条模式：`ZAPD botr -se OTR --norom ...`（`extract_assets.py:47-53`；`botr` 由 `OTRExporter/Main.cpp:56-75` 注册为 `ZFileMode::Custom+1`）。MM 侧用 `#ifdef GAME_MM` 条件注册额外类型（`Main.cpp:432-436`）。64 字节头的写出见 §3.2。

### 4.3 Torch：YAML 驱动的通用资产工厂（最新一代）

`Torch/README.md:1`："[T]orch is [O]ur [R]esource [C]onversion [H]elper / A generic asset processor for games"，用法 `./torch otr baserom.z64`、`./torch code baserom.z64`。

* **`TemplateFactory.{h,cpp}` 不是生产逻辑，而是工厂模板**（类名一律 `Type*`），演示三件事：`parse()` 解 segment、三种 exporter（Header/Code/Binary）、以及 OTR 模式下头部只写占位符：

```cpp
/* Torch/TemplateFactory.cpp:13-20 */
if (Companion::Instance->IsOTRMode()) {
    write << "static const char " << symbol << "[] = \"__OTR__" << (*replacement) << "\";\n\n";
    return std::nullopt;
}
write << "extern Type " << symbol << ";\n";
```
  真正的框架在 `src/factories/BaseFactory.h`：`BaseExporter::Export` 纯虚 + `static WriteHeader`（`:99`）、`BaseFactory::parse/parse_modding`（`:104-105`）、`ExportType {Header,Code,Binary,Modding,XML}`（`:46-52`）、`REGISTER(type,c)` 宏（`:30`）。
* **多游戏化**：`src/factories/{oot,sm64,mk64,sf64,pm64,bk64,fzerox,mario_artist,naudio}` —— 这也解释了为什么 OoT 移植的资产工具正在向 Torch 收敛，而 **`src/factories/` 下没有 mm 目录**（MM 的 4 个独有类型 `OTXM/OTAN/OKFA/OKFS` 尚无落点），迁移未覆盖 MM。
* **同时写两种容器**：`src/Companion.h:51-56 enum class ArchiveType {None, OTR, O2R}`；OTR 走 `src/archive/SWrapper.cpp:17-30`（`SFileCreateArchive(..., 16*1024, &hMpq)`）+ `:69-80`（>32 位抛异常），O2R 走 `src/archive/ZWrapper.cpp:33-46`（`mz_zip_writer_init_file(path + ".tmp")` + `MZ_BEST_COMPRESSION`）+ `:78-101`（finalize 后 `rename`）。CLI：`src/main.cpp:139-156` `o2r` 子命令、`:229-236 pack <archive-type> otr|o2r`、`:269-293` 三种 modding import 模式。
* **根条目**：`portVersion`/`version`（`Companion.cpp:1905-1912`，`ParseVersionString` `:2560-2578`）与 `torch.hash.yml`（`:1930-1933`）。
* **文档不一致（应记录为已知问题）**：`Torch/docs/otr_format/mainpage.md:9-12` 声称"`.otr` 或 `.o2r` 都是标准 ZIP 归档"，与 `SWrapper` 的 MPQ 实现相矛盾。

### 4.4 retro / neoretro：面向 mod 作者的打包 GUI

`$P/retro` 修复后是一个 **Flutter/Dart 桌面工程**：`pubspec.yaml:1-2` "retro / An OTR / O2R generation tool."；`lib/otr/resource.dart:17` `int magicID = 0xDEADBEEFDEADBEEF`，`_writeHeader()`（`:26-41`）写出的正是与 OTRExporter 同构的 64 B 头（native endian @0x00、type @0x04、gameVersion @0x08、magicID @0x0C、resourceVersion @0x10、isCustom @0x14，其余补零到 0x40），`lib/otr/resource_type.dart:1-27` 复刻 magic 表，`lib/arc/arc.dart:24-40` 按扩展名分派（`.otr` → `MPQArchive.create(path, MPQ_CREATE_SIGNATURE | MPQ_CREATE_ARCHIVE_V2, 65535)`，否则 `ZipFileEncoder`；`.o2r` 解码走 `zdec.decodeBytes`）。README 顶部宣布**已停维护**：`retro/README.md:5-11` "# Retro has moved to NeoRetro … It has been retired in favor of **NeoRetro** … This repository is kept for reference only."

后继者 `$P/neoretro`（本次新增克隆）是 C++/CMake 桌面工程：130 个 C/C++ 文件、13,118 行（cpp 11,136 + h 1,982），目录 `src/{app,features,games,image,l10n,models,otr,services,ui,utils}`、`tests/`、`tools/`、`docs/texture-packs.md`。要点：

* **UI 栈是 raylib + ImGui(rlimgui) + clay**：`CMakeLists.txt:190-246` 依次 FetchContent `raylib`、`ImGui`、`rlimgui`、`clay`，另拉 `spdlog`、`lodepng`、`stb`、`picosha2`。
* **它把 Torch 当库用**：`CMakeLists.txt:252-265` `FetchContent` 固定 `GIT_TAG 72960ca16f…`（本地浅克隆 HEAD 是 `22d32182…`，**版本不一致，本节对 Torch 的引用以本地快照为准**），并 `foreach(TORCH_GAME SM64 MK64 SF64 PM64 FZERO BK64 MARIO_ARTIST NAUDIO OOT) set(BUILD_${TORCH_GAME} OFF)` —— 这份列表等于官方承认的 Torch 游戏矩阵；同时用 `cmake/patches/torch-swrapper-close.patch` 给 Torch 的 `SWrapper::Close` 打补丁（修正"关闭失败被反向判断"并把 `hMpq` 置空 + 打印 `GetLastError()`）。
* **格式侧直接复用 Torch 枚举**：`src/otr/Types.h:5-6` `using ResourceType = Torch::ResourceType`，同文件 `TextureType` 11 项（含 `JPEG32bpp = 10`）、`kTextureFlagLoadAsRaw = 1 << 0`；`src/otr/Archive.h:13-40` 用 `ArchiveReader`/`ArchiveWriter` 抽象同时覆盖 MPQ 与 ZIP（`bool IsZip()`）。
* **功能面按游戏分屏**：`src/features/games/GameScreens.h:5-30` 有 `SohGameScreen / GhostshipGameScreen / PaperboatGameScreen / Ship2GameScreen`，`src/app/Routes.cpp:26-37` 注册对应路由（SoH 主屏 + 地图字体转换、Ghostship 文本三类 dialogs/texts/strings、Paperboat 图像与文本），另有 `src/features/inspect_otr/InspectOtrScreen.*` 归档浏览器与 `src/features/create`、`src/features/debug`。
* README 仍自述 "An OTR generation tool … Currently only Ship of Harkinian uses this format"，并描述了 `appcast.json` 自更新流程（SHA-256 校验后替换自身）。

结论：`retro` **既不是 retro-patch，也不是 SoH 的分支**，而是 OTR/O2R 手工打包器 —— 它和 `2ship2harkinian/README.md:67` 的推荐语一致。

### 4.5 容器与格式族对照（本报告 §3.2 的镜像视图）

| 维度 | `.otr`(MPQ) | `.o2r`(ZIP) | Folder | `.ootrs` |
|---|---|---|---|---|
| 解析侧 | `OtrArchive.cpp:22-62,71` | `O2rArchive.cpp:41-96` | `FolderArchive.cpp:22-41` | SoH `OotrsArchive.h:29`（内含 `O2rArchive`） |
| 写出侧 | OTRExporter `ExporterArchiveOTR.cpp:63-66,100`（`MPQ_COMPRESSION_ZLIB`）；Torch `SWrapper.cpp` | OTRExporter `ExporterArchiveO2R.cpp:51-77`；Torch `ZWrapper.cpp` | n/a | Torch `--norom` 系 |
| 编译开关 | `INCLUDE_MPQ_SUPPORT`（LUS 默认 OFF / SoH 强制 ON） | 无（libzip 必需） | 无 | 游戏侧 |
| 可写 | 否（显式拒绝） | 是 | 是 | — |
| 尺寸上限 | 32 位 `DWORD` | `size_t` | 文件 | ZIP |
| 索引来源 | `(listfile)` | ZIP 中央目录 | `Directory::ListFiles` | ZIP + 歌曲表 |
| 签名/manifest | 共用（`Archive.cpp:187-300`，BLAKE2b-512 + Ed25519 + keystore） | 同左 | 同左 | 未验证 |

产物侧现状：SoH 已完全产出 `oot.o2r / oot-mq.o2r / soh.o2r`（`OTRGlobals.cpp:281-282,418-419,616,841-848`；`CMakeLists.txt:309-349`；CI `generate-builds.yml:35-43`），mod 收集时 `.o2r` 总收、`.otr` 仅当 MPQ 支持开启时收、`.zip` 故意不收、`.ootrs` 单列（`Enhancements/mod_menu.cpp:194-210`）。

### 4.6 工具链取代关系（含推断标注）

1. **ZAPD（zeldaret）**：decomp 侧原始工具，XML→C，需要预切 baserom。
2. **ZAPDTR（HarbourMasters fork）**：加 ROM 内切分（`ZRom`）、`ExtractDirectory`/`-fl`、`otrMode`、`FileWorker` 多线程、MM 类型。依据：文件集合与 flag 集合的差集。
3. **OTRExporter**：ZAPDTR 的**插件**（§4.2 的 `#include` 与 `-se OTR` 是硬证据），把 ZAPD 输出直接变 `.bin`→`.o2r`。2SoH 至今仍是这条链。
4. **retro → neoretro**：面向 mod 作者的手工打包 GUI，README 明说 retired。
5. **Torch**：把 ZAPDTR+OTRExporter 的职责合并进 YAML factory（同一套 magic/64 B 头、同时写 OTR/O2R）并扩到 SM64/MK64/SF64/BK64。取代关系的硬证据在 Shipwright 侧：`.gitmodules` 移除 ZAPDTR/OTRExporter 而挂 `torch`；`CMakeLists.txt:327-335` 承认 Torch 尚缺 OoT header 导出；`docs/BUILDING.md:83` 直言迁移删掉了 ZAPD-based header generator。
6. **未完成迁移**：2SoH 仍嵌 ZAPDTR + OTRExporter；Torch 无 mm factory 目录。

> 本节的"取代/收敛"叙事基于代码依赖关系与文档措辞，属**强推断**；浅克隆无法用提交时间序列独立验证。

### 4.7 fast64 与 ZAPD 的正反向互补

`oot-ports-repos/fast64/README.md:5-9`：导出 F3D 显示列表 + SM64/OoT **decomp 工程资产**，支持自定义 combiner/geom mode。导出面 `fast64_internal/z64/exporter/{actor,collision,cutscene,scene,skeleton,animation}`（`z64/README.md:23-91` 逐项说明场景/actor 放置/碰撞体含水陆空与相机/灯光/出口索引/骨骼 SkinLimbs/过场）。微码枚举 `f3d/f3d_enums.py:380-393`：F3D、F3DEX/LX、**F3DEX2/LX2/ZEX（注释明确 "used in OoT and MM"）**、F3DEX2.Rej、F3DEX3 及多种顶点打包方案。与 ZAPD/TR 的关系：**fast64 做 Blender→C/dlist 的正向创作，ZAPD/TR 做 ROM→XML/C 的反向导出**（此句为基于两者文档面的推断）。值得注意的是 LUS 的 dlist 资源就是词流（`src/fast/resource/factory/DisplayListFactory.cpp:170-223`：int8 ucode → 8 字节对齐 → 逐条 w0/w1 → 扩展 op 多读 64 bit），因此 fast64 产出的 dlist 与移植引擎读取的 dlist **同构**，这是"用 Blender 重做素材再塞回 o2r"可行的根本原因。
