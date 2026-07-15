# PE-Parser

一个用 C++ 编写的跨架构 Windows PE（可移植可执行文件）分析工具。

**同时支持 PE32（32 位）和 PE32+（64 位）格式** — 无论编译目标架构如何，都能正确处理两者之间的结构差异。

## 功能特性

- **DOS 头** — 显示所有 DOS 头字段，包括 MZ 签名和 NT 头偏移
- **NT 头** — 解析 COFF 文件头、可选头（32/64 位）和数据目录
- **节头** — 列出所有节，包括名称、虚拟/原始大小、RVA/FOA 范围及特征标志
- **导入表** — 枚举导入的 DLL 及其函数（按名称和序号），支持 INT/IAT 解析
- **导出表** — 显示导出函数，包括序号、RVA、名称及转发导出检测
- **基址重定位表** — 显示重定位块、页 RVA 及每条条目的类型/偏移信息
- **RVA ↔ FOA 转换** — 在相对虚拟地址和文件偏移地址之间相互转换
- **导出函数查找** — 按名称或序号查找导出函数地址
- **32/64 位安全** — 使用架构感知的数据目录访问，即使编译为 x64 也能正确解析 PE32 文件，反之亦然

## 编译构建

### 前置要求

- Windows 操作系统
- Visual Studio 2019/2022（含 C++ 桌面开发工作负载）**或** CMake 3.10+ 配合 MSVC
- 除 Windows SDK 外无其他外部依赖

### 使用 Visual Studio (CMake) 构建

```powershell
# 克隆或打开项目文件夹
cd PE-Parser

# 使用 CMake 配置和构建
cmake -B build -S .
cmake --build build --config Release

# 可执行文件位于: build\Release\pe_parser.exe
```

### 使用开发者命令提示符构建

```powershell
# 在 Visual Studio 开发者命令提示符中：
cl /EHsc /Fe:pe_parser.exe ^
  src\main.cpp src\pe_common.cpp src\pe_parser.cpp ^
  src\pe_dos.cpp src\pe_nt.cpp src\pe_section.cpp ^
  src\pe_import.cpp src\pe_export.cpp src\pe_relocation.cpp ^
  src\pe_info.cpp src\console.cpp ^
  /I include /D _CRT_SECURE_NO_WARNINGS
```

## 使用方法

```
=== PE 文件分析工具 ===

命令：
  load <文件路径>  - 加载 PE 文件
  info             - 显示 PE 基本信息
  dos              - 显示 DOS 头
  nt               - 显示 NT 头（文件头 + 可选头）
  section          - 显示节头
  import           - 显示导入表
  export           - 显示导出表
  getprocname <名> - 按名称查找导出函数
  getprocindex <号>- 按序号查找导出函数
  relocation       - 显示基址重定位表
  rva <地址>       - 将 RVA 转换为 FOA
  foa <地址>       - 将 FOA 转换为 RVA
  clear            - 清屏
  help             - 显示帮助
  exit             - 退出程序

请输入命令 >
```

### 示例会话

```
请输入命令 > load C:\Windows\System32\kernel32.dll
PE 文件加载成功 -> C:\Windows\System32\kernel32.dll
文件大小 -> 0x000B2000 (729088 字节)
格式 -> PE32+ (64 位)

请输入命令 > info
==== PE 文件基本信息 ====
文件格式:   PE32+ (64 位)
文件类型:   DLL (动态链接库)
CPU:         x64 (AMD64)
入口 RVA:   0x0001F4B0
映像基址:   0x00007FFB6A0C0000
节数量:      7
子系统:     Windows 控制台 (CUI)
...

请输入命令 > export
==== 导出目录信息 ====
...
==== 导出函数表 ====
序号             RVA             名称
-------------------------------------------------------------------------------------------------------
1               0x0001F4B0      AcquireSRWLockExclusive
2               0x00072C40      AcquireSRWLockShared
...

请输入命令 > rva 0x1F4B0
转换结果：
  RVA: 0x0001F4B0  ->  FOA: 0x0001E8B0
  所在节: .text

请输入命令 > exit
```

## 项目结构

```
PE-Parser/
├── include/
│   ├── pe_common.h       # 公共宏、PE_CONTEXT 结构体、工具函数声明
│   ├── pe_parser.h       # PE 加载器核心、RVA/FOA 转换、显示函数
│   └── console.h         # 菜单、帮助、控制台工具函数
├── src/
│   ├── main.cpp          # 入口点和命令分发循环
│   ├── pe_common.cpp     # 工具函数：架构名称、标志打印、32/64 位辅助函数
│   ├── pe_parser.cpp     # PE 文件加载器、RVA/FOA 转换
│   ├── pe_dos.cpp        # DOS 头显示
│   ├── pe_nt.cpp         # NT 头显示（FileHeader + OptionalHeader 32/64）
│   ├── pe_section.cpp    # 节头显示
│   ├── pe_import.cpp     # 导入表解析与显示
│   ├── pe_export.cpp     # 导出表解析、按名称/序号查找
│   ├── pe_relocation.cpp # 基址重定位表显示
│   ├── pe_info.cpp       # PE 基本信息摘要
│   └── console.cpp       # 菜单渲染、帮助文本
├── CMakeLists.txt
├── .gitignore
└── README.md
```

## 关键修复：从 64 位构建中解析 32 位 PE

原始代码使用 `PIMAGE_NT_HEADERS`（根据**编译目标**解析为 `IMAGE_NT_HEADERS32*` 或 `IMAGE_NT_HEADERS64*`）来访问 `DataDirectory[]` 数组。由于 `OptionalHeader` 的布局在 PE32 和 PE32+ 之间不同，DataDirectory 数组位于不同的偏移处：

| 格式   | OptionalHeader 大小 | DataDirectory 偏移 |
|--------|--------------------|--------------------|
| PE32   | 224 字节           | +96 (0x60)         |
| PE32+  | 240 字节           | +112 (0x70)        |

当工具编译为 x64 但打开 32 位 PE 文件时，会从 64 位偏移处读取数据 — 导致导入、导出和重定位数据出错。

**修复方法：** `pe_common.cpp` 中的 `GetDataDirectory()` 函数在运行时检查 `OptionalHeader.Magic`，并在访问 `DataDirectory[]` 之前转换为正确的头类型（`IMAGE_NT_HEADERS32*` 或 `IMAGE_NT_HEADERS64*`）。这确保了无论构建架构如何，都能正确解析。

### 其他已应用的修复

- **RvaToFoa**：头区域检查现在使用 `SizeOfHeaders`（正确）而非 `NumberOfSections`（不正确）
- **CmdRelocation**：为各个重定位条目添加了缺失的 `PRINT_INFO` 输出（之前条目被解析但从未显示）
- **导入序号检测**：正确处理 64 位序号导入（第 63 位）与 32 位（第 31 位）的区别

## 重构摘要

原始约 1400 行的单一 `main.cpp` 被分解为：

| 关注点          | 原始实现                            | 重构后                                        |
|----------------|------------------------------------|----------------------------------------------|
| DOS 头显示     | `CmdDos` 中内联实现                 | `pe_dos.cpp` — 基于循环的字段打印              |
| NT 头显示      | `CmdNt` 含重复的 32/64 分支         | `pe_nt.cpp` — 拆分为 `PrintOptionalHeader32/64` |
| 架构/子系统名称 | 重复的 `switch` 语句                | `pe_common.cpp` 中的 `GetMachineName()`、`GetSubsystemName()` |
| 特征标志       | 重复的结构体数组 + 打印循环          | `PrintFlagsDword()` / `PrintFlagsWord()` 通用函数  |
| 节显示         | `CmdSection` 中内联实现             | `pe_section.cpp` 配合每个节的 `PrintOneSection()` |
| 命令分发       | `ProcessCommand` + `FindCmdHandler` | 在 `main.cpp` 中使用 `CmdEntry` 表进行了清理    |
| 全局变量       | 5 个全局 `g_*` 变量                 | 单个 `PE_CONTEXT` 结构体传递给所有函数          |
| 错误消息       | 中英文混杂，不一致                   | 统一为中文，格式一致                            |

## 许可证

本项目仅供教育目的使用。可用于学习 PE 文件格式和 Windows 内部原理。
