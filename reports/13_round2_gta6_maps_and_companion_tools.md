# 第二轮增量扫描：GTA 6 / RAGE 周边地图、遥测雷达、时间周期与伴侣工具深度报告

> **工单关联**：`emrpg/gta6-oot-rea-reverse-engineering#5` (`[Round2-GTA6-Peripherals]`)  
> **增量仓库目录**：`/root/wsl-demo/gta-peripherals/round2-maps-and-companions/`  
> **增量重构目录**：`/root/wsl-demo/rea-reconstructions/round2-gta-peripherals/`  
> **分析范围**：在第一轮 16 个仓库之外，增量拉取并深度剖析 **7 个** GTA 6 / GTA V 周边工具仓库，并结合 `/usr/local/bin/rea` 对前端包与 RAGE 二进制资源执行静态逆向验证。

---

## 一、 第二轮增量仓库全景矩阵（7 个新增仓库）

| 仓库名称 | 分类 | 核心技术栈 | 核心功能与技术亮点 |
| :--- | :--- | :--- | :--- |
| **`DevBlocky/nearest-postal`** | FiveM GPS / 邮编导航 | Lua (`CitizenFX`) / JSON | 玩家最近邮编实时欧氏距离计算、`/postal` GPS 路径点与到达自动清除事件 |
| **`Oulsen/oulsen_satmap`** | FiveM 卫星小地图 / 雷达 | `.ytd` / `.ydd` / `.dds` / Lua | 高清卫星雷达分块（`minimap_0_0..2_1` + `minimap_sea_*`）、`SetRadarZoom(1100)` 与 `oulsen_satmap_postals.json` |
| **`RatPrez/ragekit`** | RAGE 全格式 C++20 库 | C++20 / CMake / `miniz` / `Clipper2` | 同时支持 GTA V Legacy & Enhanced (Gen9) 的 RPF7/RSC7/YDR/YTD/YFT/YNV/AWC/GFX（含室内小地图）读写与 `ragekit::render` |
| **`DottieDot/gtav-timecycle-editor`** | RAGE 运行时时间周期 | C++ / ImGui / RAGE 内存结构 | 运行时修改 `rage::TimecycleModifier` 与 `TimecycleModifierValue` 内存结构（光/暗双值插值） |
| **`MASRPNathan-G/customplates`** | 车牌纹理动态替换 | Lua / FiveM DUI (CEF) | 利用 `CreateDui` + `CreateRuntimeTextureFromDuiHandle` 实时替换 `vehshare/plate01..05` 漫反射与法线贴图 |
| **`izhanlara/gta6countdown`** | GTA 6 发售倒计时 Web App | Astro / CSS / TypeScript | 轻量级 Astro 静态倒计时页面，支持暗黑/霓虹主题切换 |
| **`CoopTRUE/GTA6-COUNTDOWN`** | GTA 6 倒计时单页工具 | HTML / JS | 极简原生 HTML/JS 倒计时页面（1,630 B） |

---

## 二、 核心架构与 REA 静态逆向剖析

### 1. FiveM 导航、邮编网格与卫星雷达管线（`nearest-postal` & `oulsen_satmap`）
- **最近邻邮编检索（`nearest-postal/cl_render.lua`）**：
  - 启动时通过 `LoadResourceFile` 读取 `new-postals.json` / `ocrp-postals.json` / `old-postals.json`；主循环按可配置周期（最低 `50ms`，默认 `300ms`）遍历全部邮编坐标，计算平面欧氏距离 `#(coords - postals[i][1])` 更新 HUD。
  - 当设置了目标邮编路径点 `pBlip` 且距离小于 `config.blip.distToDelete` 时，触发 `nearest-postal:arrivedAtPostal` 事件并销毁 Blip。
- **卫星小地图分块流式架构（`oulsen_satmap`）**：
  - 将洛圣都雷达底图按 `3×2` 陆地网格（`minimap_0_0` 至 `minimap_2_1`）+ `3×2` 海域网格（`minimap_sea_0_0` 至 `minimap_sea_2_1`）拆分为独立的 `.ytd` 纹理字典与 `.ydd` 模型字典，客户端线程通过 `SetRadarZoom(1100)` 锁定雷达缩放比例以保证邮编文字清晰度。
  - **REA 边界验证**：对 `minimap_0_0.ytd` 运行 `/usr/local/bin/rea inspect-artifact` 时，REA 返回 `unsupported binary format`（`file_format_rejected`），印证了通用 PE/ELF/ZIP 分析器无法直接解析 Rockstar 专有的 RSC7 资源页头，必须依赖 `ragekit` 或 `CodeWalker` 专有解析器。

### 2. RAGE 次世代资产与时间周期底层机制（`ragekit` & `gtav-timecycle-editor`）
- **`RatPrez/ragekit`（v0.3.1）的架构突破**：
  - 相比庞大的 C# `CodeWalker`，`ragekit` 仅依赖 `miniz` 与 `Clipper2` 两个单头文件/轻量库，以纯 C++20 实现了 **GTA V Legacy 与 Enhanced（Gen9）双向格式转换**。
  - 在 `include/ragekit/rpf.h` 中清晰定义了 RPF7 四种加密模式：`None (0)`、`Open (0x4E45504F)`、`Aes (0x0FFFFFF9)`、`Ng (0x0FEFFFFF)`，并支持从游戏主程序提取 `gtav_aes_key.dat`（32 B）、`gtav_ng_key.dat`（101×272 B）与 `gtav_ng_decrypt_tables.dat`（17×16×256 words），甚至支持直接从 `.wav` 打包 `.awc` 音频与生成 Scaleform `.gfx` 室内小地图。
- **`gtav-timecycle-editor` 内存布局（`TimecycleModifier.hpp`）**：
  - 精确还原了 RAGE `TimecycleModifier` 运行时结构体：`0x0000` 为 `rage::atArray<TimecycleModifierValue>`、`0x0010` 为 `m_nameHash`、`0x0020` 为 `m_userFlags`；每个 `TimecycleModifierValue` 占 12 字节（`m_type`、`m_lightValue`、`m_darkValue`），实现了游戏内实时光照与大气参数热调。

### 3. FiveM DUI 动态车牌替换与 GTA 6 倒计时前端 REA 分析
- **`customplates`（`plate.lua`）**：利用 FiveM 内置 Chromium DUI 离屏渲染网页图片（`540×300`），通过 `CreateRuntimeTextureFromDuiHandle` 注入运行时字典 `duiTxd`，再调用 `AddReplaceTexture` 批量覆盖 `vehshare` 中的 `plate01..plate05` 与法线贴图 `plate01_n..plate05_n`，是脱离停服版 Rockstar iFruit App 的典型社区替代方案。
- **GTA 6 Web 倒计时（`gta6countdown` & `GTA6-COUNTDOWN`）**：通过 `rea analyze-javascript-application` 生成静态证据包（`gta6countdown__app.json` 含 6 个相关文件 / `197,761 B` / 3 findings；`GTA6-COUNTDOWN__app.json` 含 1 个文件 / `1,630 B` / 1 finding），确认二者均为纯静态无后端依赖的倒计时前端。

---

## 三、 第二轮增量重构产出清单（`/root/wsl-demo/rea-reconstructions/round2-gta-peripherals/`）

1. `fivem-navigation/NearestPostal_And_SatMap.ts`：重构邮编最近邻检索、路径点清除判定与 `oulsen_satmap` 12 块雷达瓦片网格常量。
2. `rage-timecycle-and-rpf/TimecycleAndRpf7_Bridge.hpp`：还原 `rage::TimecycleModifierHeader` 字节对齐内存布局与 `Rpf7Encryption` / `Rpf7KeySpec` 密钥规范。
3. `socialclub-plates/CustomPlate_DuiPipeline.ts`：重构 DUI 离屏纹理注入与 `vehshare` 车牌槽位替换管线。
