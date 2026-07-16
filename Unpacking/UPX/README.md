# UPX 壳脱壳分析 —— 扫雷 (saolei.exe)

> **脱壳结果：✅ 成功** | 使用 ESP 定律法 + Scylla 修复 IAT，脱壳后程序可正常运行

---

## 📋 项目概述

本项目记录了 Windows XP 经典扫雷程序 (`saolei.exe`) 的 UPX 加壳与手动脱壳全过程，包含 PE 结构对比分析。

| 项目 | 说明 |
|------|------|
| **原始程序** | `saolei.exe` — Windows 扫雷游戏 (Microsoft Visual C++ 编译) |
| **加壳工具** | UPX 3.05 |
| **加壳程序** | `ke_saolei.exe` — UPX 压缩后的程序 |
| **脱壳方法** | ESP 定律法 (硬件访问断点) |
| **修复工具** | Scylla (IAT 修复) + CFF Explorer (区段名还原) |
| **脱壳程序** | `ke_saolei_dump.exe` — 脱壳修复后的程序 |

---

## 🔍 脱壳流程

### 第一步：ESP 定律定位 OEP

```
1. x64dbg 载入 ke_saolei.exe
2. 看到入口点 pushad（UPX 壳特征）
3. F8 单步执行 pushad 后，记录 ESP 值
4. 对 ESP 指向的内存地址下硬件访问断点 (DWord)
5. F9 运行 → 断在 popad 后的 jmp 处
6. 单步跟踪到达 OEP：0x01003E21
```

**OEP 与原始程序入口点完全一致** ✅

### 第二步：Dump 内存

```
1. 在 OEP 处打开 Scylla 插件
2. 填写 OEP: 0x00003E21 (RVA)
3. 点击 "Dump" → 保存为 ke_saolei_dump.exe
```

### 第三步：IAT 修复

```
1. Scylla → IAT Autosearch（自动搜索 IAT）
2. Get Imports（获取导入表）
3. Fix Dump → 选中 ke_saolei_dump.exe
4. 生成修复文件 ke_saolei_dump_SCY.exe
```

### 第四步：区段名还原

```
使用 CFF Explorer 手动修改区段名：
  UPX0 → .text
  UPX1 → .rdata
  (rsrc 保持不变)
```

---

## 📊 三方 PE 信息对比

| 属性 | 原始程序 (saolei.exe) | 加壳程序 (ke_saolei.exe) | 脱壳程序 (dump) |
|------|----------------------|-------------------------|-----------------|
| **文件大小** | 119,808 字节 (117 KB) | 97,280 字节 (95 KB) | 166,400 字节 (162 KB) |
| **入口点 RVA** | `0x00003E21` | `0x00021CA0` | `0x00003E21` ✅ |
| **映像基址** | `0x01000000` | `0x01000000` | `0x01000000` ✅ |
| **区段数量** | 3 | 3 | 4 (+1 Scylla) |
| **映像大小** | `0x20000` | `0x29000` | `0x2A000` |
| **子系统** | Windows GUI | Windows GUI | Windows GUI ✅ |
| **链接器版本** | 7.0 | 7.0 | 7.0 ✅ |

### 区段对比

| # | 原始程序 | 加壳程序 | 脱壳程序（修复后） |
|---|---------|---------|-----------------|
| 0 | `.text` (代码) | `UPX0` | `.text` ✅ |
| 1 | `.data` (数据) | `UPX1` | `.rdata` |
| 2 | `.rsrc` (资源) | `.rsrc` | `.rsrc` |
| 3 | — | — | `.SCY` (Scylla IAT) |

### 导入表对比

| DLL | 原始程序 | 加壳程序 | 脱壳程序 |
|-----|---------|---------|---------|
| KERNEL32.dll | ✅ 正常 | ❌ IAT 解析失败 | ✅ 序数可用 (名称 RVA 部分失败) |
| USER32.dll | ✅ 正常 | ❌ IAT 解析失败 | ✅ 序数可用 (名称 RVA 部分失败) |
| GDI32.dll | ✅ 正常 | ❌ IAT 解析失败 | ✅ 序数可用 (名称 RVA 部分失败) |
| ADVAPI32.dll | ✅ 正常 | ❌ IAT 解析失败 | ✅ 序数可用 (名称 RVA 部分失败) |
| COMCTL32.dll | ✅ 正常 | ❌ IAT 解析失败 | ✅ 序数可用 (名称 RVA 部分失败) |
| SHELL32.dll | ✅ 正常 | ❌ IAT 解析失败 | ✅ 序数可用 (名称 RVA 部分失败) |
| WINMM.dll | ✅ 正常 | ❌ IAT 解析失败 | ✅ 序数可用 (名称 RVA 部分失败) |
| MSVCRT.dll | ✅ 正常 | ❌ IAT 解析失败 | ✅ 序数可用 (名称 RVA 部分失败) |

---

## ✅ 验证结果

| 验证项 | 状态 | 说明 |
|--------|------|------|
| 程序可运行 | ✅ | 脱壳后双击正常运行 |
| DIE 识别 | ✅ | 区段名还原后不再识别为 UPX 壳 |
| OEP 一致性 | ✅ | `0x00003E21` 与原始完全一致 |
| 区段名还原 | ✅ | 手动还原为 `.text` / `.rdata` / `.data` |
| IAT 可用性 | ⚠️ | 序数导入正常，部分名称 RVA 转换失败 |
| 文件大小一致 | ❌ | 脱壳后偏大 (162 KB vs 原始 117 KB) |
| 区段数量一致 | ❌ | 多出 `.SCY` 区段 (Scylla IAT 区段) |

---

## 🛠 使用工具

| 工具 | 用途 |
|------|------|
| [UPX 3.05](https://upx.github.io/) | PE 压缩加壳 |
| [x64dbg](https://x64dbg.com/) | 动态调试，寻找 OEP |
| [Scylla](https://github.com/NtQuery/Scylla) | IAT 自动搜索与修复 |
| [CFF Explorer](https://ntcore.com/explorer-suite/) | PE 区段名手动修改 |
| [DIE (Detect It Easy)](https://github.com/horsicq/Detect-It-Easy) | 壳识别 |
| [PE 解析器 (自研)]() | PE 结构自动化对比分析 |

---

## 📁 文件清单

```
UPX/
├── README.md                 # 本文件
├── UPX壳分析.docx            # 详细分析文档 (含截图)
├── saolei.exe                # 原始扫雷程序
├── ke_saolei.exe             # UPX 加壳后程序
└── ke_saolei_dump_SCY.exe    # 脱壳修复后程序
```

---

## 📝 结论

UPX 壳手动脱壳 **功能性成功**：

1. **ESP 定律法**精准定位 OEP，与原始程序入口点一致
2. **Scylla** 成功修复 IAT，程序正常运行
3. **CFF Explorer** 手动还原区段名，DIE 不再报警
4. 残留差异 (`SCY` 区段、文件偏大) 是 Scylla 修复工具的正常副作用，**不影响运行**

> 💡 **经验总结**：UPX 压缩壳脱壳的核心在于 OEP 定位 + IAT 修复两步。ESP 定律对 UPX 等压缩壳非常有效，但 IAT 修复需要仔细验证每个 DLL 的导入函数。

---

## 📄 License

本项目仅供学习研究使用。原始 `saolei.exe` 版权归 Microsoft 所有。
