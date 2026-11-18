# 4. 自定义模型导入链：OBJEX（.obj 扩展方言）→ zzconvert → N64

> 来源：`audio-model/io_export_objex/objex_spec.md`（**816 行**，规范原文）、
> `io_export_objex_2.5/`（Blender 2.79→3.6 导出插件，GPL）、`objex_for_2.79/`、
> `anim2link/`（C#）、`gfxasm/`。全部为源码/文档静态阅读，记 **F**（可指行）。

---

## 4.1 版本契约（F，`objex_spec.md:3-18`）

- 当前版本 `2.major = 2.0`，插件与 **zzconvert** 采用 `2.major.minor`。
- **`major` 必须两边一致**；`minor` 只允许"纯插件/纯 zzconvert 修复"；
  `major` 递增条件被明确写成 "**objex 规范本身变了**"。
- 缺失 `version` ⇒ zzconvert 提示升级 Blender 脚本；不是 `2.%d` ⇒ 提示升级 zzconvert（`:34-36`）。

⇒ 这是一套**显式的双端版本握手**，比"靠猜"的常见 mod 工具链严谨得多，可直接借用为接口约定样板。

## 4.2 相对 `.obj` 的扩展指令全集（F）

| 指令 | 语义（要点） | 位置 |
|---|---|---|
| `skellib` / `animlib` | 类 `mtllib`；**必须先 skellib 后 animlib**；可多行各一；skellib 可脱离 animlib，反之不行 | `:21-28` |
| `version` | `version 2.major` | `:30` |
| `exportid` | **强制**；objex 及其 `mtllib/animlib/skelib` 全部文件必须**同一** `exportid`；须在 `*lib` 之前；官方建议值为"导出时刻的字符串表示"，用于发现"某文件没写成功" | `:38-52` |
| `softinfo one_tag string` | 目前仅 `animation_framerate`，**不是 `20` 就报警** | `:53-64` |
| `useskel` / `clearmtl` / `attrib` | 骨骼引用 / 显式无材质 / 自由文本组属性（可多行，每行以 `attrib` 起） | `:64-84` |
| `priority number` | 整数、可为负；**高优先级先写**，默认 0；同优先级顺序未定义 | `:85-95` |
| `origin x y z` | 导出时世界坐标；写了就把网格平移到原点 | `:96-104` |
| `v x y z weight "骨名" 影响度 …` | **顶点权重内联在 v 里**；单骨时影响度 `1.0` | `:105-118` |
| `vc r g b a` | 顶点色（含 alpha），取值 **0.0..1.0 闭区间** | `:119-124` |
| `f v/vt/vn/vc` | 面可点名引用 `vc`；**同三角形内可混用顶点色与着色，游戏内插值**（N64 特性） | `:125-131` |

**平台硬约束（F，`:113-118`）**：zzconvert 要求"绑定到骨骼的顶点**只能有一个 1.0 影响度的骨**"，
否则明确报错 **"N64 skeletons don't support vertices influenced by multiple bones"**
⇒ N64 皮肤矩阵只有单骨权重，这是导入失败的最常见根因，规范把它写成了显式条款。

## 4.3 纹理子系统（F，`newtex`，`:140-237`）

- `newtex name` + `map filename`（**余下整行都是路径 ⇒ 文件名可含空格**）。
- **`pointer 0xPointer`**：不写转换后地址而写**指定的 32 位指针**；
  用例是"由 RAM segment 控制的指针"（例：Link 眼睛 `0x08000000`）与"引用别的文件里的纹理"。
  - 唯一非法值是 `0`（表示未设置）；必须是十六进制。
  - **发现（`?`）**：文档说"32-bit value"，紧接的例子却是 `0x0400000400`（**40 位 / 10 个十六进制位**，注为
    "Hylian Shield, rgba5551, 32x64"）⇒ 规范自身笔误或"段号+偏移"混写，**不可按字面实现**。
- **`format fmt`**：9 种 —— `rgba32, rgba16, ia16, ci4, ci8, ia4, ia8, i4, i8`
  （与 zzrtl 的 `n64texconv.c` 实现集**完全一致**：`onebit/rgba5551/rgba8888/ci4/ci8/i4/i8/ia4/ia8/ia16`，
  命名略有差异：`rgba16≡rgba5551`、`rgba32≡rgba8888`）⇒ **F：两个仓库互相印证同一格式集**。
- **`alphamode mode`**：`edge` / `average` / `white` / `black` / `image`
  （`image` 在 `ci` 格式且不可见像素唯一色 >4 时**回落到 `edge`**）；
  缺省：**I 系格式 = `image`，其余 = `edge`**。
- **`palette number`**（仅 ci 格式，须 > 0）：**同槽多纹理会合成"优化调色板"**并回写所有引用该槽的纹理。
- `texturebank filename`：**像素数据取自另一个文件**，但尺寸/映射仍按 `map` 的图；
  未指定 format 时**默认 `ci8`**；用例为"动画眼睛纹理、昼夜窗纹理"这类**未被引用的条带状纹理**。
- `forcewrite` / `forcenowrite`：**无视引用关系**强制写入 / 强制不写入。
- 纹理也有 `priority`：用例明写"**必须落在文件内特定偏移的纹理**（如 Link 的眼、嘴）"。

## 4.4 材质与显示列表注入（F，`newmtl`，`:238-340`）

- 材质级 `priority`：用例 = "含透明的三角形要**最后**写进 OPA，避免绘制问题"。
- `vertexshading mode ∈ {color, normal, dynamic, none}`；
  `dynamic` 自动判定（`vc` 恰为 `0xFFFFFFFF` 不透明白时按法线处理），
  并**据此切换几何模式 `G_LIGHTING` 标志**（`:270-277`）⇒ 规范级地把"顶点色 vs 光照"绑定到 GBI 标志。
- **`gbi gbiMacro`**（`:279-292`）：材质之后的每一行 `gbi` 都被当作 **F3DEX2 GBI 宏**参与该材质的显示列表构造，
  并支持 zzconvert 变量替换：
  `_texelXaddress / _texelXformat / _texelXbitdepth / _texelXwidth / _texelXheight /
   _texelXwidth-1 / _texelXheight-1 / _texelXmasks / _texelXmaskt`（X = 0|1，对应两个 texel）。
  另有 `gbivar`（`:314`）、`standalone`（`:334`）、`empty`（`:342`）、`texel0/texel1`（`:354`）。
- ⇒ **这就是"自定义模型如何拿到任意渲染指令"的官方通道**：文本 DSL → GBI 宏 → 显示列表，
  与 `gfxasm/`（独立 **F3DEX2 汇编器**，其 `bin/win32/gfxasm.f3dex2.exe` 见报告 §3 REA 取证）构成同一层的两种用法。
  `z64ext/src/asm/c/gbi.h` 提供完整 GBI 宏定义（例：`Z_CMP/Z_UPD` 位定义 `:136-137`），供 minasm 汇编使用。

## 4.5 `.skel` / `.anim` 文件（F，`:387-551`）

```
newskel "name" "extra"
segment 0xCustomSegment [local]
pbody [parent "anotherSkeleton" "boneName"]
```
- `extra` 的**游戏语义枚举**：`"z64player"` = Link 骨架格式；`"z64npc"` = 非 Link；
  `"z64dummy"` = 只用来产出 **Pbody**（自包含可变形网格）的迷你骨架。
- `segment 0xSegment [local]`：定制**变形矩阵存放段**；必须十六进制；带 `local` 时矩阵视为
  "本地、以单位矩阵初始化"，且**由用户保证该地址可被覆盖**；
  **缺省 `0x0D000000`**（例：`segment 0x06005800 local`、`0x0B000000`）。
- **骨骼书写格式**：`(bones * 2)` 行，每行首个非空白字符是 `+`（push）或 `-`（pop），
  push 后跟 `"name" pos.x pos.y pos.z`（**相对父骨位置**）；`+` 与 `-` 数量必须相等 ⇒
  **用缩进 + 进出栈表达骨骼树**，而不是索引表。
  样例（规范原文节选）：
  ```
  newskel "player" "z64player"
  + "RootControl" 0.000000 0.000000 -0.000000
      + "Waist" -0.004000 -0.104000 -0.000000
        + "LowerControl" 0.607000 0.000000 -0.000000
          + "Thigh.R" -0.172000 0.050000 -0.190000
            + "Shin.R" 0.697000 0.000000 -0.000000
  ```
  ⇒ 这份骨名表（RootControl/Waist/LowerControl/Thigh.R/Shin.R…）就是 OoT 玩家骨架的权威命名（F）。
- `.anim`：`newanim`（`:512`）；`softinfo animation_framerate` 固定 20。
- 组命名约定（`:552-620`）：`collision`、`colliders`、`collider.joints`、`collider.cylinder` ⇒
  **碰撞体是用组名（而非独立文件）从模型里"认领"出来的**；`pbody` 提供物理体。

## 4.6 anim2link：`.anim` → Link 动画（F，C#，含 4 个提交进仓库的 exe）

- 用途（README 原文）："convert `.anim` files exported by the OBJEX exporter to
  Link's animation format used by Ocarina of Time or Majora's Mask"。
- **实现限制（README 原文）**：自动注入器**不能覆盖比目标更小的动画**；
  "如果你用更小的动画覆盖原动画，新动画的尺寸仍是先前导入的那个大" ⇒ **尺寸只增不减**（F）。
- 致谢：**"rankaisija ported/created the `Fortnite_Default_Dance` animation"**
  ⇒ 与 §3.2.1 里 `z64audio.exe` 的构建路径 `/home/rankaisija/mxe/...` **同一作者 handle**
  ⇒ 交叉印证 z64.me 生态（z64audio / anim2link / z64ext / zzrtl 系列）围绕同一批维护者（F，两条独立证据）。
- 仓库里 `source/obj/**` 与 `source/bin/**` 各有 Debug/Release `anim2link.exe`：
  | 文件 | SHA-256 |
  |---|---|
  | `obj/Debug/anim2link.exe` | `3114142c148c6a06883743497d3bc04841364c6e7206778ac1aefccf71a5aa73` |
  | `bin/Debug/anim2link.exe` | 同上（**逐字节相同**，符合 MSBuild 复制行为） |
  | `obj/Release/anim2link.exe` | `c5689faa081ab337837b3c49175d5a751ffc23f0adcc0fe0d851ebf753584b55` |
  | `bin/Release/anim2link.exe` | 同上 |
  ⇒ 只有 **2 个不同镜像**（Debug 一对、Release 一对），30,208 / 29,184 B；
  属"构建中间产物被误提交"（卫生问题，非安全问题）。`anim2link.exe` 的 REA 取证见报告 §5.3。

## 4.7 管线全景（本轮增量结论）

```
Blender (io_export_objex 2.x，导出 .objex/.mtl/.skel/.anim)
   │  exportid 一致性校验、vertexshading、gbi 宏
   ▼
zzconvert  ──(格式集 = ci4/ci8/i4/i8/ia4/ia8/ia16/rgba16/rgba32)──►  N64 显示列表 + 纹理 + 骨架
   │  anim2link：.anim → Link 动作（尺寸只增不减）
   │  gfxasm：F3DEX2 汇编（独立 exe，minasm = minimips64 的前身）
   ▼
zzrtl（文件系统 dump/inject + cloudpatch） + z64ext（运行时扩展 DMA/解压/表扩容）
   ▼
N64 实体卡 / Wii VC；音频侧由 z64audio(.book.bin/.loopbook.bin/.vadpcm.bin) → seq64 音源库
```
（各节点均有上表源码依据；`zzconvert` 本体**未在本轮取得源码** ⇒ 记 U，见 §4.8）

## 4.8 未找到 / 边界

- **U**：`zzconvert` 可执行/源码未在克隆集合内（规范文档描述其行为，但转换实现未取证）。
- **U**：`.anim` / `.skel` 的**二进制**产物布局（文本规范已还原，二进制段表未还原）。
- **U**：`z64ext` 需 `oot-debug.z64`（README BUILDING 第 1 步："Place your copy of oot-debug.z64 (**not provided**)"）
  ⇒ 无 ROM 即无法验证其运行时行为；本轮只做静态接口还原。
- **?**：`objex_spec.md` 的 `pointer` 32/40 位自相矛盾（§4.3）。
- 本轮**未编译、未运行**上述任何工具（含 `anim2link.exe`、`gfxasm.f3dex2.exe`、`z64audio.exe`）。
