# 《塞尔达传说：时之笛》闭源编译器与预编译工具链 REA 深度逆向与重构报告

> **工单编号**：`emrpg/gta6-oot-rea-reverse-engineering#2`  
> **分析工具**：`/usr/local/bin/rea` (`rea@6.9.0`)：`inspect-native-exports`、`inspect-pe-imports`、`inspect-pe-resources`、`inspect-native-initializers`、`inspect-elf-unwind-ranges`、`inspect-elf-notes`、`inspect-binary-hardening`、`audit-release-exposure`、`build-reconstruction-obligation-ledger`  
> **证据存档**：`/root/wsl-demo/rea-reconstructions/oot-binaries/rea-evidence/`  
> **C/C++ 重构源码**：`/root/wsl-demo/rea-reconstructions/oot-binaries/`

---

## 1. 分析环境与方法说明

本次逆向针对本地工作区中的 **23 个**核心闭源编译器组件与预编译工具二进制（涵盖 SGI IRIX IDO 5.3/7.1、3DS 反汇编工具、OoT-Randomizer 压缩/注入/差分工具及 `seq64_console.exe`）执行了全量 REA 静态取证与重构账本生成。

> **能力边界说明**：当前宿主机未安装 Ghidra 12.x、IDA 或 Kuna 等第三方反编译引擎（`rea capabilities` 显示 `ghidra/ida/hopper/kuna` 为 `unavailable`）。因此，所有二进制的导出符号、导入表（IAT/ILT）、初始化段、ELF Unwind FDE 范围、安全加固属性、暴露面审计与重构义务账本均由 REA 内置解析器**直接实测观测**；C/C++ 重构代码则基于 REA 实测结构与二进制只读字符串/符号表进行等价架构还原。

---

## 2. 目标二进制全量 REA 静态取证矩阵

### 2.1 SGI IRIX IDO 5.3 / 7.1 闭源编译器运行时 (MIPS32 Big-Endian ELF)

| 目标组件 | SHA-256 (前 12 位) | 架构 | 导出符号数 | 初始化器 | 关键发现 |
| :--- | :--- | :--- | :---: | :---: | :--- |
| `ido5.3_compiler/lib/libmalloc.so` | `cd6cce9583a6` | `mips` | 28 | 2 | 与 IDO 7.1 `libmalloc.so` **字节级完全一致** |
| `ido5.3_compiler/lib/libmalloc_old.so` | `5742b9c8844a` | `mips` | 28 | 2 | 旧版堆分配器实现（独立构建） |
| `ido5.3_compiler/usr/lib/libc.so.1` | `803f8b127194` | `mips` | 2,605 | 2 | IRIX 5.3 标准 C 运行时 |
| `ido5.3_compiler/usr/lib/libexc.so` | `b59bf75bcbff` | `mips` | 25 | 2 | MIPS 异常分发与栈回溯（`find_rpd` / `unwind`） |
| `ido5.3_compiler/usr/lib/libgen.so` | `bb66dea12420` | `mips` | 95 | 2 | 路径/目录工具集（`basename`/`dirname`/`mkdirp`/`regcmp`） |
| `ido5.3_compiler/usr/lib/libm.so` | `8acc66c2d131` | `mips` | 201 | 2 | MIPS 数学库 |
| `ido7.1_compiler/lib/libmalloc.so` | `cd6cce9583a6` | `mips` | 28 | 2 | 同 IDO 5.3 |
| `ido7.1_compiler/lib/libc.so.1` | `864bfba8caeb` | `mips` | 3,217 | 2 | 相比 5.3 新增 **612 个导出符号** |
| `ido7.1_compiler/usr/lib/libexc.so` | *(N/A)* | `ELF` | 28 | 0 | 相比 5.3 新增 `__exc_set_gp_and_resume`、`unwind_name`、`find_rpd_name` |
| `ido7.1_compiler/usr/lib/libm.so` | `a7eed5db464b` | `mips` | 201 | 2 | IRIX 7.1 更新版数学库 |

### 2.2 3DS 逆向、OoTR 二进制工具与音频序列转换器 (PE / ELF)

| 工具二进制 | 格式 / 架构 | SHA-256 (前 12 位) | 导入 DLL (函数数) / 导出 | Unwind FDEs / Init | ASLR (`dynamic_base`) & 工具链特征 |
| :--- | :--- | :--- | :---: | :---: | :--- |
| `ctrtool.exe` | PE32+ `x86_64` | `921ab18aa2e0` | 1 DLL (`KERNEL32.dll`, 88 函数) | — | **enabled**；静态链接 CRT，纯 Win32 API 封装 |
| `ctrtool` (ELF) | ELF `x86_64` | `c7ede2251448` | 8 exports | 256+ FDEs / 17 init | C++11 (`std::runtime_error` / `std::regex_error`) |
| `n3dsdisasm.exe` | PE32+ `x86_64` | `2bde6c09bde0` | 10 DLLs (71 函数) | — | **enabled**；动态链接 `VCRUNTIME140.dll` + Universal CRT |
| `n3dsdisasm` (ELF) | ELF `x86_64` | `131fd076eb2f` | 2 exports | 256+ FDEs / 4 init | 10.2 MB，内嵌 Capstone ARM/Thumb 反汇编引擎 |
| `Compress.exe` | PE32+ `x86_64` | `f6ad34f6cd6b` | 2 DLLs (`KERNEL32`+`msvcrt`, 119) | — | **enabled**；MinGW-w64 编译 |
| `Compress` (ELF) | ELF `x86_64` | `19ad907a0286` | 2 exports | 23 FDEs / 4 init | 26.6 KB，含 `yaz0_internal` / `yaz0_encode` |
| `Decompress.exe` | **PE32 `x86` (32位)** | `28403baff19b` | 3 DLLs (`KERNEL32`+`msvcrt`, 59) | — | **disabled**；唯一 32 位构建 |
| `Decompress` (ELF) | ELF `x86_64` | `a28c212a86bd` | 1 export | 17 FDEs / 4 init | 14.5 KB，极简 Yaz0 解码器 |
| `gzinject.exe` | PE32+ `x86_64` | `78f09b277702` | 2 DLLs (`KERNEL32`+`msvcrt`, 89) | — | **disabled**；MinGW-w64 编译 |
| `gzinject` (ELF) | ELF `x86_64` | `e9a1df9b5fd9` | 4 exports | 69 FDEs / 4 init | 80.0 KB，Wii WAD/TMD/Ticket/AES-CBC 注入 |
| `minibsdiff.exe` | PE32+ `x86_64` | `f9b5e70be183` | 2 DLLs (`KERNEL32`+`msvcrt`, 55) | — | **enabled**；MinGW-w64 编译 |
| `minibsdiff` (ELF) | ELF `x86_64` | `1c8baf426391` | 0 exports | 23 FDEs / 2 init | 22.7 KB，`BSDIFF40` 差分补丁器 |
| `seq64_console.exe` | PE32+ `x86_64` | `7b5f10c23652` | **22 DLLs (342 函数)** | — | **enabled**；JUCE 框架 + `MSVCP140` + `WININET`/`WS2_32`/`ole32` |

---

## 3. 核心二进制内部架构与 C/C++ 重构实现

### 3.1 IDO 5.3 / 7.1 IRIX 运行时库 (`ido_irix_runtime_reconstructed.c`)
- **`libmalloc.so`**：基于 28 个导出符号还原了经典 SGI malloc 块头链表结构（`malloc_hdr_t`）、队列完整性自检 `__checkq()`、对齐分配 `memalign()`（按 2 的幂对齐并在前驱指针槽回写原始地址）、`recalloc()` 增量清零与 `mallopt()`/`mallinfo()` 统计接口。
- **`libexc.so`**：还原了基于 `.pdata` 运行时过程描述符（`runtime_pdr_t`）的二分查找 `find_rpd()` 与排序比较器 `compare_rpdrs()`；并标注了 IDO 7.1 新增的全局指针恢复跳板 `__exc_set_gp_and_resume`。
- **`libgen.so`**：还原了 `basename_irix()` 与 `dirname_irix()` 等路径规范化逻辑。

### 3.2 3DS 工具链：`ctrtool` 与 `n3dsdisasm`
- **`ctrtool` (`oot3d_ctrtool_reconstructed.cpp`)**：从 `ctrtool.exe` 仅依赖 `KERNEL32.dll`（88 个函数，含 `CreateFileW` / `ReadFile` / `SetFilePointerEx` / `RtlVirtualUnwind`）的特征可见其采用了静态链接 C++ 运行时。重构实现了 `NcchHeader`（`0x200` 字节对齐媒体单元、`NCCH` 魔数 `0x4843434E`）与 `ExefsFileHeader` 解析器及 `extract_exefs()` 流程。
- **`n3dsdisasm` (`oot3d_n3dsdisasm_reconstructed.cpp`)**：结合 `VCRUNTIME140.dll` + Universal CRT 导入与 ELF 中观测到的 `arm_func_start`、`thumb_func_start`、`non_word_aligned_thumb_func_start`、`arm_far_jump`、`thumb_far_jump` 及断言 `arminsn->op_count > 0`，重构了 ARM/Thumb 双模式对齐校验、函数头发射与远近跳转标签生成逻辑。

### 3.3 OoT-Randomizer 工具链：`Compress`、`Decompress`、`gzinject`、`minibsdiff`
- **`Compress` & `Decompress` (`ootr_yaz0_compressor_reconstructed.c`)**：依据 `Compress` 中观测到的 `"Zelda64 Compressor, Version 2.1"`、`yaz0_internal`、`yaz0_encode` 及 `dmaTable.dat` 越界报错，完整还原了 Big-Endian `Yaz0` 头（16 字节）、`0x1000` 滑动窗口最长匹配搜索（`yaz0_internal`）与 2/3 字节变长编码/解码管线。
- **`gzinject` & `minibsdiff` (`ootr_gzinject_minibsdiff_reconstructed.c`)**：还原了 Wii WAD 头部结构（`header_size = 0x20`、`"Is\0\0"` 类型标识、证书/Ticket/TMD/Data/Footer 分段）、TMD 偏移 `0x19C` 区域码修补以及 `BSDIFF40` 64 位符号编码头校验。

### 3.4 `seq64_console.exe` (`seq64_console_reconstructed.cpp`)
- 通过 `inspect-pe-imports` 确认其基于 **JUCE** 框架静态编译（引入 `USER32`、`SHELL32`、`ole32`、`WININET`、`WS2_32` 等 22 个系统 DLL 共 342 个导入），结合观测到的 CLI 报错字符串（`--in`、`--out`、`--abi`、`Error parsing XML of `）重构了命令行参数状态机与入口骨架。

---

## 4. 安全加固与暴露面审计总结 (`audit-release-exposure`)

1. **ASLR 与位数碎片化**：`Decompress.exe` 为 32 位 PE32 且未启用 `dynamic_base`（`gzinject.exe` 同样禁用 `dynamic_base`）；其余 x86_64 PE 工具均已开启 `dynamic_base`。
2. **原生暴露面**：所有 23 个二进制均未随包附带 `.pdb` / `.dSYM` / `.debug` 伴随调试文件，但 ELF 工具普遍保留了完整符号表与 `.eh_frame` Unwind 信息（如 `ctrtool` / `n3dsdisasm` 达 256+ FDEs），为本次静态结构还原提供了充分证据。
