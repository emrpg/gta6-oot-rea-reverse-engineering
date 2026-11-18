# 第二轮增量扫描：《塞尔达传说：时之笛》周边追踪器、多世界客户端与地图工具深度报告

> **工单关联**：`emrpg/gta6-oot-rea-reverse-engineering#7` (`[Round2-OoT-Trackers-And-Maps]`)  
> **增量仓库目录**：`/root/wsl-demo/oot-peripherals/round2-trackers-and-maps/`  
> **增量重构目录**：`/root/wsl-demo/rea-reconstructions/round2-oot-trackers/`  
> **分析范围**：在第一轮《时之笛》周边基础上增量拉取并剖析 **5 个**核心追踪器、多世界客户端与 Spoiler Log 可视化项目，结合 `/usr/local/bin/rea` 完成 Electron/JS/TS 应用图谱静态逆向与核心协议重构。

---

## 一、 第二轮增量仓库全景矩阵（5 个新增仓库）

| 仓库名称 | 核心技术栈 | REA 静态分析规模 | 核心定位 |
| :--- | :--- | :--- | :--- |
| **`ArchipelagoMW/Z5Client`** | Electron (`main.js` / `preload.js`) + BizHawk Lua (`ootMulti.lua`) | 10 JS 文件 / 494,449 B / 6,660 AST 节点 / 84 findings | Archipelago 官方 OoT 多世界客户端、`.apz5` 补丁关联与 BizHawk 桥接 |
| **`midoshouse/ootr-multiworld`** | Rust Workspace (`multiworld` / `multiworld-bizhawk` / `ootrmwd`) + C# | - | Mido's House OoTR 多世界联机服务端与版本化 WebSocket 协议（`v16`/`v17`） |
| **`Selene-T/Tracker-of-Time`** | VB.NET (`Memory.vb` / `soh.vb` / `minimap.vb`) | - | 支持 BizHawk、RetroArch 与 **Ship of Harkinian (SoH)** PC 移植版的自动追踪器 |
| **`OOT-Multiworld-Tracker/Multiworld-Tracker`** | Electron + Webpack + Node.js (`app.js`) | 69 JS 文件 / 1,462,093 B / 20,567 AST 节点 / 656 findings | 集成 Archipelago 与内置 WebServer 的跨端多世界自动追踪器 |
| **`amickael/spoilizer`** | React + TypeScript + `yup` | 64 TS/JS 文件 / 91,885 B / 10,512 AST 节点 / 597 findings | OoTR 单人/多世界 Spoiler Log 与入口随机（Entrance Rando）可视化解析器 |

---

## 二、 核心协议与 REA 静态逆向剖析

### 1. `ArchipelagoMW/Z5Client`：Electron IPC 边界与 N64 RDRAM 遥测协议
- **REA Electron 图谱取证（`Z5Client__app.json`）**：
  - REA 识别出 **2 个 `BrowserWindow`**、**4 个 `contextBridge` API**、**17 个暴露成员**、**15 个主进程 `ipcMain` 处理器**与 **17 个字面量 IPC 通道**。
  - `main.js` 通过 `app.commandLine.appendSwitch('disable-renderer-backgrounding')` 防止 Chromium 后台降频影响实时内存轮询，并在 Windows 安装阶段注册 `.apz5`（Archipelago Binary Patch）文件关联与本地 `net` Socket 服务端。
- **N64 RDRAM 自动追踪与远程多世界上下文（`lua/ootMulti.lua`）**：
  - 锚定 NTSC 1.0 `save_context_offset = 0x11A5D0`：包含 `equipment_offset (+0x70)`、`scene_flags_offset (+0xD4, 步长 0x1C)`（`chest@0x0`、`switches@0x4`、`collectibles@0xC`）、`skulltula_flags_offset (+0xE9C)`、`item_get_inf_offset (+0xEF0)` 与 `inf_table_offset (+0xEF8)`。
  - **临时多世界上下文缓冲区（`0x40002C`）**：当玩家拾取非本地多世界物品时，ROM 补丁将 4 字节写入 `0x40002C`：`[0]=0x00`（远程标志）、`[1]=scene_id`、`[2]=location_type`（`0x01` 宝箱 / `0x02` 地面物品）、`[3]=location_bit`，`ootMulti.lua` 据此区分本地与远程检查状态。

### 2. `Selene-T/Tracker-of-Time`：Ship of Harkinian (SoH) PC 移植版跨版本内存偏移矩阵
- 在 `soh.vb` 中，针对不固定基址的 PC 原生移植版 **Ship of Harkinian**，通过读取只读区版本标识字符串（如 `"RACHAEL ALFA (3.0.0)"` 至 `"BRADLEY ECHO (5.1.4)"`）动态切换 `gSaveCtxOff`、场景指针 `gOffset` 与小地图房间偏移 `gRoomAddr`，并精确处理了 `5.0.1 -> 5.1.4` 之间背包偏移量增加 `0x0E` 的结构变化。

### 3. `ootr-multiworld`、`Multiworld-Tracker` 与 `spoilizer`
- **`midoshouse/ootr-multiworld`**：Rust 服务端采用 `async_proto` + `rocket_ws` 实现版本化 WebSocket 协议（`v16.rs` / `v17.rs`），`ServerError` 枚举内置 `Future(u8)` 未知变体兜底以保持前后向兼容。
- **`Multiworld-Tracker`**：REA 完整恢复其 Electron 语义（`electron_semantics: "recovered"`），`app.js` 作为总线桥接 `AutoTracker`（`Archipelago` 分支 `id === 1`）、`WebServer` 与 `Electron` 渲染进程。
- **`spoilizer`**：通过 `yup.lazy` 动态校验单人（`logSchema`）与多世界（`mwLogSchema`）Spoiler Log，支持 `:woth_locations`（Way of the Hero）、`:playthrough` 及双态入口映射 `entrances`（`string | { region, from }`）。

---

## 三、 第二轮增量重构产出清单（`/root/wsl-demo/rea-reconstructions/round2-oot-trackers/`）

1. `archipelago-z5client/N64RamAutoTracker_And_MultiworldBridge.ts`：还原 N64 RDRAM `SaveContext` 偏移表与 `0x40002C` 远程检查解码器。
2. `soh-autotracker/SohVersionOffsets.cs`：还原 SoH `3.0.0..5.1.4` 跨版本内存特征与 `SAV()` 偏移调整函数。
3. `spoiler-and-multiworld/SpoilerLog_And_EntranceSchema.ts`：还原 OoTR Spoiler Log 单/多世界 TypeScript 接口与 `ServerError` 协议枚举。
