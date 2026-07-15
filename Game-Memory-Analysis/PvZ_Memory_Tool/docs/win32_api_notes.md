# Win32 API 详解

本文档详细说明项目中使用的 Windows API 函数，包括功能、参数和在本项目中的具体用途。

---

## 一、权限提升

### OpenProcessToken

```c
BOOL OpenProcessToken(
    HANDLE  ProcessHandle,   // 进程句柄 (GetCurrentProcess())
    DWORD   DesiredAccess,   // TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
    PHANDLE TokenHandle      // 输出: 令牌句柄
);
```

**用途**: 获取当前进程的访问令牌句柄，用于后续权限调整。

### LookupPrivilegeValue

```c
BOOL LookupPrivilegeValueW(
    LPCWSTR lpSystemName,    // NULL = 本地系统
    LPCWSTR lpName,          // SE_DEBUG_NAME
    PLUID   lpLuid           // 输出: LUID
);
```

**用途**: 将权限名称 (`SE_DEBUG_NAME`) 转换为系统唯一的 LUID 值。

### AdjustTokenPrivileges

```c
BOOL AdjustTokenPrivileges(
    HANDLE            TokenHandle,
    BOOL              DisableAllPrivileges,  // FALSE = 只调整指定的
    PTOKEN_PRIVILEGES NewState,              // 新权限状态
    DWORD             BufferLength,           // 0 = 不使用 PreviousState
    PTOKEN_PRIVILEGES PreviousState,          // NULL
    PDWORD            ReturnLength            // NULL
);
```

**用途**: 启用 `SeDebugPrivilege`，允许进程访问其他进程的内存空间。

### 在本项目中的位置

```cpp
// src/memory_helper.cpp
BOOL MemoryEnableDebugPrivilege()
```

---

## 二、进程发现与句柄管理

### FindWindow

```c
HWND FindWindowW(
    LPCWSTR lpClassName,     // NULL = 不按类名搜索
    LPCWSTR lpWindowName     // 窗口标题
);
```

**用途**: 通过窗口标题查找游戏窗口句柄。

### GetWindowThreadProcessId

```c
DWORD GetWindowThreadProcessId(
    HWND  hWnd,              // 窗口句柄
    LPDWORD lpdwProcessId    // 输出: 进程 ID
);
```

**用途**: 从窗口句柄获取游戏进程的 PID。

### OpenProcess

```c
HANDLE OpenProcess(
    DWORD dwDesiredAccess,   // PROCESS_ALL_ACCESS
    BOOL  bInheritHandle,    // FALSE
    DWORD dwProcessId        // 目标进程 PID
);
```

**用途**: 以完全访问权限打开游戏进程，获取操作句柄。

**安全风险**: `PROCESS_ALL_ACCESS` 请求最高权限，杀毒软件可能告警。

### CloseHandle

```c
BOOL CloseHandle(HANDLE hObject);
```

**用途**: 释放进程句柄和线程句柄，防止句柄泄漏。

### 在本项目中的位置

```cpp
// src/memory_helper.cpp
DWORD MemoryFindProcessByWindow(LPCWSTR windowTitle)
HANDLE MemoryOpenProcess(DWORD pid)
```

---

## 三、内存读写

### ReadProcessMemory

```c
BOOL ReadProcessMemory(
    HANDLE  hProcess,              // 目标进程句柄
    LPCVOID lpBaseAddress,         // 远程内存地址
    LPVOID  lpBuffer,              // 本地缓冲区
    SIZE_T  nSize,                 // 读取大小
    SIZE_T  *lpNumberOfBytesRead   // 实际读取字节数
);
```

**用途**: 从游戏进程内存中读取数据（阳光值、金币值、指针等）。

**关键特性**:
- 跨越进程边界读取内存
- 需要 `PROCESS_VM_READ` 权限
- 读取失败不抛异常，返回 FALSE

### WriteProcessMemory

```c
BOOL WriteProcessMemory(
    HANDLE  hProcess,
    LPVOID  lpBaseAddress,         // 远程内存地址
    LPCVOID lpBuffer,              // 本地数据缓冲区
    SIZE_T  nSize,                 // 写入大小
    SIZE_T  *lpNumberOfBytesWritten
);
```

**用途**: 向游戏进程内存写入数据（修改阳光、金币，写入 NOP 指令）。

**关键特性**:
- 可以修改代码段（`.text`）实现指令注入
- 需要 `PROCESS_VM_WRITE` 和 `PROCESS_VM_OPERATION` 权限
- 对代码段的写入可能触发 DEP 保护（本项目写入的是已标记为可执行的区域）

### 多级指针读取流程

```
ReadProcessMemory(0x6A9EC0) → 一级指针
一级指针 + offset1 → ReadProcessMemory → 二级指针
二级指针 + offset2 → ReadProcessMemory → 目标值
```

### 在本项目中的位置

```cpp
// src/memory_helper.cpp
BOOL MemoryRead(HANDLE, LPCVOID, LPVOID, SIZE_T)
BOOL MemoryWrite(HANDLE, LPVOID, LPCVOID, SIZE_T)
BOOL MemoryReadMultiLevelPointer(...)
BOOL MemoryWriteMultiLevelPointer(...)
```

---

## 四、远程内存分配与 Shellcode 执行

### VirtualAllocEx

```c
LPVOID VirtualAllocEx(
    HANDLE hProcess,               // 目标进程句柄
    LPVOID lpAddress,              // NULL = 系统自动选择
    SIZE_T dwSize,                 // 分配大小
    DWORD  flAllocationType,       // MEM_COMMIT | MEM_RESERVE
    DWORD  flProtect               // PAGE_EXECUTE_READWRITE
);
```

**用途**: 在游戏进程的虚拟地址空间中分配可执行内存，用于存放 Shellcode。

**标志说明**:
- `MEM_COMMIT`: 立即分配物理内存
- `MEM_RESERVE`: 保留虚拟地址空间
- `PAGE_EXECUTE_READWRITE`: 允许读写和执行（Shellcode 必需）

### WriteProcessMemory (Shellcode 写入)

将 Shellcode 字节数组写入上一步分配的远程内存区域。

### CreateRemoteThread

```c
HANDLE CreateRemoteThread(
    HANDLE                 hProcess,
    LPSECURITY_ATTRIBUTES  lpThreadAttributes,  // NULL
    SIZE_T                 dwStackSize,          // 0 = 默认
    LPTHREAD_START_ROUTINE lpStartAddress,       // Shellcode 地址
    LPVOID                 lpParameter,          // NULL
    DWORD                  dwCreationFlags,      // 0 = 立即运行
    LPDWORD                lpThreadId            // NULL
);
```

**用途**: 在游戏进程中创建线程，以 Shellcode 地址为入口点执行。

**关键要点**:
- Shellcode 必须以 `ret` 指令结尾
- 线程执行的代码在目标进程上下文中运行
- 这是游戏修改中最"底层"的技术之一

### WaitForSingleObject

```c
DWORD WaitForSingleObject(
    HANDLE hHandle,        // 线程句柄
    DWORD  dwMilliseconds  // INFINITE = 无限等待
);
```

**用途**: 等待远程线程执行完毕，确保 Shellcode 完全执行后再继续。

### VirtualFreeEx

```c
BOOL VirtualFreeEx(
    HANDLE hProcess,
    LPVOID lpAddress,              // 远程内存地址
    SIZE_T dwSize,                 // 0 = 释放整个区域
    DWORD  dwFreeType              // MEM_RELEASE
);
```

**用途**: 释放之前分配的远程内存，防止内存泄漏。

### 完整 Shellcode 注入流程

```
1. VirtualAllocEx    → 在游戏进程分配 PAGE_EXECUTE_READWRITE 内存
2. WriteProcessMemory → 将 Shellcode 字节码写入远程内存
3. CreateRemoteThread → 创建远程线程，入口点 = Shellcode 地址
4. WaitForSingleObject → 等待线程执行完成
5. VirtualFreeEx     → 释放远程内存
```

### 在本项目中的位置

```cpp
// src/patch_engine.cpp
BOOL PatchExecuteShellcode(HANDLE hProcess, const BYTE* shellcode, SIZE_T size)
BOOL PatchSpawnCherryBomb(HANDLE hProcess, int x, int y)
```

---

## 五、汇编指令参考

### NOP 指令 (0x90)

```
NOP = No Operation（空操作）
只消耗一个 CPU 周期，不改变任何寄存器或内存状态
```

### JLE (0x7E)

```
JLE = Jump if Less than or Equal（小于等于时跳转）
基于 ZF 和 SF 标志位
用于实现植物冷却的"如果冷却未结束则等待"逻辑
```

### JNE (0x75)

```
JNE = Jump if Not Equal（不等时跳转）
基于 ZF 标志位
用于实现大嘴花吞噬的冷却检查
```

---

## 六、错误处理

本项目对每个 Win32 API 调用都进行了错误检查：

```cpp
if (!SomeWin32Function(...))
{
    printf("[!] 函数名 失败, GetLastError: %lu\n", GetLastError());
    // 清理资源
    return FALSE;
}
```

`GetLastError()` 获取最后一次 API 调用的错误代码，对调试至关重要。

### 常见错误码

| 代码 | 含义 | 可能原因 |
|------|------|----------|
| 5 | ERROR_ACCESS_DENIED | 权限不足，需管理员运行 |
| 87 | ERROR_INVALID_PARAMETER | 参数错误 |
| 299 | ERROR_PARTIAL_COPY | ReadProcessMemory 仅读取了部分 |

---

## 参考资料

- [Microsoft Win32 API 文档](https://docs.microsoft.com/en-us/windows/win32/api/)
- [Windows Internals, Part 1 (7th Edition)](https://learn.microsoft.com/en-us/sysinternals/resources/windows-internals)
- [Practical Malware Analysis](https://nostarch.com/malware) - 第11章: 恶意代码的隐蔽执行
