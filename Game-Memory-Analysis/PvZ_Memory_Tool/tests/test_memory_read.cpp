#include "../src/memory_helper.h"
#include <stdio.h>

// ============================================================================
// 内存读取单元测试
//
// 编译:
//   cl /EHsc /I../src test_memory_read.cpp ../src/memory_helper.cpp
//
// 注意: 此测试需要植物大战僵尸中文版正在运行才能通过
// ============================================================================

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (cond) { \
        printf("  [PASS] %s\n", msg); \
        g_testsPassed++; \
    } else { \
        printf("  [FAIL] %s\n", msg); \
        g_testsFailed++; \
    } \
} while(0)

// ============================================================================
// 测试1: 权限提升
// ============================================================================
void Test_EnableDebugPrivilege()
{
    printf("\n--- 测试: 调试权限提升 ---\n");
    BOOL result = MemoryEnableDebugPrivilege();
    TEST_ASSERT(result, "调试权限提升成功");
}

// ============================================================================
// 测试2: 查找进程
// ============================================================================
void Test_FindProcess()
{
    printf("\n--- 测试: 查找游戏进程 ---\n");
    DWORD pid = MemoryFindProcessByWindow(L"植物大战僵尸中文版");
    TEST_ASSERT(pid != 0, "找到游戏进程 PID");
}

// ============================================================================
// 测试3: 打开进程
// ============================================================================
void Test_OpenProcess()
{
    printf("\n--- 测试: 打开进程句柄 ---\n");
    DWORD pid = MemoryFindProcessByWindow(L"植物大战僵尸中文版");
    if (!pid)
    {
        printf("  [SKIP] 游戏未运行\n");
        return;
    }

    HANDLE hProcess = MemoryOpenProcess(pid);
    TEST_ASSERT(hProcess != NULL, "进程句柄获取成功");

    if (hProcess)
        CloseHandle(hProcess);
}

// ============================================================================
// 测试4: 内存读取 — 读取阳光值
// ============================================================================
void Test_ReadSunlight()
{
    printf("\n--- 测试: 读取阳光值 ---\n");

    DWORD pid = MemoryFindProcessByWindow(L"植物大战僵尸中文版");
    if (!pid)
    {
        printf("  [SKIP] 游戏未运行\n");
        return;
    }

    HANDLE hProcess = MemoryOpenProcess(pid);
    if (!hProcess)
    {
        printf("  [SKIP] 无法打开进程\n");
        return;
    }

    // 阳光: [[0x6A9EC0 + 0x768] + 0x5560]
    DWORD offsets[] = { 0x768, 0x5560 };
    DWORD sunlight = 0;
    BOOL result = MemoryReadMultiLevelPointer(hProcess, 0x6A9EC0, offsets, 2, &sunlight);
    TEST_ASSERT(result, "多级指针读取成功");
    printf("    当前阳光值: %lu\n", sunlight);

    // 基本合理性检查
    TEST_ASSERT(sunlight < 100000, "阳光值在合理范围内 (<100000)");

    CloseHandle(hProcess);
}

// ============================================================================
// 测试5: 内存读取 — 基本接口
// ============================================================================
void Test_BasicMemoryRead()
{
    printf("\n--- 测试: 基本内存读取 ---\n");

    DWORD pid = MemoryFindProcessByWindow(L"植物大战僵尸中文版");
    if (!pid)
    {
        printf("  [SKIP] 游戏未运行\n");
        return;
    }

    HANDLE hProcess = MemoryOpenProcess(pid);
    if (!hProcess)
    {
        printf("  [SKIP] 无法打开进程\n");
        return;
    }

    // 测试读取 DOS 头 (MZ 签名)
    WORD dosSignature = 0;
    BOOL result = MemoryRead(hProcess, (LPCVOID)0x00400000, &dosSignature, sizeof(WORD));
    TEST_ASSERT(result, "读取 PE 基址 DOS 签名");
    TEST_ASSERT(dosSignature == 0x5A4D, "DOS 签名验证 (MZ = 0x5A4D)");
    printf("    实际值: 0x%04X (期望: 0x5A4D)\n", dosSignature);

    CloseHandle(hProcess);
}

// ============================================================================
// 运行所有测试
// ============================================================================
int main()
{
    printf("==========================================\n");
    printf("  PvZ Memory Tool - 单元测试\n");
    printf("==========================================\n");
    printf("注意: 部分测试需要游戏正在运行\n");

    Test_EnableDebugPrivilege();
    Test_FindProcess();
    Test_OpenProcess();
    Test_ReadSunlight();
    Test_BasicMemoryRead();

    printf("\n==========================================\n");
    printf("  测试结果: %d 通过, %d 失败, %d 跳过\n",
        g_testsPassed, g_testsFailed, 5 - g_testsPassed - g_testsFailed);
    printf("==========================================\n");

    return g_testsFailed > 0 ? 1 : 0;
}
