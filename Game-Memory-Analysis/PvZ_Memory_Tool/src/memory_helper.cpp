#include "memory_helper.h"
#include <stdio.h>

// ============================================================================
// 调试权限提升
// ============================================================================
BOOL MemoryEnableDebugPrivilege()
{
    HANDLE hToken;
    LUID sedebugNameValue;
    TOKEN_PRIVILEGES tkp;

    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        printf("[!] OpenProcessToken 失败, GetLastError: %lu\n", GetLastError());
        return FALSE;
    }

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugNameValue))
    {
        printf("[!] LookupPrivilegeValue 失败, GetLastError: %lu\n", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = sedebugNameValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
    {
        printf("[!] AdjustTokenPrivileges 失败, GetLastError: %lu\n", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    printf("[+] 调试权限已提升\n");
    return TRUE;
}

// ============================================================================
// 按窗口标题查找进程 ID
// ============================================================================
DWORD MemoryFindProcessByWindow(LPCWSTR windowTitle)
{
    HWND hWnd = FindWindowW(NULL, windowTitle);
    if (!hWnd)
    {
        printf("[!] 未找到窗口: %ls\n", windowTitle);
        return 0;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    printf("[+] 找到游戏进程 PID: %lu\n", pid);
    return pid;
}

// ============================================================================
// 打开进程句柄
// ============================================================================
HANDLE MemoryOpenProcess(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess)
    {
        printf("[!] 打开进程失败 (PID: %lu), GetLastError: %lu\n", pid, GetLastError());
        return NULL;
    }
    printf("[+] 进程句柄已打开 (PID: %lu)\n", pid);
    return hProcess;
}

// ============================================================================
// 进程内存读取
// ============================================================================
BOOL MemoryRead(HANDLE hProcess, LPCVOID address, LPVOID buffer, SIZE_T size)
{
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, address, buffer, size, &bytesRead) || bytesRead != size)
    {
        printf("[!] 读取内存失败, 地址: 0x%p, 大小: %zu\n", address, size);
        return FALSE;
    }
    return TRUE;
}

// ============================================================================
// 进程内存写入
// ============================================================================
BOOL MemoryWrite(HANDLE hProcess, LPVOID address, LPCVOID buffer, SIZE_T size)
{
    if (!WriteProcessMemory(hProcess, address, buffer, size, NULL))
    {
        printf("[!] 写入内存失败, 地址: 0x%p, 大小: %zu\n", address, size);
        return FALSE;
    }
    printf("[+] 写入成功, 地址: 0x%p, 大小: %zu 字节\n", address, size);
    return TRUE;
}

// ============================================================================
// 多级指针解引用 - 读取
//
// 遍历多级偏移链：baseAddr → +offsets[0] → Read → +offsets[1] → Read → ...
// ============================================================================
BOOL MemoryReadMultiLevelPointer(
    HANDLE hProcess,
    DWORD baseAddress,
    const DWORD offsets[],
    int offsetCount,
    DWORD* outValue)
{
    if (offsetCount < 1 || outValue == NULL)
        return FALSE;

    DWORD currentAddr = baseAddress;

    // 读取第一级
    if (!MemoryRead(hProcess, (LPCVOID)(DWORD_PTR)currentAddr, &currentAddr, sizeof(DWORD)))
    {
        printf("[!] 多级指针读取失败: 第0级 (基址 0x%08X)\n", baseAddress);
        return FALSE;
    }

    // 遍历后续偏移
    for (int i = 0; i < offsetCount; i++)
    {
        currentAddr += offsets[i];

        if (i == offsetCount - 1)
        {
            // 最后一级：直接读取目标值
            if (!MemoryRead(hProcess, (LPCVOID)(DWORD_PTR)currentAddr, outValue, sizeof(DWORD)))
            {
                printf("[!] 多级指针读取失败: 第%d级 (地址 0x%08X)\n", i + 1, currentAddr);
                return FALSE;
            }
        }
        else
        {
            // 中间级：读取下一级指针
            if (!MemoryRead(hProcess, (LPCVOID)(DWORD_PTR)currentAddr, &currentAddr, sizeof(DWORD)))
            {
                printf("[!] 多级指针读取失败: 第%d级 (地址 0x%08X)\n", i + 1, currentAddr);
                return FALSE;
            }
        }
    }

    return TRUE;
}

// ============================================================================
// 多级指针解引用 - 写入
// ============================================================================
BOOL MemoryWriteMultiLevelPointer(
    HANDLE hProcess,
    DWORD baseAddress,
    const DWORD offsets[],
    int offsetCount,
    DWORD value)
{
    if (offsetCount < 1)
        return FALSE;

    DWORD currentAddr = baseAddress;

    // 读取第一级
    if (!MemoryRead(hProcess, (LPCVOID)(DWORD_PTR)currentAddr, &currentAddr, sizeof(DWORD)))
    {
        printf("[!] 多级指针写入失败: 第0级 (基址 0x%08X)\n", baseAddress);
        return FALSE;
    }

    // 遍历偏移到倒数第二级
    for (int i = 0; i < offsetCount - 1; i++)
    {
        currentAddr += offsets[i];
        if (!MemoryRead(hProcess, (LPCVOID)(DWORD_PTR)currentAddr, &currentAddr, sizeof(DWORD)))
        {
            printf("[!] 多级指针写入失败: 第%d级 (地址 0x%08X)\n", i + 1, currentAddr);
            return FALSE;
        }
    }

    // 最后一级：写入目标值
    currentAddr += offsets[offsetCount - 1];
    return MemoryWrite(hProcess, (LPVOID)(DWORD_PTR)currentAddr, &value, sizeof(DWORD));
}
