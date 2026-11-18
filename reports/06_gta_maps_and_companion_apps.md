# GTA 6（GTA VI）及 GTA 系列地图、制图测绘与伴侣 App 专项深度调研报告

> **调研范围**：本报告结合 `/root/wsl-demo/gta-peripherals/maps-and-apps/` 下新拉取的 **6 个开源项目** 与 `/root/wsl-demo/gta-projects/` 下既有的 **2 个核心测绘项目** 的本地源码剖析，并系统调研了 **GTA VI Mapping Project、vimap、GTAWeb.eu / GTALens、MapGenie** 等闭源社区地图，以及 **Rockstar 官方 iFruit 与 RDR2 Companion App** 的协议架构与演进规律。

---

## 一、 开源项目全景矩阵（8 个本地源码项目）

| 项目名称 | 本地路径 | 核心技术栈 | 定位与核心能力 |
| :--- | :--- | :--- | :--- |
| **gtamaplib** | `gta-projects/gtamaplib` | Python / NumPy / SciPy / Pillow | 基于 GTA 右手 ENU 坐标系的 3D 相机标定与射线三角测量核心库 |
| **gtamaplib-vc** | `gta-peripherals/maps-and-apps/gtamaplib-vc` | Python (`scipy.optimize`) / 原生 WebGL / HTTP API | `gtamaplib` 伴侣平台：稀疏束调整全局优化器、2D/3D WebGL 查看器与 3D 飞行/游戏模式 |
| **gta6map.github.io** | `gta-peripherals/maps-and-apps/gta6map.github.io` | Python (Pillow) / HTML5 Canvas & JS | GTA VI 社区地标地图前端与 7 级 `1024×1024` 金字塔瓦片/多图层叠加生成器 |
| **gta6-leonida-atlas** | `gta-projects/gta6-leonida-atlas` | React 19 / TS 6 / Leaflet / Three.js 0.186 / Dexie | 本地优先的 Leonida 交互式地图与程序化 3D 街景探索器 |
| **gta-vi-beyond** | `gta-peripherals/maps-and-apps/gta-vi-beyond` | Next.js 16 / React 19 / Tailwind v4 / Framer Motion | GTA VI 社区概念展示站、倒计时与载具/武器/角色图鉴 Web App |
| **live_map** | `gta-peripherals/maps-and-apps/live_map` | Node.js / `ws` (WebSocket) / Lua | FiveM 实时玩家坐标与 Blip 遥测 WebSocket 广播后端 |
| **webmap** | `gta-peripherals/maps-and-apps/webmap` | Lua (`httpmanager` / `weathersync`) / JS | 轻量级 FiveM & RedM HTTP JSON 轮询实时地图（含天气与时间同步） |
| **TrackM** | `gta-peripherals/maps-and-apps/TrackM` | C# (`CitizenFX.Core`) / `Sider` (Redis) | 基于 Redis 存储与位移阈值过滤的 FiveM 通用实体追踪后端 |

---

## 二、 GTA VI 开源制图、摄影测量与交互前端源码剖析

### 1. `gtamaplib` 与 `gtamaplib-vc`：从单镜头三角测量到全局束调整优化
- **几何基础（`gtamaplib/gtamaplib.py`）**：
  - 严格对齐 GTA 内部**右手 ENU（East-North-Up）坐标系**：`+X` 东、`+Y` 北、`+Z` 上；Yaw 绕世界 `+Z` 轴（0° 为正北，逆时针递增），Pitch 绕局部 `+X` 轴，Roll 绕局部 `+Y` 轴。
  - 通过 `Camera._get_fov_from_lines()` 提取画面中正交水平线与垂直线族的消失点，反推未知预告片截图的水平/垂直视场角（HFOV/VFOV），并利用 `find_camera` 进行多进程并行参数扫描。
- **全局束调整与 3D 可视化（`gtamaplib-vc`）**：
  - **双端口前后端分离（`server.py` & `api.py`）**：`server.py` 在 `127.0.0.1:8026` 提供静态 UI 服务，`api.py` 在 `8027` 端口提供标注编辑与增量持久化 API（`gtamaplib-vc-observation-edits-v1`）。
  - **稀疏非线性最小二乘优化器（`optimize.py`）**：基于 `scipy.optimize.least_squares` 与 `scipy.sparse.lil_matrix` 构建多相机-多地标联合束调整（Bundle Adjustment）管线。每个相机优化 7 维参数向量 `[x, y, z, yaw_delta, pitch, roll, hfov]`，并设置了严格数值稳定性保护（如 `DEFAULT_MIN_TRIANGULATION_BASELINE_DEG = 10.0` 防止窄基线射线交点误差放大，以及湖面高程先验 `LAKE_LEONIDA_Z = 5.0`）。
  - **零依赖 WebGL 3D 引擎（`ui/map3d.js`）**：不依赖 Three.js 等外部库，直接用原生 WebGL 渲染 `32768×32768` 世界网格上的社区瓦片（`/gtadb.org/maps/tiles/6/yanis,14`），并叠加社区重建的 3D 建筑线框（如 Four Seasons、Sunshine Skyway、Hanks Waffles、Jason's House 等 JSON 线框资产），甚至内置了支持手柄与键盘的 3D 漫游/飞行模式（`game/`）。

### 2. `gta6map.github.io`：多底图金字塔瓦片与透明叠加层校准体系
- **核心切图管线（`tiles.py`）**：
  - 将 `level_bounds = ((-16384, -16384), (16384, 16384))`（即边长 **32,768 米** 的世界范围）切分为 `z = 0..6` 共 7 个缩放层级，单张瓦片尺寸 `1024×1024 px`。在 `z = 5` 时，`32768 / (2^5 × 1024) = 1.0 m/px`，即精确达到 **1 像素 = 1 米** 的物理比例尺。
  - **多制图师底图与无冗余叠加层（`overlays`）**：源码中同时兼容多位社区制图师的底图与局部高精度拼图，例如主底图 `("dupzor", 51, 0.558, (9037, 6693))`、`("yanis", 6, 0.558, (9037, 6693))`，并通过线性仿射偏移公式 `zero = (pixel_x - world_x * scale, pixel_y + world_y * scale)` 将 `aiwe`、`martipk`、`rickrick`、`vlad` 等局部区块透明叠加到底图上，避免重复生成全量瓦片。

### 3. `gta6-leonida-atlas` 与 `gta-vi-beyond`：现代前端地图与社区展示站
- **`gta6-leonida-atlas`（`gta-projects/gta6-leonida-atlas`）**：采用 React 19 + TypeScript 6 + Leaflet（`CRS.Simple`，`INITIAL_BOUNDS: [-10000..3300, -8000..9200]`）+ Dexie（IndexedDB 本地优先存储）+ Three.js `0.186.1`，在 `src/features/street-leonida/` 下实现了带区域流式加载、水面反射、大气散射与视差窗户的程序化 3D 街景探索器。
- **`gta-vi-beyond`（`maps-and-apps/gta-vi-beyond`）**：基于 Next.js `16.2.9` + React `19.2.4` + Tailwind CSS v4 + Framer Motion `12.42.0` 构建的 GTA VI 概念交互门户，包含 `Countdown.tsx`（发售倒计时）、`CharactersSection.tsx`、`VehiclesSection.tsx`、`WeaponsSection.tsx` 与 `ViceCityShowcase.tsx`。

---

## 三、 FiveM / RedM 实时地图与遥测流架构对比

在 GTA V / RDR2 多人联机生态中，社区衍生出了三种典型架构的“实时第二屏幕/Web 地图”方案：

1. **WebSocket 实时推送架构（`live_map`）**：
   - `src/sockets.js` 使用 Node.js `ws` 库在 FiveM 服务端直接拉起 WebSocket 服务（支持基于 `req.headers.origin` 的跨域白名单校验），通过 `wss.broadcast` 定时向所有在线 Web 客户端推送 `{ type: "playerData", payload }`。
   - 客户端 `fxmanifest.lua` 暴露 `reverseWeaponHash`、`reverseVehicleHash`、`reverseStreetHash`、`reverseZoneHash`、`reverseAreaHash` 等原生哈希反查导出函数，将 RAGE 内部整数哈希翻译为可读文本。
2. **HTTP JSON 轮询架构（`webmap`）**：
   - `client.lua` 响应服务端事件 `webmap:updateInfo`，采集 `GetEntityCoords`、`GetEntityHeading`、`GetEntityHealth` 上报服务端；`server.lua` 依赖 `httpmanager` 暴露 `/info.json` 与 `/config.json`，并集成 `weathersync` 导出游戏内实时时间与天气预报，同时兼容 `gta5` 与 `rdr3`。
3. **Redis 解耦分布式追踪架构（`TrackM`）**：
   - `TrackM.Server/Server.cs` 使用 C#（`CitizenFX.Core`）与 `Sider` Redis 客户端池，将 FiveM 游戏服与外部 Web 前端彻底解耦。
   - 服务端通过 `trackm_update_interval`（最小 `500ms`）与 `trackm_movement_threshold`（采用平方距离比较减少开方开销）过滤无效抖动，并通过 `tm:Track`、`tm:Untrack`、`tm:MetadataSet` 事件总线支持自定义实体（如警车单元、NPC）元数据写入 Redis。

---

## 四、 闭源社区地图生态深度调研

1. **GTA VI Mapping Project（DuPz0r 等社区制图工程）**：
   - 起源于 2022 年 9 月大规模开发片段泄露事件，由社区核心制图者 **DuPz0r** 等人在 GTAForums 与 Discord 协同推进。
   - **方法论**：从泄露开发画面调试信息中提取 RAGE 引擎内部浮点坐标，锚定 `(0, 0)` 世界原点基准与相对距离；后续结合 Trailer 1/2 及官方截图的太阳方位角、地平线曲率、高速路标与海岸线轮廓，历经 `v0.044` 至 `v0.051+` 等数十次迭代，逐步拼合出涵盖 Vice City、Leonida Keys、Lake Leonida 与 Grassrivers 的完整州级地理模型。
2. **`vimap`（`vimap.saamexe.com`）**：
   - 专为 GTA VI Mapping Project 打造的闭源交互式 Web 查看器，高频同步 DuPz0r 等人的最新迭代底图，核心特色是提供 **与 GTA V 洛圣都（Los Santos）地图的 1:1 比例尺叠图对比**、网格坐标读取与地标标注浏览。
3. **`GTAWeb.eu` / `GTALens`**：
   - 覆盖 GTA V、GTA Online、Red Dead Online 及 GTA VI 追踪的多游戏交互式地图枢纽。其技术强项在于**高密度动态/每日刷新点管理**（如 GTA Online 的藏匿屋、街头毒贩、沉船、时间挑战赛）与多图层实时过滤，底层同样依赖 Leaflet/自定义瓦片金字塔与社区 crowdsourced 坐标库。
4. **`MapGenie`（Web + iOS / Android 伴侣 App）**：
   - 商业化最成熟的第三方游戏伴侣地图平台，针对 **GTA V** 收录超过 2,000+ 点位（50+ 分类，如飞船碎片、信件残页、特技跳跃、随机事件），针对 **GTA:SA** 提供帮派涂鸦、牡蛎、马蹄铁、拍照点全收集追踪。
   - **产品架构**：提供 iOS / Android 原生 App 与 Web 端，支持免费版基础进度勾选与 **MapGenie+ 订阅制**（解锁多存档独立进度 Profile、无限自定义标记与跨设备云同步）。

---

## 五、 Rockstar 官方伴侣 App 架构演进与 GTA VI 前瞻

### 1. `iFruit` App（2013–2022）架构与停服始末
- **三层异步云端架构**：
  1. **移动端壳层**：仿 iOS 6 拟物化界面，原生 UI 混排 WebView。
  2. **Rockstar Social Club（RSC）中间件**：App **不与本地主机/PC 直连**，所有操作（如 Los Santos Customs 改车、自定义车牌、训练小查 Chop）均封装为 HTTPS REST 请求发送至 RSC 云端。
  3. **游戏客户端延迟拉取**：玩家启动 *GTA V* / *GTA Online* 时，游戏客户端在初始化握手阶段向云端拉取待同步队列并应用至存档/线上角色。
- **技术债与停服原因（2022 年 12 月 12 日正式停服）**：
  - **硬编码资产库耦合**：iFruit 内部硬编码了 2013 年首发时的载具与改装件 ID 映射表，未随 *GTA Online* 近十年的 DLC 更新同步扩展；玩家在游戏内驾驶新车后打开 LS Customs 模块极易触发解析异常、无限加载或闪退。
  - **平台与安全框架老化**：App 自 2019 年起基本停止维护，iOS/Android 淘汰 32 位架构、升级 TLS 握手与 OAuth 鉴权规范后，旧客户端彻底失效。Rockstar 最终下架 App，并将核心的“自定义车牌”功能迁移至官网 Web 页面直接下发。

### 2. `RDR2 Companion App` 实时第二屏幕地图同步协议
- **“云端鉴权 + 局域网直连”混合双平面架构**：
  1. **控制平面（Cloud-Assisted Pairing）**：移动端与主机/PC 端分别登录同一个 **Rockstar Social Club** 账号，且需处于**同一局域网（Wi-Fi）**下；RSC 服务器仅负责身份验证与会话配对令牌交换。
  2. **数据平面（LAN Direct Socket）**：配对成功后，遥测数据绕过公网云端，直接在本地局域网建立自定义 **UDP/TCP Socket** 通道。
- **增量遥测同步机制**：
  - 移动端本地预先缓存完整的高分辨率矢量/瓦片地图资源，**不通过网络传输任何画面像素**。
  - 游戏引擎以高频率将主角 `(X, Y, Z)` 坐标、相机朝向矩阵、动态任务标记与路径点（Waypoints）序列化为极轻量级的二进制增量包（Delta Telemetry）推送至移动端，移动端在本地 Canvas 上实时重绘图标与路线，实现亚秒级无感双向交互。

### 3. 对 GTA VI（GTA 6）官方伴侣 App 的技术预示
结合 iFruit 的教训与 RDR2 Companion App 的成功经验，GTA VI 若推出官方伴侣工具，预计将呈现三大架构演进方向：
1. **数据驱动与 Web/PWA 化**：告别硬编码资产表，转向从 Social Club / 游戏服务端动态下发载具、资产与活动配置（类似目前官网车牌工具的形态），避免因 DLC 膨胀导致客户端崩溃。
2. **局域网低延迟第二屏幕（RDR2 协议升级版）**：利用现代局域网 mDNS/UPnP 自动发现与加密本地 Socket，实现 Leonida 巨型无缝地图的实时 GPS 导航、手机内网（模拟游戏内智能手机/社交媒体）互动及玩家状态监控。
3. **与游戏内“虚拟手机”生态打通**：GTA 系列一贯拥有核心的手机交互系统，在 GTA VI 强化社交媒体与即时通讯叙事的背景下，伴侣 App 极有可能承担“现实手机映射游戏内手机/地图”的跨屏体验。
