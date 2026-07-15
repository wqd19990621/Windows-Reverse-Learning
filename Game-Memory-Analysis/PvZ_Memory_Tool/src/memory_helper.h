#pragma once
#include <Windows.h>
#include <stdint.h>

// ============================================================================
// 游戏内存操作封装
//
// 封装 Win32 API 进程操作，提供：
//   1. 调试权限提升
//   2. 按窗口标题查找进程
//   3. 安全的进程内存读写
//   4. 多级指针解引用
// ============================================================================

// --- 权限与进程管理 ---

// 提升当前进程为调试权限（SE_DEBUG_NAME）
// 返回 TRUE 表示成功获取权限
BOOL MemoryEnableDebugPrivilege();

// 通过窗口标题查找进程 ID
// 返回 0 表示未找到
DWORD MemoryFindProcessByWindow(LPCWSTR windowTitle);

// 以完全访问权限打开目标进程
// 返回 NULL 表示失败
HANDLE MemoryOpenProcess(DWORD pid);

// --- 内存读写 ---

// 读取目标进程内存（失败时弹出消息框提示）
BOOL MemoryRead(HANDLE hProcess, LPCVOID address, LPVOID buffer, SIZE_T size);

// 写入目标进程内存（失败时弹出消息框提示）
BOOL MemoryWrite(HANDLE hProcess, LPVOID address, LPCVOID buffer, SIZE_T size);

// --- 多级指针解引用 ---

// 从基址出发，通过多级偏移解引用，读取最终地址处的 DWORD 值
// baseAddress: 起始基址
// offsets:     偏移数组
// offsetCount: 偏移层数
// outValue:    输出最终读取的值
// 返回 TRUE 表示成功
BOOL MemoryReadMultiLevelPointer(
    HANDLE hProcess,
    DWORD baseAddress,
    const DWORD offsets[],
    int offsetCount,
    DWORD* outValue
);

// 从基址出发，通过多级偏移解引用，向最终地址写入 DWORD 值
BOOL MemoryWriteMultiLevelPointer(
    HANDLE hProcess,
    DWORD baseAddress,
    const DWORD offsets[],
    int offsetCount,
    DWORD value
);
