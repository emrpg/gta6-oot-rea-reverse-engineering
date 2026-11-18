# Ocarina-Atlas Linux AppImage 启动器的 Electron IPC 面（REA 图谱 + 源码双向核验）

分析对象：`/root/wsl-demo/oot-peripherals/Ocarina-Atlas/launcher/linux-appimage/`
Evidence：`../evidence/Ocarina-Atlas__app.json`（`ev_0286954cf953b050e547facba802faae834fa7969fc0a23f58672621614f42aa`，root_artifact `451baa2b…`，778/778 文件检查）
表格：`03_ocarina_atlas_electron_surface.csv`（49 行观测）、`04_ocarina_atlas_endpoints.csv`、`05_ocarina_atlas_storage_keys.csv`

`reports/08_oot_maps_trackers_and_apps.md:55` 已从源码侧记录 Ocarina-Atlas 的四形态交付，其中包含
`launcher/linux-appimage/main.cjs`（1,329 行）。本节是那次人工阅读的**机读化与交叉验证**：REA 的
Electron 抽取器（`observe-ipc-channel-expression`、`observe-ipc-handler`、`observe-context-bridge-api`、
`observe-browser-window-construction`）把同一表面投影为 49 条带 `文件:行:列` 的观测，逐条与源码对齐成立；
`preload.cjs` 为 32 行。两侧不一致之处只有一处计数口径（见 §2 末尾说明），没有事实冲突。

## 1. 窗口与隔离设置

| 事实 | 位置 | 证据等级 |
|---|---|---|
| `new BrowserWindow({...})`，`width 556 / height 480(compact)`，`backgroundColor #13100C`，`title 'Ocarina Atlas'` | `main.cjs:1147-1161` | 观察（exact，`browser-window` 节点） |
| `webPreferences.preload = preload.cjs`、`contextIsolation: true`、`nodeIntegration: false` | `main.cjs:1157-1159` | 观察（源码直读 + `web_preferences_status: object-literal`） |
| `loadFile(index.html)`，菜单栏隐藏 | `main.cjs:1164-1165` | 源码直读 |

## 2. contextBridge 暴露面

`window.ocarinaAtlas`，14 个成员，`unknown_members: 0`（`preload.cjs:5-31`）。

| 渲染侧方法 | IPC 通道 | 方向模式 | main 侧处理 |
|---|---|---|---|
| `getInitialState` | `launcher:get-state` | invoke | `main.cjs:1171` 返回 `getPublicState()+logs` |
| `startServer` / `stopServer` | `launcher:start-server` / `launcher:stop-server` | invoke | `main.cjs:1176` / `:1181` |
| `openMap` | `launcher:open-map` | invoke | `main.cjs:1186` → `shell.openExternal(state.localUrl)` |
| `openExternal(url)` | `launcher:open-external` | invoke | `main.cjs:1194`，仅接受 `/^https?:\/\//i` 字符串 |
| `copyText(text)` | `launcher:copy-text` | invoke | `main.cjs:1202` → `clipboard.writeText` |
| `setupSync` | `launcher:setup-sync` | invoke | `main.cjs:1210` |
| `refreshLan` | `launcher:refresh-lan` | invoke | `main.cjs:1225` |
| `toggleLanMode(enabled)` | `launcher:toggle-lan` | invoke | `main.cjs:1230`，`0.0.0.0` ↔ `127.0.0.1` 并重启服务 |
| `updateSettings(patch)` | `launcher:update-settings` | invoke | `main.cjs:1245`，仅在 `lifecycle === 'stopped'` 时改端口/主机 |
| `resetRpcDefaults` | `launcher:reset-rpc-defaults` | invoke | `main.cjs:1270` |
| `checkUpdates` | `launcher:check-updates` | invoke | `main.cjs:1280` → `checkOfficialUpdates(state.version)` |
| `clearLogs` | `launcher:clear-logs` | invoke | `main.cjs:1287` |
| `setDrawerOpen(open)` | `launcher:set-drawer-open` | invoke | `main.cjs:1292` → `setSize(w, 480|732, true)` |
| `onState` / `onLog` | `launcher:state` / `launcher:log` | listen（main→renderer 推送） | `preload.cjs:23` / `:29` |

节点计数（`03_ocarina_atlas_electron_surface.csv`）：`ipc-channel` 30、`ipc-handler` 16
（14 个 `ipcMain.handle` + 2 个渲染侧 `listen`）、`context-bridge-api` 1、`browser-window` 1。
30 个通道观测 = 14 invoke + 14 handle + 2 listen，两侧各计一次。

## 3. 启动器常量与外部依赖（源码直读）

| 常量 | 值 | 位置 |
|---|---|---|
| `DEFAULT_HOST` / `DEFAULT_PORT` | `0.0.0.0` / `3000` | `main.cjs:13-14` |
| `DEFAULT_LICHII_HOST` / `DEFAULT_LICHII_PORT` | `127.0.0.1` / `6545` | `main.cjs:15-16` |
| `MAX_LOG_LINES` | 500 | `main.cjs:17` |
| `OFFICIAL_REPO_URL` | `https://github.com/antonioalvromero/Ocarina-Atlas` | `main.cjs:19` |
| 窗口尺寸 | 556×480 / 556×732 | `main.cjs:62-64` |
| 环境变量覆盖 | `HOST`、`PORT`、`LICHII_PORT`（或 `FRITCHII_PORT`）、`LONGSHOT_SAVE_DIR`、`NX2SARIA_SAVE_DIR` | `main.cjs:84-87, 998-1001` |

`startServer()` 以 `ELECTRON_RUN_AS_NODE=1`、`NODE_ENV=production` 派生子进程，并注入
`HOST/PORT/LICHII_HOST/LICHII_PORT/APP_DIST_DIR`（`main.cjs:990-1012`）；进程枚举集合
`RECOMP_PROCESS_NAMES`、`NX2SARIA_PROCESS_NAMES`、`EMULATOR_PROCESS_NAMES`（`main.cjs:23-43`）
用于检测已在运行的移植版/模拟器。

## 4. `launcher:setup-sync` 写入的隐藏配置

`setupSync()`（`main.cjs:253-267`）向 6 个候选路径写入调试服务器开关：

```
EDEN_NX2_DEBUG_SERVER=<lichiiHost>:<lichiiPort>
```

候选文件（XDG 感知）：`{config,data}Home / {lichii,fritchii,cheezyemu} / nx2_switches.txt`，
其中 `configHome = $XDG_CONFIG_HOME 或 ~/.config`、`dataHome = $XDG_DATA_HOME 或 ~/.local/share`。
若无匹配配置目录，`state.syncStatus = 'warn'` 并提示手工添加（`main.cjs:1216-1220`）。

## 5. 同一次分析中的其他网络面

- 11 条 `endpoint` 观测：`/api/lichii/state`（`src/lib/useLichiiTracker.ts:806`）、
  `/api/lichii/stream`（`EventSource`，`:885`）、`/api/sync/state`（`src/lib/storage.ts:206, 276`）、
  `https://api.github.com/repos/${safeSlug}/releases/generate-notes`（`scripts/generate-release-notes.mjs:113`）、
  `https://api.openai.com/v1/chat/completions`（`:170`）、2 条 service-worker 的 `request` 变量（`pwa/sw.js:41, 49`）、
  2 条 `route` 候选（`/$zone`、`/`，`src/routeTree.gen.ts:17, 22`，`inferred/low`）。
- 13 条 `storage` 观测：`oot-map:v1:view-zoom`、`oot-map:v1:prereq-auto-sync`、`oot-map:v1:owned-prereqs`
  为字面量键（`src/lib/storage.test.ts:196, 198, 210, 214`）；其余 10 条键为动态表达式（`i18n.ts:151/168`、
  `useFullscreenMap.ts:122/132/134`、`pwa/sw.js:10/37/46`）。
- 路由清单（`route_inventory`）：8 次出现 / 4 个路由模式 / 72 个来源；全部 `classification: unknown`，
  机制为 `navigation-target`、`route-string-constant`。这些只是静态候选，不证明可达。
