# PvZ Win32 Memory Modifier

植物大战僵尸中文版 Win32 内存修改工具

基于 Win32 API 的控制台应用程序，通过直接操作游戏进程内存实现多种修改功能。所有功能无需注入 DLL，纯外部进程操作。

## 功能特性

| # | 功能 | 实现技术 | 难度 |
|---|------|----------|------|
| 1 | 无限阳光 | 多级指针解引用 + WriteProcessMemory | ★★☆ |
| 2 | 无限金币 | 多级指针解引用 + WriteProcessMemory | ★★☆ |
| 3 | 自定义阳光 | 用户输入 + 多级指针写入 | ★★☆ |
| 4 | 自定义金币 | 用户输入 + 多级指针写入 | ★★☆ |
| 5 | 植物无冷却 | 代码段 NOP 注入 (JLE → NOP) | ★★★ |
| 6 | 大嘴花无冷却 | 代码段 NOP 注入 (JNE → NOP) | ★★★ |
| 7 | 植物无敌 | 单字节代码修改 (-04 → 00) | ★★☆ |
| 8 | 樱桃炸弹 | Shellcode 远程线程注入 | ★★★★ |
| 9 | 全图樱桃炸弹 | Shellcode 批量循环注入 | ★★★★☆ |

## 内存地址参考

```
阳光:  [[0x6A9EC0 + 0x768] + 0x5560]
金币:  [[0x6A9EC0 + 0x82C] + 0x28]

植物CD NOP:    0x004875E6  (jle 0x14 → nop nop)
大嘴花CD NOP:   0x004616E5  (jne 0x5F → nop nop)
植物无敌:       0x00530043  (0xFC → 0x00, 将 -04 改为 +00)
SpawnPlant:    0x0040D130  (种植函数)
```

## 快速开始

### 环境要求

- Windows 10/11
- Visual Studio 2022（含 C++ 桌面开发）或 CMake 3.15+
- 植物大战僵尸中文版

### 编译

```powershell
# CMake 方式
cmake -B build -S .
cmake --build build --config Release

# 或直接使用 MSVC
cl /EHsc /std:c++17 src\*.cpp /Fe:pvz_tool.exe
```

### 运行

```
# 1. 启动植物大战僵尸中文版
# 2. 以管理员身份运行
pvz_tool.exe

# 3. 连接游戏
pvz> connect

# 4. 输入功能编号
pvz> 1     # 无限阳光
pvz> 5     # 切换植物无CD
pvz> 9     # 全图樱桃炸弹
```

## 项目结构

```
PvZ_Memory_Tool/
├── .gitignore
├── LICENSE                         # MIT + 学习用途声明
├── README.md
├── CMakeLists.txt
├── src/                            # 核心源码
│   ├── main.cpp                    # 控制台入口 + 菜单
│   ├── memory_helper.h / .cpp      # Win32 进程/内存操作封装
│   ├── offsets.h                   # 游戏内存偏移定义
│   └── patch_engine.h / .cpp       # NOP注入/Shellcode引擎
├── tools/
│   └── find_offset.py              # CE特征码扫描辅助工具
├── docs/
│   ├── analysis_report.md          # 完整逆向分析报告
│   ├── win32_api_notes.md          # Win32 API 函数详解
│   └── screenshots/                # 调试截图
└── tests/
    └── test_memory_read.cpp        # 内存读取单元测试
```

## 核心技术要点

### 多级指针解引用

游戏使用指针链存储阳光和金币值。需要通过三次 `ReadProcessMemory` 调用解引用：

```
Read(0x6A9EC0) → ptr1 → ptr1+0x768 → Read() → ptr2 → ptr2+0x5560 → Read() → 阳光值
```

### 代码段 NOP 注入

通过 `WriteProcessMemory` 修改游戏 `.text` 段指令：

```
原始: 7E 14  (JLE  = 冷却未结束则跳转)
修改: 90 90  (NOP = 始终通过)
```

### Shellcode 远程线程注入

在目标进程分配可执行内存，写入 Shellcode，创建远程线程调用游戏内部函数：

```
VirtualAllocEx → WriteProcessMemory → CreateRemoteThread → WaitForSingleObject
```

详见 [`docs/analysis_report.md`](docs/analysis_report.md)。

## 免责声明

本项目仅供学习研究：
- Windows 进程内存管理
- Win32 API 进程间通信
- 逆向工程与汇编分析
- Shellcode 编写与代码注入

**禁止用于任何违反游戏服务条款或法律法规的用途。**
