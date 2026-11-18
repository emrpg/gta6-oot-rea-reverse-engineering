## 3.9 citizenfx/natives + txAdmin —— 逆向知识的数据库化

本节关注 FiveM 生态中最不"代码化"、却最具复用价值的一层：把 IDA 里做出的结论沉淀成**可 diff、可校验、可 code-gen 的版本化数据集**。

### 3.9.1 布局：Markdown 文件即数据库行

`/root/orca-workspaces/gta-repos/citizenfx-natives`（浅克隆，1 commit，HEAD `c00e0ed`）共 6437 个 `.md`（`find` 计数），按 **44 个大写命名空间目录**平铺：`NETWORK/`(824)、`VEHICLE/`(751)、`PED/`(612)、`HUD/`(509)、`GRAPHICS/`(392)、`MISC/`(323)、`STATS/`(311)、`TASK/`(306)、`MONEY/`(301)、`AUDIO/`(276)、`PLAYER/`(248)、`CAM/`(245)，尾部如 `SAVEMIGRATION/`(3)、`LOCALIZATION/`(3)。**没有** gta5/rdr3/fourm/nyi 之类的 per-title 顶层目录——本仓库只收原版 GTA V native，见 README.md:21-29（CFX 命名空间另存于 FiveM 源码树）。

记录格式由 `.github/CONTRIBUTING.md:17-45` 约定、由生成器 codegen_out_markdown.lua:170-181 定型：YAML front matter（全库只出现两个键：`ns:` 6416 次、`aliases:` 2298 次）＋ `## NAME` ＋ 一个 ```` ```c ```` 块，其首行注释是 `// <64 位 native hash> <32 位 joaat>`，随后是 C 语法签名。两条与 HEAD 提交直接相关的记录逐字如下（WATER/AddExtraCalmingQuad.md:1-8、WATER/RemoveExtraCalmingQuad.md:1-8）：

````
---
ns: WATER
aliases: ["0xFDBF4CDBC07E1706", "_ADD_CURRENT_RISE"]
---
## ADD_EXTRA_CALMING_QUAD
```c
// 0xFDBF4CDBC07E1706 0xA9419B6D
int ADD_EXTRA_CALMING_QUAD(float xLow, float yLow, float xHigh, float yHigh, float height);
```
````
````
---
ns: WATER
aliases: ["0xB1252E3E59A82AAF", "_REMOVE_CURRENT_RISE"]
---
## REMOVE_EXTRA_CALMING_QUAD
```c
// 0xB1252E3E59A82AAF 0x45268B6F
void REMOVE_EXTRA_CALMING_QUAD(int extraQuad);
```
````

参数/返回类型的演进用 `cs_type(旧类型) 新类型` 标注以保持向后兼容（CONTRIBUTING.md:43，实例 VEHICLE/SetTrainSpeed.md:7）。文档正文不是结构化字段：`## Parameters` 5341 条、`## Return value` 2584 条、`## Examples` 292 条（Lua 282 / C# 107 / JS 92）。

| 标题 | 数据位置 | 记录数 |
| --- | --- | --- |
| GTA V (gta5) | citizenfx-natives 命名空间目录 | **6437**（其中 `N_0x*` 十六进制占位名 1003 条，15.6%） |
| CFX 扩展 | `citizenfx-fivem/ext/native-decls/*.md` | 889（`apiset: client` 546 / `server` 186 / `shared` 79，余为多值写法） |
| GTA V 引擎输入（按 build） | `ext/natives/natives_stash/gta_universal.lua` 76015 行、`gta_0193D0AF.lua` 68527 行、`gta_21E43A33.lua` 60120 行 | universal 内 `native "…"` 5180 条，带 `jhash` 4586 条 |
| RDR3 | `ext/natives/natives_stash/rdr3_universal.lua` | **0 字节（占位）** |
| ny / LibertyM | `ext/natives/natives_stash/ny_universal.lua` | **0 字节（占位）** |

### 3.9.2 数据周围的工具：三段流水线，校验器在仓库之外

| 阶段 | 入口 | 关键脚本 | 产物 / 去向 |
| --- | --- | --- | --- |
| PR 校验 | `.github/workflows/check-pr.yml:24-31` | `.ci/run-pr:11-14` 用 `git diff-tree` 取改动的 `.md`，逐个交 `node $BUILDER_ROOT/verify-one.js`（run-pr:37） | 通过/失败列表；失败即 `die` |
| 构建器安装 | `.ci/setup-builder:8-15` | `apt-get install libclang-14-dev` + **shallow clone `citizenfx/native-doc-tooling`** | 解析器与模板全在该外部仓库（FiveM 侧 `.gitmodules` 亦以 submodule 引用） |
| 部署 | `.github/workflows/deploy-repo.yml`（`publish` 分支，:47-48 rsync） | `.ci/run-deploy:18-21`：`node index.js <repo>` → `natives.json`；`build-template.js lua` → `natives.lua`；`build-template.js reference` → `natives.html` | docs.fivem.net/natives 与运行时 Lua 绑定 |
| 兼容层 | `.github/workflows/run-compatgen.yml`（`workflow_dispatch`，:78 `git fetch --shallow-since=<date>`） | `.ci/run-compat` → `compatgen.js --use-history --start-date`（默认 2023-01-01，`.ci/config.sh:4`） | `natives_global_client_compat.lua`：按 build 历史签名生成跨版本 shim |

**这里没有 IDA/Ghidra 工件**：全仓库无 `.i64/.idb/.idc/` 及重命名脚本（`find` 验证）——反汇编现场在别处，本仓库只存"结论 + 可执行 schema"。同时 CI **只做单文件语法校验**，不做全库一致性检查——这可由数据本身证实：64 位 hash 存在一对重复（`NETWORK/NetworkBlockJoinQueueInvites.md` 与 `NETWORK/N_0xcfeb8af24fc1d0bb.md` 同为 `0xCFEB8AF24FC1D0BB`），且 `status/verified/unverified` 一类字段在本库根本不存在，置信度只能靠散文表达（`unknown` 108 次、`unused` 66、`deprecated` 11、`has been removed` 1）。「推断」带 working/removed/unverified 状态的分级实际存在于 NativeDB/文档站侧，而非这份 Markdown 数据库。

### 3.9.3 HEAD 提交：一次 joaat 混用的数据完整性修复

JOAAT 即 Jenkins one-at-a-time，32 位；可直接读到的实现是 `citizenfx-fivem/ext/sdk/resources/sdk-root/shell/src/utils/joaat.ts:1-16`（先 `toLowerCase()`，再 `h+=c; h+=h<<10; h^=h>>>6`，收尾 `<<3 / >>>11 / <<15`），同一算法复制于 `ext/sdk/resources/sdk-game/src/shared.ts:27-43`；引擎 C++ 侧另有 FNV-1a 家族 `code/shared/fnv.h:80`（`0x811C9DC5/0x1000193`），与 joaat 不是一回事。

按该算法独立复算即得：`joaat("add_extra_calming_quad")=0xA9419B6D`、`joaat("remove_extra_calming_quad")=0x45268B6F`——与上面两条记录的第二个 hash **逐位吻合**。也就是说 DB 的不变量是 `jhash == joaat(小写名)`。HEAD 提交 `c00e0ed "Fix joaat mixup for RemoveCurrentRise (#732)"` 把 `0x7DBCEF6F` 改为 `0x45268B6F`，并把 `RemoveCurrentRise.md` 重命名为 `RemoveExtraCalmingQuad.md`、以 `aliases: [..., "_REMOVE_CURRENT_RISE"]` 保留旧社区名。复算显示旧值既不等于 `joaat("REMOVE_CURRENT_RISE")=0x7CDCA3CF`，也不等于库内任何记录的 jhash，「推断」它是主机版某个"把 rise 计数清零"的另一个 native 的 jhash，被误抄到了这条记录上。

为何这是数据完整性 bug 而非笔误：jhash 是"名字→ID"的唯一性契约，codegen 的 JSON 输出直接把它当字段发射（`ext/natives/codegen_out_json.lua:71`、解析入口 `codegen.lua:113-114`），错误值会把兼容层/文档锚点指向一个不存在或不同的入口，等价于一次与别的 native 身份的隐式碰撞。全库量化：4881 条带 jhash 的记录里 **4313 条满足不变量**，568 条不满足——其中 388 条是 `_0x…` 自动名、177 条是社区名（CONTRIBUTING.md:25 明确"社区命名与 hash 不匹配"，属预期），但仍有 **3 条真实命名记录不匹配**：`MISC/CancelOnscreenKeyboard.md`、`PATHFIND/GetPosAlongGpsTypeRoute.md`、`VEHICLE/SetVehicleExclusiveDriver.md`。它们至今未被 CI 拦下，反向印证 3.9.2 的结论；32 位 jhash 全库 **0 个重复值**。

### 3.9.4 txAdmin：FiveM 的运维层（次要）

`/root/orca-workspaces/gta-repos/txAdmin`，MIT 且版权归 **Take-Two**（LICENSE:3），npm workspaces `core/nui/panel/shared`，735 文件 / 572 个 ts·js·lua 源文件。它同时是一个 FXServer resource（fxmanifest.lua:11-12 `fx_version 'cerulean'` / `game 'common'`）和一个独立 Node 进程：`core/boot/getNativeVars.ts:31-38` 直接调用 `GetConvar/GetCurrentResourceName/GetResourceMetadata/GetResourcePath` 取宿主信息——即其启动本身消费 native 表面。管理面：`FxRunner/ProcessManager`（子进程启停）、`FxMonitor/index.ts:17-45`（heart-beat 与 health-check 双看门狗，`fatalLimit` 60 s / 180 s，崩溃/挂起自动重启）、`Database/instance.ts:17`（lowdb JSON，`DATABASE_VERSION = 5` + migrations）、`AdminStore/providers/CitizenFX.ts:6`（`idms.fivem.net` OIDC 登录）、`deployer/recipeEngine`、`whitelist`、`masterActions`、`cfgEditor`。最接近"反作弊/完整性"的是诊断上报：`core/lib/fxserver/scanMonitorFiles.ts:8-13` 对配置/资源文件做 NFKC 归一化后 SHA-1，限额 300 文件·20 MB·10 层深度。任务假设的 `baseline`、`sv_pure`、`gameResStart`、`backups` 子系统在本仓库**均未出现**：真正的 pure 内容校验在引擎侧（`code/client/shared/PureModeState.h:12-38` 解析命令行 `pure_<n>`；`code/components/citizen-server-impl/src/InitConnectMethod.cpp:399` 定义 `sv_pureLevel`；`net/src/NetLibrary.cpp:1716` 随 server info 下发），`getBackup.ts:22` 只是导出 `playersDB.json`。可见 txAdmin 定位为"运维 + 权限 + 可观测性"，不做内容校验。

### 3.9.5 数据库仓库 ↔ 引擎内消费

链接点是三条路径：`ext/natives/codegen_out_markdown.lua:170-181` 证明 citizenfx/natives 的 `.md` 语法本身就是引擎侧 Lua 模板的输出格式（`ns`/`aliases`/`// hash jhash` 一一对应）；`ext/natives/codegen.lua:582-605` 按输入文件名选择标题（`gta_universal`→`inp/natives_global.lua`、`rdr3_universal`、`ny_universal`，`ourGame` 置位）；`ext/natives/natives_stash/gta_universal.lua:40-52` 展示引擎内的 DSL 记录（`native "GET_PLAYER_PED"` / `hash "0x43A66C31C68491C0"` / `jhash (0x6E31E993)` / `arguments{}` / `ns "PLAYER"` / `returns "Ped"` / `doc`），格式说明见 `ext/natives/README.md:17-31`。反向回灌由 `ext/native-doc-gen/build.sh:29,40,44` 完成：`codegen.lua … markdown` 生成 CFX 侧 `.md`，`native-doc-tooling/index.js` 用 libclang 解析回 JSON（「推断」`.md` 的 C 块正是靠 libclang 抽取类型，这也解释了 setup-builder:8 为何装 libclang）。分工因此清晰：**natives 仓库是权威人类可读数据库，ext/natives 是编译器/消费端**。

### 3.9.6 这一层证明了什么

RAGE 脚本 VM 的表面积在此被精确量化：GTA V 6437 条 + CFX 自有 889 条，44 个命名空间，NETWORK 独占 824 条（12.8%），说明线上化是该 VM 的主战场；1003 条仍是 `N_0x…` 占位，即命名覆盖率约 84.4%。跨 build 版本化以文件名为键（`gta_0193D0AF`、`gta_21E43A33`、`gta_universal`）加 compatgen 的历史签名抽取；多标题扩展则已**预留但未填充**——`ext/natives/Makefile:1-24` 与 `codegen_out_cs_v2.lua:2-7` 的 `PRODUCTS = { global=GTA_FIVE/FiveM, rdr3=IS_RDR3/RedM, ny=GTA_NY/LibertyM }` 表明接入新作只需一个 stash 文件与一套 `.md` 目录。逆向知识的最终形态不是脚本，而是带不变量（`jhash == joaat(name)`）、带 CI、带别名兼容层的数据集——这正是 HEAD 提交值得写进方法论的原因。
