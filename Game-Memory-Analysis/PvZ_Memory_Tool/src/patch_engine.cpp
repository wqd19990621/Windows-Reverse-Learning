#include "patch_engine.h"
#include "offsets.h"
#include "memory_helper.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// 状态跟踪 — 保存原始字节用于恢复
// ============================================================================
static BYTE g_plantCD_original[PLANT_CD_PATCH_SIZE]     = { 0 };
static BYTE g_chomperCD_original[CHOMPER_CD_PATCH_SIZE]  = { 0 };
static BYTE g_invincible_original[PLANT_INVINCIBLE_SIZE] = { 0 };

static BOOL g_plantCD_active     = FALSE;
static BOOL g_chomperCD_active   = FALSE;
static BOOL g_invincible_active  = FALSE;

// ============================================================================
// NOP 注入
// ============================================================================
BOOL PatchNop(HANDLE hProcess, DWORD address, int byteCount, BYTE* savedOriginal)
{
    if (byteCount <= 0 || byteCount > 256)
        return FALSE;

    // 保存原始字节
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, (LPCVOID)(DWORD_PTR)address, savedOriginal, byteCount, &bytesRead)
        || bytesRead != (SIZE_T)byteCount)
    {
        printf("[!] PatchNop: 读取原始字节失败, 地址: 0x%08X\n", address);
        return FALSE;
    }

    // 写入 NOP 指令
    BYTE nopBuf[256];
    memset(nopBuf, 0x90, byteCount);

    if (!WriteProcessMemory(hProcess, (LPVOID)(DWORD_PTR)address, nopBuf, byteCount, NULL))
    {
        printf("[!] PatchNop: 写入 NOP 失败, 地址: 0x%08X\n", address);
        return FALSE;
    }

    printf("[+] NOP 注入成功: 地址 0x%08X, %d 字节\n", address, byteCount);
    return TRUE;
}

// ============================================================================
// 恢复原始字节
// ============================================================================
BOOL PatchRestore(HANDLE hProcess, DWORD address, const BYTE* original, int byteCount)
{
    if (!WriteProcessMemory(hProcess, (LPVOID)(DWORD_PTR)address, original, byteCount, NULL))
    {
        printf("[!] PatchRestore: 恢复失败, 地址: 0x%08X\n", address);
        return FALSE;
    }
    printf("[+] 恢复原始指令成功: 地址 0x%08X, %d 字节\n", address, byteCount);
    return TRUE;
}

// ============================================================================
// 字节补丁
// ============================================================================
BOOL PatchBytes(HANDLE hProcess, DWORD address, const BYTE* bytes, int count)
{
    return MemoryWrite(hProcess, (LPVOID)(DWORD_PTR)address, bytes, count);
}

// ============================================================================
// 远程 Shellcode 执行
//
// 流程：
//   1. VirtualAllocEx 在目标进程分配可执行内存
//   2. WriteProcessMemory 写入 Shellcode
//   3. CreateRemoteThread 在目标进程创建线程执行
//   4. WaitForSingleObject 等待线程完成
//   5. VirtualFreeEx 释放内存
// ============================================================================
BOOL PatchExecuteShellcode(HANDLE hProcess, const BYTE* shellcode, SIZE_T size)
{
    if (!shellcode || size == 0 || size > 4096)
    {
        printf("[!] Shellcode 无效 (size=%zu)\n", size);
        return FALSE;
    }

    // 1. 在目标进程分配可执行内存
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, size,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!pRemoteMem)
    {
        printf("[!] VirtualAllocEx 失败, GetLastError: %lu\n", GetLastError());
        return FALSE;
    }
    printf("[+] 远程内存已分配: 0x%p (%zu 字节)\n", pRemoteMem, size);

    // 2. 写入 Shellcode
    if (!WriteProcessMemory(hProcess, pRemoteMem, shellcode, size, NULL))
    {
        printf("[!] 写入 Shellcode 失败\n");
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        return FALSE;
    }
    printf("[+] Shellcode 已写入远程内存\n");

    // 3. 创建远程线程执行
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)pRemoteMem, NULL, 0, NULL);
    if (!hThread)
    {
        printf("[!] CreateRemoteThread 失败, GetLastError: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        return FALSE;
    }

    // 4. 等待执行完成
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    // 5. 释放远程内存
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);

    printf("[+] Shellcode 执行完成\n");
    return TRUE;
}

// ============================================================================
// 切换植物 CD
// ============================================================================
BOOL PatchTogglePlantCD(HANDLE hProcess, BOOL enable)
{
    if (enable && !g_plantCD_active)
    {
        if (PatchNop(hProcess, PLANT_CD_PATCH_ADDR, PLANT_CD_PATCH_SIZE, g_plantCD_original))
        {
            g_plantCD_active = TRUE;
            printf("[+] 植物无冷却: 已启用\n");
            return TRUE;
        }
    }
    else if (!enable && g_plantCD_active)
    {
        if (PatchRestore(hProcess, PLANT_CD_PATCH_ADDR, g_plantCD_original, PLANT_CD_PATCH_SIZE))
        {
            g_plantCD_active = FALSE;
            printf("[+] 植物无冷却: 已关闭\n");
            return TRUE;
        }
    }
    else
    {
        printf("[i] 植物无冷却: 状态未变化 (当前=%s)\n", g_plantCD_active ? "开启" : "关闭");
    }
    return FALSE;
}

// ============================================================================
// 切换大嘴花 CD
// ============================================================================
BOOL PatchToggleChomperCD(HANDLE hProcess, BOOL enable)
{
    if (enable && !g_chomperCD_active)
    {
        if (PatchNop(hProcess, CHOMPER_CD_PATCH_ADDR, CHOMPER_CD_PATCH_SIZE, g_chomperCD_original))
        {
            g_chomperCD_active = TRUE;
            printf("[+] 大嘴花无冷却: 已启用\n");
            return TRUE;
        }
    }
    else if (!enable && g_chomperCD_active)
    {
        if (PatchRestore(hProcess, CHOMPER_CD_PATCH_ADDR, g_chomperCD_original, CHOMPER_CD_PATCH_SIZE))
        {
            g_chomperCD_active = FALSE;
            printf("[+] 大嘴花无冷却: 已关闭\n");
            return TRUE;
        }
    }
    else
    {
        printf("[i] 大嘴花无冷却: 状态未变化 (当前=%s)\n", g_chomperCD_active ? "开启" : "关闭");
    }
    return FALSE;
}

// ============================================================================
// 切换植物无敌
// ============================================================================
BOOL PatchTogglePlantInvincible(HANDLE hProcess, BOOL enable)
{
    if (enable && !g_invincible_active)
    {
        // 先读取原始值
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(hProcess, (LPCVOID)(DWORD_PTR)PLANT_INVINCIBLE_ADDR,
            g_invincible_original, PLANT_INVINCIBLE_SIZE, &bytesRead))
        {
            printf("[!] 读取植物无敌原始字节失败\n");
            return FALSE;
        }

        // 写入 0x00（将 -04 改为 +00）
        BYTE zero = 0x00;
        if (MemoryWrite(hProcess, (LPVOID)(DWORD_PTR)PLANT_INVINCIBLE_ADDR, &zero, PLANT_INVINCIBLE_SIZE))
        {
            g_invincible_active = TRUE;
            printf("[+] 植物无敌: 已启用\n");
            return TRUE;
        }
    }
    else if (!enable && g_invincible_active)
    {
        if (PatchRestore(hProcess, PLANT_INVINCIBLE_ADDR, g_invincible_original, PLANT_INVINCIBLE_SIZE))
        {
            g_invincible_active = FALSE;
            printf("[+] 植物无敌: 已关闭\n");
            return TRUE;
        }
    }
    else
    {
        printf("[i] 植物无敌: 状态未变化 (当前=%s)\n", g_invincible_active ? "开启" : "关闭");
    }
    return FALSE;
}

// ============================================================================
// 樱桃炸弹种植 Shellcode
//
// 等效汇编逻辑：
//   pushad
//   push -1              ; slot = 自动选择
//   push 2               ; plant_type = 2 (樱桃炸弹)
//   mov  eax, y          ; y 坐标
//   push x               ; x 坐标
//   mov  ebx, [0x6A9EC0] ; 游戏基址
//   mov  ebx, [ebx+0x768]; 游戏对象指针
//   push ebx             ; this 指针
//   mov  edx, 0x40D130   ; SpawnPlant 函数地址
//   call edx
//   popad
//   ret
// ============================================================================
BOOL PatchSpawnCherryBomb(HANDLE hProcess, int x, int y)
{
    if (x < 0 || x > GRID_MAX_X || y < 0 || y > GRID_MAX_Y)
    {
        printf("[!] 坐标超出范围: (%d, %d), 有效范围: x(0~%d) y(0~%d)\n",
            x, y, GRID_MAX_X, GRID_MAX_Y);
        return FALSE;
    }

    // 手写 Shellcode 字节码 (x86 32-bit)
    // 使用 push 0x7F 作为坐标占位符，运行时替换为实际坐标
    BYTE shellcode[] = {
        0x60,                               // pushad
        0x6A, 0xFF,                         // push -1 (slot = auto)
        0x6A, (BYTE)CHERRY_BOMB_TYPE,       // push 2 (plant type)
        0xB8, (BYTE)y, 0x00, 0x00, 0x00,   // mov eax, y
        0x6A, (BYTE)x,                       // push x
        0x8B, 0x1D, 0xC0, 0x9E, 0x6A, 0x00, // mov ebx, [0x6A9EC0]
        0x8B, 0x9B, 0x68, 0x07, 0x00, 0x00, // mov ebx, [ebx+0x768]
        0x53,                               // push ebx
        0xBA, 0x30, 0xD1, 0x40, 0x00,       // mov edx, 0x40D130
        0xFF, 0xD2,                         // call edx
        0x61,                               // popad
        0xC3                                // ret
    };

    SIZE_T shellcodeSize = sizeof(shellcode);

    printf("[+] 种植樱桃炸弹: (%d, %d)\n", x, y);
    return PatchExecuteShellcode(hProcess, shellcode, shellcodeSize);
}
