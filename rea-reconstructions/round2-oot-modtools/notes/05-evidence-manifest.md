# 5. 第二轮 REA 取证结果与证据清单

命令：`/usr/local/bin/rea`（第一轮为 6.9.0；本轮同一安装）。所有原始输出落盘在本目录同级 `../evidence/`，
`.gitignore` 已屏蔽 `rea-reconstructions/**/*.json`，**大体积 bundle 不入库**。

---

## 5.1 逐工件身份（F）

| 工件 | 大小 | SHA-256 | 格式/架构 | 导入 | imphash(md5) | 签名 |
|---|---|---|---|---|---|---|
| `audio-model/gfxasm/bin/win32/gfxasm.f3dex2.exe` | 65,536 | `b222470dd8171517b2e31eacb1acb251b8c7384eae98bbbcf51a2562f0508ca2` | PE, machine **332 (i386)** | `KERNEL32.dll`, `msvcrt.dll` | `17326f9b13060ea6dd2eccac9708e87c` | 无 |
| `audio-model/z64ext/bin/util/minasm.exe` | 78,336 | `da9b4ed465daff2cfb4fc8c31a0f9a1ef94ec5f38e3e8066c9890c598a503e2e` | PE, 332 | `KERNEL32.dll`, `msvcrt.dll` | **`836e423864b7d5f109a372e6e87e1c69`** | 无 |
| `audio-model/z64ext/bin/util/minasm` | 88,672 | `608a10ffc35f4b4d50f4bf9a70e69d7afe018da8ad4df8bebcc2f0c1d4fbca9b` | **ELF, machine 62 (x86-64)** | — | — | — |
| `audio-model/CMB-Exporter/cmb_internal/tools/windows/etc1compress.exe` | 60,416 | `49a7d1e6088f97ba17cdd0611bb4aba84a9da2de60ea509b835194f303e0b07a` | PE, machine **34404 (amd64)** | `MSVCP140`, `VCRUNTIME140(_1)`, `api-ms-win-crt-{runtime,heap,convert,stdio,filesystem,string,math}` | `8846c1010cee843eee6aeb981065ca14` | 无 |
| `audio-model/CMB-Exporter/cmb_internal/tools/linux/etc1compress` | 54,568 | `23d803b02d133e7803501a81ae96cd2a44206eac1f5ed707ec5d39f0d20b0d00` | ELF, 62 | — | — | — |
| `audio-model/N3DSCmbViewer/N3DSCmbViewer/ETC1Lib.dll` | 17,408 | `ce1c34bccfb0fc66869b5eebaa86024247aa2acb9341cc425aa05455d0e560a7` | PE, 332 | **`MSVCR100.dll`**, `KERNEL32.dll` | `15d915044f77aac50ab76b4ae391817d` | 无 |
| `audio-model/anim2link/source/{obj,bin}/Debug/anim2link.exe` | 30,208 | `3114142c148c6a06883743497d3bc04841364c6e7206778ac1aefccf71a5aa73` | PE / **.NET metadata v4.0.30319**, x86 | — | — | — |
| `audio-model/anim2link/source/{obj,bin}/Release/anim2link.exe` | 29,184 | `c5689faa081ab337837b3c49175d5a751ffc23f0adcc0fe0d851ebf753584b55` | 同上，MVID `1508620e-6c98-418a-a678-a9851a12d981` | — | — | — |
| `save/save-file-converter/.../psp-encryption.wasm` | 70,000 | （见 §5.4） | WASM | — | — | — |
| `patch-otr/RomPatcher.js/`（目录） | 928 KB | root `43e96a4b801b9fa74112a109d2db15b74bcd0c08a9da0f4f4c04d52d94b17d2c` | JS 应用 | — | — | — |
| `oot-peripherals/z64-sample-creation-tools/z64audio.exe`（第一轮仓库内） | 2,757,038 | `3d173b43b807a7205c290149c92915fd5c6aedfbbe72ba92832c45b6df4965bf` | PE, 332 | `GDI32/KERNEL32/msvcrt/OPENGL32/SHELL32/USER32` | `0a0da823eab2ae5675eeaa71d57ae838` | 无 |

`evidence_id` 摘录：`inspect-artifact` gfxasm `ev_7c18f185…5ad6b`（manifest `agm_7f47935e…`，
root `art_168aa602…`，`graph_sha256 99624007a283aea80680393b8cec703fb7859384d1b9c55abfd3e8c85631e921`，
`node_count 1 / edge_count 0`）；`inspect-pe-imports` gfxasm `ev_bc09a5b7…ceb5`、
minasm.exe `ev_ecfc7936…8031`、etc1compress.exe `ev_2b568b93…05ae`、ETC1Lib.dll `ev_6540a0b4…b314`；
`inspect-managed-members` Release `ev_2a62a193…9b5b`；
`inspect-pe-certificates` z64audio `ev_9384ccd5…6813`。

## 5.2 加固矩阵（F，`inspect-binary-hardening`）

| 工件 | dynamic_base | high_entropy_va | nx_compat | force_integrity | no_seh | guard_cf | cfg_instrumented | authenticode | debug_record |
|---|---|---|---|---|---|---|---|---|---|
| `gfxasm.f3dex2.exe` | **disabled** | n/a | **disabled** | disabled | disabled | disabled | unknown（无 load config 目录） | **disabled** | **disabled** |
| `minasm.exe` | **disabled** | n/a | **disabled** | disabled | disabled | disabled | unknown | **disabled** | **disabled** |
| `etc1compress.exe` | **enabled** | **enabled** | **enabled** | disabled | disabled | disabled | **enabled**（GuardFlags 有 `IMAGE_GUARD_CF_INSTRUMENTED`） | **disabled** | **enabled** |
| `ETC1Lib.dll` | enabled | n/a | enabled | disabled | disabled | disabled | unknown（Load config `Size` 未覆盖 GuardFlags） | **disabled** | enabled |

ELF 侧（`/tmp` 无关，均为 Linux x86-64）：

| 工件 | PIE | NX | RELRO | stack protector | FORTIFY |
|---|---|---|---|---|---|
| `etc1compress`（ELF） | enabled（`ET_DYN` + `DF_1_PIE`） | enabled | **full**（`PT_GNU_RELRO` + `BIND_NOW`） | **enabled**（动态符号有未定义 `__stack_chk_fail`） | 未发现符号 |
| `minasm`（ELF） | enabled | enabled | **partial**（`PT_GNU_RELRO` 有、`BIND_NOW` **未**置位） | **未发现** `__stack_chk_fail` | 未发现符号 |

⇒ **本轮可交付的加固结论**：同一 `etc1compress` 源码在 Linux/Windows 两侧都拿到了现代缓解项，
而 z64.me 生态的 `gfxasm/minasm` 是 **32 位、无 ASLR、无 DEP** 的老式 MinGW 产物；
四个 Windows 镜像**全部无 Authenticode 目录**（`IMAGE_DIRECTORY_ENTRY_SECURITY` 地址与大小均为 0）。
REA 自身声明的边界（照录）："PE control-flow guard state is the GuardFlags linker claim, not a check that
CFG checks run"、"A missing stack-protector or fortify symbol does not establish that the protection is absent"、
"This inspection reads headers, program tables, dynamic symbols, and imports. It does not execute the image
or assess exploitability."

## 5.3 `anim2link.exe`（Release）托管成员（F）

- assembly `anim2link` **version 1.0.0.0**，`flags 0`，**未签名**（无公钥），metadata **`v4.0.30319`**，`kind: none`。
- **TypeDef 7 / Field 38 / MethodDef 46 / Param 45 / MemberRef 151 / TypeRef 91 /
  CustomAttribute 37 / Property 8 / PropertyMap 4 / MethodSemantics 14 / StandAloneSig 13 /
  TypeSpec 2 / NestedClass 1 / ManifestResource 2 / AssemblyRef 5**；
  语义图 **fields 38 / methods 46 / member_refs 151 / call_edges 509**；`exported_types: []`。
- 类型全集：`<Module>`、`anim2link.Form1`、`anim2link.Processing`、`anim2link.Program`、
  `anim2link.Properties.Resources`、`anim2link.Properties.Settings`、`Animation`。
- 关键字段/方法名（可直接当接口读）：`Proc`、`Animations`、**`Anime_Size`**、`Do_Not_Alert`、
  `<FloorPlane>k__BackingField`、`btnExportAll_Click`、`btnRefresh_Click`、
  `add_Load/add_Click/add_CheckedChanged/add_SelectedIndexChanged/add_Tick`、`checkBox1`、`listView1`、`groupBox1`
  ⇒ **WinForms 单窗体工具**，`Anime_Size` 即 README 所述"检测尺寸、只增不减"逻辑的落点。
- Debug↔Release：`obj/**` 与 `bin/**` 内容两两相同（构建产物被提交），Debug 成员表更大（9,851 vs 9,208 行 YAML）。
- REA 托管侧限制（照录 6 条要点）：元数据 token 是**构建局部坐标**，只有在报告的 SHA-256 + MVID 下才有意义；
  CIL 指令锚点仅从**文件承载的方法体**解码，**不加载、不执行**目标程序集；
  Field 常量取自 Constant 表而非运行时值；#US 字符串只覆盖被 `ldstr` 加载的字面量。

## 5.4 WASM 边界（U）

`inspect-wasm-artifact` 仍不可用：需要 `REA_WABT_BIN_DIRECTORY` 指向含
`wasm-validate / wasm-objdump / wasm2wat` 且 `--version` ∈ [1.0.42, 1.1.0) 的目录；
REA 明确"shell export 不会更新已运行的 MCP server"且"不得隐式安装工具" ⇒ 本轮**未安装 WABT**。
自行解析的头部事实（不依赖 WABT）：
- 第一轮 `ootr-tracking-map/.../pocketsphinx.wasm` 449,555 B，magic `\0asm` version 1，
  段尺寸表 `(1,680)(2,341)(3,2001)(6,9)(7,212)(9,1100)(10,355269)(11,89910)`；
- 第二轮 `save-file-converter/.../psp-encryption.wasm` 70,000 B（PSP 存档加密，与 OoT 无关）。
⇒ **段级/函数级 WASM 还原整体记 U**，并在报告 §方法学里作为固定限制陈述。

## 5.5 `RomPatcher.js` JS 应用图（F）

见 `02-rompatcher-otr-textures.md §2.7`：relevant 29 / parsed JS 25 / AST 49,307 / findings 123 /
`graph.coverage.status = partial` / 语义图 unknown 23,685 / bundle 73,860,123 B；
REA 自报盲区为 `index_template.html` 第 16 行起的 1 个内联脚本。

## 5.6 与第一轮的对照（方法学增量，F）

| 维度 | 第一轮（工单 #4，专有/打包产物） | 第二轮（工单 #8，开源周边） |
|---|---|---|
| 主要可用命令 | `inspect-artifact`、`inspect-pe-imports`、`inspect-managed-members`、`analyze-javascript-application`（出厂 bundle）、`build-reconstruction-obligation-ledger` | `inspect-artifact`、`inspect-pe-imports`、`inspect-pe-certificates`、`inspect-binary-hardening`、`inspect-managed-members`、`analyze-javascript-application`（**源码树**） |
| 证据来源 | 100% `shipped-artifact`（130 条义务全部） | 仍以 shipped-artifact 为主，但**规范可由源码/文档直接锚定**，故接口还原不再依赖 bundle 反推 |
| 义务台账 | 130 条，**0 可验证**，129 无 owner | 本轮**未建台账**：目标是开源代码，"重构义务"应表达为"上游已给实现"，与台账语义不匹配（决定见报告 §6） |
| 失败/不可用 | minisign 签名不可验（记 `?`） | WABT 缺失 ⇒ WASM 记 U；`--evidence_output` 仅 `analyze-javascript-application` 支持（其余命令报 `Unknown flag`） |
