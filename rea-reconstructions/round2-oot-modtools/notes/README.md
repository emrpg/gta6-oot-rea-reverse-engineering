# 第二轮增量扫描 · REA 逆向笔记索引

目标目录：`/root/wsl-demo/oot-peripherals/round2-audio-save-modtools/`（克隆源码）
证据目录：`../evidence/`（REA 原始 JSON bundle，本地保留，`.gitignore` 已屏蔽）

| 笔记 | 主题 | 分级 |
|---|---|---|
| `01-save-converters.md` | N64/3DS/Switch 存档格式与互转接口 | F / R / U |
| `02-rompatcher-otr-textures.md` | RomPatcher.js、.otr/.o2r 容器、纹理 GUI | F / R / ? |
| `03-zzrtl-import-pipeline.md` | zzrtl/zzromtool 导入管线与 z64audio 黑箱 | F / R / ? |
| `04-objex-model-pipeline.md` | OBJEX 指令规范、`.skel`/`.anim`、anim2link | F / R / ? |
| `05-evidence-manifest.md` | 证据文件 → SHA-256 → 命令 对照、加固矩阵、方法学 | F |
| `06-checksums-and-container-identities.md` | 校验和算法族、容器身份（OTR=MPQ/O2R=ZIP）、`.zpf`/`.patch`、并发产出核查 | F / R / ? |
| `90-agent-claims-verification.md` | 子 agent 与并发提交断言逐条复核结果 | 逐条 |

## 图例（沿用第一轮）

- **F = 已核实事实**：有 `path:line` 或 REA 证据 ID 直接支撑。
- **R = 参考实现可支撑**：从源码/文档可以写出接口，但未逐字节验证真实样本。
- **? = 异常或与预期矛盾**：证据存在但解释不唯一，需要二进制之外授权。
- **U = 未找到 / 无法在当前授权下确定**。
