#pragma once
#include <Windows.h>

// ============================================================================
// 植物大战僵尸中文版 内存偏移定义
//
// 适用版本: 植物大战僵尸中文版（特定版本）
// 查找工具: Cheat Engine 7.x / x64dbg
// 更新日期: 2026-07
// ============================================================================

// --- 游戏模块基址 ---
// 动态基址，每次运行通过进程枚举获取；静态分析中使用 0x00400000
#define PVZ_STATIC_BASE             0x006A9EC0  // 全局数据指针基址

// --- 阳光地址链 ---
// 公式: [[[0x6A9EC0] + 0x768] + 0x5560]
#define SUNLIGHT_BASE_PTR           0x006A9EC0
#define SUNLIGHT_OFFSET_L1          0x768
#define SUNLIGHT_OFFSET_L2          0x5560
#define SUNLIGHT_DEFAULT_VALUE      9999
#define SUNLIGHT_MAX_VALUE          999999

// --- 金币地址链 ---
// 公式: [[[0x6A9EC0] + 0x82C] + 0x28]
#define GOLD_BASE_PTR               0x006A9EC0
#define GOLD_OFFSET_L1              0x82C
#define GOLD_OFFSET_L2              0x28
#define GOLD_DEFAULT_VALUE          9999
#define GOLD_MAX_VALUE              999999

// --- 植物冷却 CD 代码位置 ---
// 原始指令: jle plantsvszombies.4875FC  (7E 14)
// 修改为:    nop; nop                  (90 90)
#define PLANT_CD_PATCH_ADDR         0x004875E6
#define PLANT_CD_PATCH_SIZE         2

// --- 大嘴花吞噬冷却 CD 代码位置 ---
// 原始指令: jne PlantsVsZombies.exe+61746  (75 5F)
// 修改为:    nop; nop                      (90 90)
#define CHOMPER_CD_PATCH_ADDR       0x004616E5
#define CHOMPER_CD_PATCH_SIZE       2

// --- 植物无敌代码位置 ---
// 原始指令: add dword ptr [esi+40], -04  (... 83 46 40 FC ...)
// 修改为:    add dword ptr [esi+40], +00  (... 83 46 40 00 ...)
// 需要修改的是 FC → 00，位于指令的第4个字节
#define PLANT_INVINCIBLE_ADDR       0x00530043
#define PLANT_INVINCIBLE_SIZE       1

// --- 种植函数地址 ---
// SpawnPlant(int slot, int type, int x, int y, void* gameObj)
#define SPAWN_PLANT_FUNC_ADDR       0x0040D130

// --- 游戏窗口标题 ---
#define PVZ_WINDOW_TITLE            L"植物大战僵尸中文版"

// --- 樱桃炸弹 ---
#define CHERRY_BOMB_TYPE            2       // 植物类型 ID
#define GRID_MAX_X                  7       // 列 0~7
#define GRID_MAX_Y                  4       // 行 0~4

// ============================================================================
// 原始指令字节（用于恢复）
// ============================================================================

// 植物 CD 原始指令: jle 0x14 → {0x7E, 0x14}
static const BYTE PLANT_CD_ORIGINAL_BYTES[PLANT_CD_PATCH_SIZE]   = { 0x7E, 0x14 };

// 大嘴花 CD 原始指令: jne 0x5F → {0x75, 0x5F}
static const BYTE CHOMPER_CD_ORIGINAL_BYTES[CHOMPER_CD_PATCH_SIZE] = { 0x75, 0x5F };

// 植物无敌原始值: -04 → {0xFC}
static const BYTE PLANT_INVINCIBLE_ORIGINAL_BYTES[PLANT_INVINCIBLE_SIZE] = { 0xFC };

// NOP 指令
static const BYTE NOP_BYTES[2] = { 0x90, 0x90 };
static const BYTE ZERO_BYTE[1] = { 0x00 };
