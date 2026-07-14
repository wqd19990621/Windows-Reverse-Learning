# 扫雷辅助工具 (Lenss)

基于 MFC 的 Windows 经典扫雷（winmine.exe）内存读取与自动扫雷工具。

## 功能

- ▶️ **快捷开局** — 一键启动初级 / 中级扫雷
- 🔍 **内存读取** — 实时读取雷区布局数据（数字、雷、空格分布）
- 📊 **数据可视化** — 以十六进制矩阵形式展示雷区
- 🎯 **自动扫雷** — 遍历所有非雷格子并自动点击

## 原理

1. `FindWindow` 定位 "扫雷" 窗口
2. `ReadProcessMemory` 读取雷区内存数据
3. 解析每个格子的编码值（`0x8F`=雷，`0x10`=行尾，`0x41`~`0x48`=数字1~8）
4. `SendMessage(WM_LBUTTONDOWN/UP)` 模拟鼠标点击非雷格子

## 项目结构

```
├── src/
│   ├── MemoryReader.h        # 封装 ReadProcessMemory
│   ├── MemoryReader.cpp      # 实现
│   ├── MainDialog.h          # MFC 对话框头文件
│   ├── MainDialog.cpp        # MFC 对话框实现（核心逻辑）
│   └── resource.h            # 资源ID定义
├── analysis/
│   ├── offsets.md            # 关键内存偏移记录
│   └── ce_screenshots/       # Cheat Engine 截图
├── README.md
└── .gitignore
```

## 编译 & 运行

### 环境要求

- Windows 7 或更高版本
- Visual Studio 2022（含 **MFC 组件**）
- 经典扫雷 `winmine.exe`（本工具仅适配 Win7 版扫雷）

### 编译

```powershell
# 使用 VS 开发者命令行或直接打开 .sln 编译
msbuild Lenss.slnx /p:Configuration=Release /p:Platform=x64
```

### 以管理员权限运行

> ⚠️ 必须右键 "以管理员身份运行"，否则 `OpenProcess(PROCESS_ALL_ACCESS)` 将失败。

## 关键内存偏移

详见 [analysis/offsets.md](analysis/offsets.md)

| 地址 | 类型 | 含义 |
|------|------|------|
| `0x01005361` | `BYTE[24][32]` | 雷区数据 |
| `0x01005338` | `DWORD` | 雷区高度 |
| `0x01005194` | `DWORD` | 测试基址 |

## 免责声明

本项目仅供学习 Windows API（`ReadProcessMemory`、`SendMessage`、MFC）之用。
请勿用于任何破坏游戏公平性或违反游戏服务条款的行为。

## 许可

MIT License
