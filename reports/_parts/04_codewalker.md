## 3.4 CodeWalker（dexyfex/CodeWalker）

### 项目定位
CodeWalker 是 dexyfex 用 C# 编写的 GTA V / RAGE 引擎数据离线解析与重打包套件，同时是一个基于 SharpDX + Direct3D11 的世界/模型查看器与地图编辑器。它在本报告语境中的价值不在渲染，而在于它把 RAGE 的容器格式（RPF/RSC）、类型元信息（META/PSO/RBF）与资源类布局（drawable/bounds/clip/nav）完整地"可执行化"了：既能解码，也能重新编码成游戏可加载的文件。`README.md:1-8` 自述 scope 为 "viewing the contents of GTAV RPF archives"，并要求 DX11/SM4.0、Windows x64、.NET 4.5+。

### 仓库元数据
- 浅克隆：`.git/shallow` 存在，`git rev-list --count HEAD` = **1**，无 tag（`git describe` 失败），HEAD = `485d56be`（2025-04-11，作者 dexy，"ModManager progress"），remote `https://github.com/dexyfex/CodeWalker` master。
- **无 LICENSE 文件**。仅有 `Notice.txt`（内嵌 Neodymium 2015 / Cameron Berry 2017 的 MIT 片段、hamish-milne FbxWriter 的 GPL 声明）与 `Readme_Src.txt`："This source code is released for educational purposes only."（推断：整体无 OSI 许可证，代码版权实际保留）。
- 源码 vs 附带二进制：克隆内无任何 DLL/EXE/ASI；随源码提交的二进制只有 `Shaders/*.cso` 90 个（由 `CodeWalker.Shaders/CodeWalker.Shaders.vcxproj:79` 的 `ObjectFileOutput` 编译 HLSL 产生）、`CodeWalker.Core/Resources/magic.dat`（154 KB 加密密钥块，`CodeWalker.Core.csproj:17` 作为 `<Resource>` 嵌入）、图标与 92 个 `.resx`。发布包中的 DLL 不在源码树（推断）。

### 解决方案与模块清单（行数）
`CodeWalker.sln` 含 10 个项目：`CodeWalker.Shaders`(vcxproj/HLSL)、`CodeWalker.Core`(netstandard2.0)、`CodeWalker`(net48 WinExe x64)、`CodeWalker.WinForms`、`CodeWalker.ErrorReport`、`CodeWalker.Peds`、`CodeWalker.RPFExplorer`、`CodeWalker.Vehicles`、`CodeWalker.ModManager`、`CodeWalker.Gen9Converter`。**仓库中不存在 `Common.Libraries.props` / `Version.props`**，也不存在 `CodeWalker.Cmd` / `CodeWalkerDoc` 项目（题设假设需修正）。NuGet 依赖：`SharpDX 4.2.0` 全家桶（D3D11/Direct2D1/D3DCompiler/XAudio2/XInput/Mathematics）、`DockPanelSuite.ThemeVS2015 3.0.6`、`FCTB 2.16.24`；**无 OpenTK、无 System.Buffers**。C# 总量 392 文件 / 327,047 行（其中 `.Designer.cs` 44,858 行）。

| 模块 | 文件 | 行 | 职责 |
|---|---|---|---|
| `CodeWalker.Core/GameFiles`（根） | 4 | 9,499 | `RpfFile.cs` 2,882 / `RpfManager.cs` / `GameFileCache.cs` 5,984 / `GameFile.cs` |
| `GameFiles/FileTypes` | 41 | 53,231 | 每种 `.y*` 扩展名的入口解析器 |
| `GameFiles/MetaTypes` | 13 | 53,734 | META/PSO/RBF + `MetaNames.cs` 哈希名字表 |
| `GameFiles/Resources` | 21 | 41,600 | RSC 资源类布局（Drawable/Bounds/Clip/Frag/Nav/Texture…） |
| `GameFiles/Utils` | 5 | 5,452 | `GTACrypto` `GTAKeys` `Jenk` `Data` `DDSIO` |
| `CodeWalker.Core/World`+`Utils` | 28 | 13,800 | 世界模型（Space/Timecycle/Water/Vehicle）与 BVH/FBX |
| `CodeWalker`（查看器/UI） | 223 | 137,028 | WorldForm/ExploreForm/Project 编辑器/渲染 |

### RAGE 资源格式全景
| 格式 | 解析器 | 载体 | 解码 | 回写 |
|---|---|---|---|---|
| `.rpf` | `GameFiles/RpfFile.cs` | 容器 | 完整 | 完整（含重建 TOC、扩档、碎片整理） |
| `.ytd/.gtxd` | `FileTypes/YtdFile.cs`+`Resources/Texture.cs` | RSC | 完整（BC7/DXGI、Gen9 `ShaderResourceViewG9`） | 有 `Save()` |
| `.ydr/.ydd/.yft` | `Ydr/Ydd/YftFile.cs`+`Drawable.cs` 7,190 行、`Frag.cs` | RSC | 完整（ShaderGroup/VertexDeclaration/IndexBuffer/Skeleton） | 有 |
| `.ybn` | `YbnFile.cs`+`Bounds.cs` 5,488 | RSC | 9 种 `BoundsType` 全含 BVH（`Bounds.cs:115,2508`） | 有编辑器面板 |
| `.ymap` | `YmapFile.cs` 3,666 | META | 完整 | 有，但 2 类数据丢失（见下） |
| `.ymt` | `YmtFile.cs` | META/PSO/RBF 三态嗅探（`YmtFile.cs:84-120`） | 场景区域完整 | 部分 |
| `.ynd` | `YndFile.cs` 1,913 | RSC `NodeDictionary` | 完整 | 有 `Save()` |
| `.ynv` | `YnvFile.cs` 1,406 + `Resources/Nav.cs` | RSC | poly/point/portal/edge | `Builders/YnvBuilder.cs` 可**生成** |
| `.ycd` | `YcdFile.cs`+`Clip.cs` 4,466 | RSC v46 | 完整 | 有 |
| `.ymt/.ytyp/.cut` | `MetaTypes/*` | META | 完整 | 有 |
| `.rel`（音频） | `RelFile.cs` **24,909** | 自有容器 | 含脚本 VM 反汇编 | 8 处 TODO |
| `.awc` | `AwcFile.cs` 3,169 | 二进制 | 解码+PCM 合成 | 有 |
| `.fxc` | `FxcFile.cs` 1,896 + `CodeWalker/GameFiles/FxcParser.cs` | DXBC | 含反编译文本 | 否 |
| `.mrf`（move network） | `MrfFile.cs` 5,613 | `MoVE` | 部分（`NotImplementedException` ×5） | 否 |
| `.ypdb`（AI pose db） | `YpdbFile.cs` 411 | 二进制 | `PoseMatcherMatchSample`/`WeightSet` | 否 |
| `gta5_cache_y.dat` | `CacheDatFile.cs` 1,074 | 文本+结构 | LOD 层级/interior/bounds store | — |
无独立 `ShaderGroup.cs`/`PhysicsSystem.cs`/`Gxt2.cs`/`PagedFile.cs`：ShaderGroup 在 `Drawable.cs:14`，物理在 `Frag.cs`，GXT2 为 `Gxt2File.cs`，分页语义由 `RpfResourceFileEntry` + `ResourcePagesInfo` 承担。

### RpfArchive 深剖
头部 16 B：`RPF7` 版本、EntryCount、NamesLength、Encryption 四类（`RpfFile.cs:140-143`；枚举 `RpfFile.cs:2147-2152`：`OPEN=0x4E45504F`、`AES=0x0FFFFFF9`、`NG=0x0FEFFFFF`）。TOC 与名字表整体用密钥解密后，逐条 16 B 表项按第二个 uint 判别类型：`0x7fffff00`=目录、bit31=0=二进制文件、否则资源文件（`RpfFile.cs:192-208`）；名字通过 `NameOffset` 从 names blob 取 NUL 串（`RpfFile.cs:216-217`），再由 `EntriesIndex/Count` 栈式遍历生成路径树（`RpfFile.cs:239-268`）。资源表项为 24-bit `FileSize` + 24-bit `FileOffset`（**扇区=512 B**，`StartPos + FileOffset*512`）+ `SystemFlags/GraphicsFlags`；当 `FileSize==0xFFFFFF` 时真实长度藏在 RSC 头 16 字节的第 `7,14,5,2` 字节里，源码原话 `//just....why`（`RpfFile.cs:2650-2660`、`1683-1690`）。页大小被压缩进 32-bit flags：低 4 位 `BaseShift` 给 `BaseSize=0x200<<shift`，其后 1/2/4/6/7 bit 五档页计数（上限 `0x7F,0x3F,0xF,3,1`）与 4 个 tail 位，高 4 位是版本（`RpfFile.cs:2732-2760`）。
加密：AES 路径是 **Rijndael-256/128-ECB 无填充**（`GTACrypto.cs:47-68`）；NG 路径是 17 轮查表密码，密钥按 `(jenk(name) + length + 61) % 0x65` 从 101 组扩展密钥中选取（`GTACrypto.cs:96-101`）。**仓库不含密钥本身**：`GTAKeys.cs:22-24` 明确 "This file contains only SHA1 hashes of all interesting values. There is NO key."，运行时以 1 MB 并行块、8 字节对齐在 `GTA5.exe` 中扫 SHA1 匹配来定位 AES key、101 组 NG 密钥、17×16×256 解密表与 `PC_LUT`（`GTAKeys.cs:71-152`，`HashSearch` 在 `740-780`，共 375 个 SHA1 字面量）；加密表则由解密表反解：`RandomGauss.Solve()`（GF(2) 线性求逆）与 `LookUpTableGenerator.BuildLUTs2()`。结果被 deflate + AES + 以 `JenkHash(PC_AES_KEY)` 为种子的 4 路伪随机流加性混淆成 `magic.dat`（`GTAKeys.cs:228-244` / `264-313`）。写档同样完备：`WriteHeader`（`958-1003`）、`GrowArchive`/`RelocateFile`/`FindHole`/`Defragment`/`SetEncryptionType`。

### 分块标签与哈希机制
GTA V 的 RSC 资源不用 chunk fourcc 表，而是**虚地址 + 类型标签**：`ResourceDataReader` 用 `SYSTEM_BASE=0x50000000`、`GRAPHICS_BASE=0x60000000` 两段流做统一寻址（`ResourceData.cs:45-46,127-167`），`blockPool` 按偏移去重（`:172-216`），多态块经 `IResourceXXSystemBlock.GetType()` 以枚举/`VFT` 分派（`Bounds.cs:494-517`）。每块首字段是 `VFT`——原样保存的 C++ 虚表指针：`ShaderGroup.VFT = 1080113136`（小端即 `"0ET@"`，`Drawable.cs:22`），`Texture.cs:1342-1370` 甚至列出实采的 exe 虚表 RVA 集合 `0x1406b77d8 / 0x140695e58 / …`，`Clip.cs:3060`、`Frag.cs:3980` 同类。名字侧才是"known-hash 表"：`MetaTypes/MetaNames.cs` 的 `enum MetaName : uint` 含 **20,239** 条 `名字 = jenkins哈希`，其生成器 `MetaNamesUtil.GetNameEnumCode()`（`:28-48`）与 `MetaTypes.GetTypesString()`（`MetaTypes.cs:116-200`）把运行时从游戏 typeinfo 读到的键反查为源码；`JenkIndex`（`Jenk.cs:199-262`）在解析过程中自学习（`RpfManager.cs:353+` 扫描全部 RPF 名字、`YmapFile.cs:114` "just shove them in there"）。

### MetaBase 反射式类布局镜像
本版本已无 `MetaBase.cs`（题设需修正）：改为 **显式 struct + 手写 typeinfo 表** 的双轨镜像。游戏 META 文件自带 `MetaStructureInfo/MetaEnumInfo`（`Meta.cs:231-307`、字段类型码 `MetaStructureEntryDataType`：`Hash=0x4A, Array=0x52, Structure=0x05, Float_XYZ=0x33 …`，`Meta.cs:331-355`），`MetaTypes.EnsureMetaTypes()` 把每张 `.ymt` 的 typeinfo 汇入全局 `StructDict/EnumDict` 并做一致性比对（`MetaTypes.cs:30-70`）。C# 侧结构体注释直接写着 `//128 bytes, Key:1825799514`（`CEntityDef`，`MetaTypes.cs:2766`），配合 `[TC(typeof(EXP))]` 仅用于 PropertyGrid 展示，序列化靠 `Marshal.StructureToPtr/GCHandle` 零拷贝（`MetaTypes.cs:1437-1520`）；`MetaBuilder.GetMeta()` 再把 `GetStructureInfo(MetaName)` 里的**手写**布局表（`MetaTypes.cs:297-…`，含偏移、类型、引用键）写回文件的 StructureInfos 段，使游戏能解释这些块。`MetaWrapper` 抽象类（`MetaTypes.cs:2085-2095`）定义 `Load(Meta, MetaPOINTER)` / `Save(MetaBuilder)`，即 36 个 `MC*` 包装类的双向映射入口。

### 序列化往返保真度
解码/编码严格成对：每个块 `Read()` 逐字段读、`Write()` 先把指针与计数回算再写（如 `ShaderGroup` 的 `ShadersCount1/2`、`ShaderGroupBlocksSize`，`Drawable.cs:57-127`）。未识别字节一律保留为 `Unknown_XXh` 原值（Core 中 **822** 处）并原样写回，属"保真优先"策略。写出时 `ResourceBuilder.Build()` 对每个块断言 `pos_after-pos_before == BlockLength`（Gen9 用 `BlockLength_Gen9`，20 处，如 `Texture.cs:887-888` 144↔128），否则抛 "error in system length"（`ResourceBuilder.cs:558-585`），并按页装箱算法（`:331-430`）重算 `System/GraphicsFlags` 后拼 `RSC7` 头。META 侧条目强制 16 B 对齐以免指针带子偏移（`MetaBuilder.cs:47-63`）。缺口有自我披露：`YmapFile.cs:685-686` `LogSaveWarning("containerLods were not saved. (TODO!)")`；`RpfFile.cs:1708` `//TODO: binary encryption`。

### 运行时与离线分界
全部为离线进程：`Program.cs:20-50` 解析 `menu/explorer/project/vehicles/peds` 参数选择窗体；`CodeWalker/Rendering/DirectX/DXManager.cs` + `ShaderManager.cs` 加载 `Shaders/*.cso`；世界流送用 `GameFileCache`（`ConcurrentQueue` 请求队列、`Cache` LRU 512 MB/10 s，`Utils/Cache.cs:13-14`）+ `gta5_cache_y.dat` 的 LOD 树（`GameFileCache.cs:1251-1270`）；编辑面板含 ymap entity/cargen/grass/lodlight、ynd、ynv(poly/point/portal)、ybn、ytyp(MLO room/portal/entityset)、scenario ymt、train track、audio zones。仓库内**无任何注入式 .asi/C++ 游戏内组件**，`ExploreForm.cs:2269` 反而提示"Loading by the game will require a mod loader such as OpenRPF.asi or OpenIV.asi"。`CodeWalker.ModManager`(1,825 行) 只做 mods 文件夹安装；`Gen9Converter` 调 `Tools/ConvertAssetsForm` 做 PC→Enhanced 资源转换（`ShadersGen9Conversion.xml` 为手建参数名映射表）。

### 逆向来源标记
`Notice.txt`（Neodymium/Cameron Berry/hamish-milne）；`RelFile.cs:16-20` "adapted from CamxxCore's RageAudioTool"；`Todo.txt` 引用 gtaforums timecycle 帖与 RageAudioTool 仓库；`DDSIO.cs:2`（Microsoft/D3DX 派生）；`MenyooXml.cs:32-33` 引用 me2ymap/Guad MapEditor；`GTAFolder.cs:151` 读 OpenIV 注册键；`Bounds.cs:761,836`、`GameFileCache.cs:609` 等大量 "hack/TODO" 自述；虚表 RVA 常量（`Texture.cs:1342`）与 `(MetaName)3844724227` 这类未命名哈希（`MetaTypes.cs:351`）是最直接的"来自二进制"证据。

### 成熟度与缺口
| 级别 | 项 |
|---|---|
| 完整（读+写+UI 编辑） | RPF/RSC 容器、AES+NG 双向、ytd/ydr/ydd/yft/ybn/ycd/yld/ymap/ytyp/ynd/ynv(+ynv 生成器)/ymt 场景区 |
| 读优先、写弱或缺失 | rel(音频 VM)、mrf、awc 合成、fxc 反编译、ypdb、gxt2、heightmap/watermap |
| 明确未做 | 非 META 的 ymap/ped 变体（`YmapFile.cs:239`）、containerLods、二进制条目加密、NavMesh 编辑器之外的 collision 编辑（`Todo.txt` 列表） |
量化：Core 内 TODO 124 处、`Unknown_` 字段 822 个、`NotImplementedException` 10 处（集中在 Mrf/ResourceBaseTypes）。

### 对 GTA VI 逆向的方法论启示
1）"哈希表即格式"：把 20,239 条 jenkins 名表与 375 条 SHA1 密钥指纹作为可再生资产维护，密钥不进仓库、由目标 exe 自举提取——同样适用于 Gen9/Next 的密钥轮换。2）类型元信息优先：GTA V 的 META 自带结构描述，因此先做"运行时自描述→自动生成 C# 布局"的工具链（`GetTypesString()`），比手猜字段可靠。3）保真写回是验证手段：以 `BlockLength` 断言 + 16 B 对齐 + 页装箱重算，能立刻暴露布局错误；`magic.dat`/`.cso` 的"源码+少量二进制"分层分发也值得沿用。4）虚表 RVA 集合可作为跨版本识别锚点（推断）。
