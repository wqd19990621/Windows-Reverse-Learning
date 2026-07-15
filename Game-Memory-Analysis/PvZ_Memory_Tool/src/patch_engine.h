#pragma once
#include <Windows.h>
#include <stdint.h>

// ============================================================================
// 代码修改引擎
//
// 提供运行时代码注入 / 修改能力：
//   1. NOP 注入（用于移除条件跳转 / 冷却检查）
//   2. 字节级补丁（写入 / 恢复原始指令）
//   3. Shellcode 远程线程注入（用于调用游戏内部函数）
// ============================================================================

// --- NOP 注入 ---

// 将指定地址的 N 个字节替换为 NOP (0x90)
// 保存原始字节以便后续恢复
BOOL PatchNop(HANDLE hProcess, DWORD address, int byteCount, BYTE* savedOriginal);

// 恢复之前保存的原始字节
BOOL PatchRestore(HANDLE hProcess, DWORD address, const BYTE* original, int byteCount);

// --- 字节补丁 ---

// 向指定地址写入自定义字节数组
BOOL PatchBytes(HANDLE hProcess, DWORD address, const BYTE* bytes, int count);

// --- 远程 Shellcode 注入 ---

// 在目标进程中执行 Shellcode
// shellcode:  shellcode 字节数组
// size:       shellcode 大小
// 返回 TRUE 表示执行成功
BOOL PatchExecuteShellcode(HANDLE hProcess, const BYTE* shellcode, SIZE_T size);

// --- 预定义功能 ---

// 切换植物冷却 CD（TRUE = 无CD, FALSE = 恢复）
BOOL PatchTogglePlantCD(HANDLE hProcess, BOOL enable);

// 切换大嘴花吞噬 CD（TRUE = 无CD, FALSE = 恢复）
BOOL PatchToggleChomperCD(HANDLE hProcess, BOOL enable);

// 切换植物无敌（TRUE = 无敌, FALSE = 恢复）
BOOL PatchTogglePlantInvincible(HANDLE hProcess, BOOL enable);

// 在指定坐标 (x,y) 种植樱桃炸弹
BOOL PatchSpawnCherryBomb(HANDLE hProcess, int x, int y);
