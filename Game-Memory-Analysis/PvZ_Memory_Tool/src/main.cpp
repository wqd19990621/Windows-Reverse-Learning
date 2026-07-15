#include "offsets.h"
#include "memory_helper.h"
#include "patch_engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 全局句柄（整个会话期间复用）
// ============================================================================
static HANDLE g_hProcess = NULL;

// ============================================================================
// 菜单显示
// ============================================================================
void ShowMenu()
{
    system("cls");
    printf("\n");
    printf("  ============================================\n");
    printf("      植物大战僵尸中文版 - 内存修改工具\n");
    printf("  ============================================\n\n");

    printf("  进程状态: %s\n\n",
        g_hProcess ? "[已连接]" : "[未连接] 请先使用 'connect' 连接游戏");

    printf("  --- 资源修改 ---\n");
    printf("  1. 无限阳光         (999999)\n");
    printf("  2. 无限金币         (999999)\n");
    printf("  3. 设置阳光数值\n");
    printf("  4. 设置金币数值\n\n");

    printf("  --- 代码修改 ---\n");
    printf("  5. 植物无冷却       (NOP 注入)\n");
    printf("  6. 大嘴花吞噬无冷却  (NOP 注入)\n");
    printf("  7. 植物无敌         (字节修改)\n\n");

    printf("  --- 高级功能 ---\n");
    printf("  8. 种植单颗樱桃炸弹  (指定坐标)\n");
    printf("  9. 全图樱桃炸弹     (5x8 全覆盖)\n\n");

    printf("  --- 系统 ---\n");
    printf("  connect  - 连接到游戏进程\n");
    printf("  status   - 查看当前修改状态\n");
    printf("  help     - 显示帮助\n");
    printf("  exit     - 退出程序\n\n");
}

// ============================================================================
// 连接到游戏进程
// ============================================================================
void DoConnect()
{
    if (g_hProcess)
    {
        printf("[i] 已连接到游戏进程，请先断开连接\n");
        return;
    }

    // 1. 提升权限
    MemoryEnableDebugPrivilege();

    // 2. 查找游戏窗口
    DWORD pid = MemoryFindProcessByWindow(PVZ_WINDOW_TITLE);
    if (!pid)
    {
        printf("[!] 请先启动植物大战僵尸中文版\n");
        return;
    }

    // 3. 打开进程
    g_hProcess = MemoryOpenProcess(pid);
}

// ============================================================================
// 断开连接
// ============================================================================
void DoDisconnect()
{
    if (g_hProcess)
    {
        CloseHandle(g_hProcess);
        g_hProcess = NULL;
        printf("[+] 已断开游戏进程连接\n");
    }
}

// ============================================================================
// 无限阳光
// ============================================================================
void DoUnlimitedSun()
{
    if (!g_hProcess) { printf("[!] 请先连接游戏进程\n"); return; }

    DWORD offsets[] = { SUNLIGHT_OFFSET_L1, SUNLIGHT_OFFSET_L2 };
    MemoryWriteMultiLevelPointer(g_hProcess, SUNLIGHT_BASE_PTR, offsets, 2, SUNLIGHT_MAX_VALUE);
    printf("[+] 阳光已设置为 %d\n", SUNLIGHT_MAX_VALUE);
}

// ============================================================================
// 无限金币
// ============================================================================
void DoUnlimitedGold()
{
    if (!g_hProcess) { printf("[!] 请先连接游戏进程\n"); return; }

    DWORD offsets[] = { GOLD_OFFSET_L1, GOLD_OFFSET_L2 };
    MemoryWriteMultiLevelPointer(g_hProcess, GOLD_BASE_PTR, offsets, 2, GOLD_MAX_VALUE);
    printf("[+] 金币已设置为 %d\n", GOLD_MAX_VALUE);
}

// ============================================================================
// 设置阳光
// ============================================================================
void DoSetSun()
{
    if (!g_hProcess) { printf("[!] 请先连接游戏进程\n"); return; }

    printf("请输入阳光数值 (默认 %d): ", SUNLIGHT_DEFAULT_VALUE);
    char line[32] = { 0 };
    fgets(line, sizeof(line), stdin);

    DWORD value = atoi(line);
    if (value <= 0) value = SUNLIGHT_DEFAULT_VALUE;

    DWORD offsets[] = { SUNLIGHT_OFFSET_L1, SUNLIGHT_OFFSET_L2 };
    MemoryWriteMultiLevelPointer(g_hProcess, SUNLIGHT_BASE_PTR, offsets, 2, value);
    printf("[+] 阳光已设置为 %lu\n", value);
}

// ============================================================================
// 设置金币
// ============================================================================
void DoSetGold()
{
    if (!g_hProcess) { printf("[!] 请先连接游戏进程\n"); return; }

    printf("请输入金币数值 (默认 %d): ", GOLD_DEFAULT_VALUE);
    char line[32] = { 0 };
    fgets(line, sizeof(line), stdin);

    DWORD value = atoi(line);
    if (value <= 0) value = GOLD_DEFAULT_VALUE;

    DWORD offsets[] = { GOLD_OFFSET_L1, GOLD_OFFSET_L2 };
    MemoryWriteMultiLevelPointer(g_hProcess, GOLD_BASE_PTR, offsets, 2, value);
    printf("[+] 金币已设置为 %lu\n", value);
}

// ============================================================================
// 切换 CD / 无敌
// ============================================================================
void DoTogglePlantCD()
{
    if (!g_hProcess) { printf("[!] 请先连接游戏进程\n"); return; }
    printf("输入 1 开启 / 0 关闭: ");
    char line[8] = { 0 };
    fgets(line, sizeof(line), stdin);
    PatchTogglePlantCD(g_hProcess, atoi(line) != 0);
}

void DoToggleChomperCD()
{
    if (!g_hProcess) { printf("[!] 请先连接游戏进程\n"); return; }
    printf("输入 1 开启 / 0 关闭: ");
    char line[8] = { 0 };
    fgets(line, sizeof(line), stdin);
    PatchToggleChomperCD(g_hProcess, atoi(line) != 0);
}

void DoToggleInvincible()
{
    if (!g_hProcess) { printf("[!] 请先连接游戏进程\n"); return; }
    printf("输入 1 开启 / 0 关闭: ");
    char line[8] = { 0 };
    fgets(line, sizeof(line), stdin);
    PatchTogglePlantInvincible(g_hProcess, atoi(line) != 0);
}

// ============================================================================
// 单颗樱桃炸弹
// ============================================================================
void DoCherryBomb()
{
    if (!g_hProcess) { printf("[!] 请先连接游戏进程\n"); return; }

    printf("请输入 X 坐标 (0~%d): ", GRID_MAX_X);
    char line[16] = { 0 };
    fgets(line, sizeof(line), stdin);
    int x = atoi(line);

    printf("请输入 Y 坐标 (0~%d): ", GRID_MAX_Y);
    fgets(line, sizeof(line), stdin);
    int y = atoi(line);

    PatchSpawnCherryBomb(g_hProcess, x, y);
}

// ============================================================================
// 全图樱桃炸弹
// ============================================================================
void DoFullMapCherryBomb()
{
    if (!g_hProcess) { printf("[!] 请先连接游戏进程\n"); return; }

    printf("[*] 开始全图种植樱桃炸弹 (5x8 = 40 颗)...\n");

    int count = 0;
    for (int y = 0; y <= GRID_MAX_Y; y++)
    {
        for (int x = 0; x <= GRID_MAX_X; x++)
        {
            if (PatchSpawnCherryBomb(g_hProcess, x, y))
                count++;
            Sleep(30); // 避免游戏响应不过来
        }
    }

    printf("[+] 全图樱桃炸弹种植完成！共种植 %d 颗\n", count);
}

// ============================================================================
// 状态查看
// ============================================================================
void DoStatus()
{
    printf("\n===== 当前状态 =====\n");
    printf("进程连接: %s\n", g_hProcess ? "已连接" : "未连接");

    if (g_hProcess)
    {
        // 读取当前阳光和金币值
        DWORD offsetsSun[] = { SUNLIGHT_OFFSET_L1, SUNLIGHT_OFFSET_L2 };
        DWORD offsetsGold[] = { GOLD_OFFSET_L1, GOLD_OFFSET_L2 };
        DWORD sun = 0, gold = 0;

        if (MemoryReadMultiLevelPointer(g_hProcess, SUNLIGHT_BASE_PTR, offsetsSun, 2, &sun))
            printf("当前阳光: %lu\n", sun);

        if (MemoryReadMultiLevelPointer(g_hProcess, GOLD_BASE_PTR, offsetsGold, 2, &gold))
            printf("当前金币: %lu\n", gold);
    }
    printf("=====================\n\n");
}

// ============================================================================
// 帮助
// ============================================================================
void DoHelp()
{
    printf("\n===== 使用说明 =====\n\n");
    printf("第一步: 启动植物大战僵尸中文版\n");
    printf("第二步: 运行本程序，输入 connect 连接游戏\n");
    printf("第三步: 输入数字 1-9 选择功能\n\n");
    printf("功能说明:\n");
    printf("  1/2 - 一键将阳光/金币设为 999999\n");
    printf("  3/4 - 自定义阳光/金币数值\n");
    printf("  5   - 移除植物种植冷却时间（NOP 注入代码段）\n");
    printf("  6   - 移除大嘴花吞噬冷却时间\n");
    printf("  7   - 植物不受僵尸伤害\n");
    printf("  8   - 在指定坐标召唤樱桃炸弹（输入 x y）\n");
    printf("  9   - 全图 5×8 格子全部种植樱桃炸弹\n\n");
    printf("注意: 5/6/7 修改了游戏代码段，重启游戏即恢复\n");
    printf("=====================\n\n");
}

// ============================================================================
// 主循环
// ============================================================================
int main()
{
    printf("\n");
    printf("  ╔══════════════════════════════════════════╗\n");
    printf("  ║  植物大战僵尸中文版 - 内存修改工具 v1.0  ║\n");
    printf("  ║  PvZ Win32 Memory Modifier               ║\n");
    printf("  ╚══════════════════════════════════════════╝\n");
    printf("\n  输入 'help' 查看使用说明\n\n");

    while (1)
    {
        printf("pvz> ");
        fflush(stdout);

        char line[256] = { 0 };
        if (!fgets(line, sizeof(line), stdin))
            break;

        // 去除尾部换行
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';
        if (len == 0)
            continue;

        // 命令路由
        if (strcmp(line, "connect") == 0)
            DoConnect();
        else if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0)
        {
            DoDisconnect();
            printf("再见！\n");
            break;
        }
        else if (strcmp(line, "help") == 0)
            DoHelp();
        else if (strcmp(line, "status") == 0)
            DoStatus();
        else if (strcmp(line, "menu") == 0)
            ShowMenu();
        else if (strcmp(line, "1") == 0)
            DoUnlimitedSun();
        else if (strcmp(line, "2") == 0)
            DoUnlimitedGold();
        else if (strcmp(line, "3") == 0)
            DoSetSun();
        else if (strcmp(line, "4") == 0)
            DoSetGold();
        else if (strcmp(line, "5") == 0)
            DoTogglePlantCD();
        else if (strcmp(line, "6") == 0)
            DoToggleChomperCD();
        else if (strcmp(line, "7") == 0)
            DoToggleInvincible();
        else if (strcmp(line, "8") == 0)
            DoCherryBomb();
        else if (strcmp(line, "9") == 0)
            DoFullMapCherryBomb();
        else if (strcmp(line, "") == 0)
            continue;
        else
            printf("[!] 未知命令: '%s', 输入 'help' 查看帮助\n", line);

        printf("\n");
    }

    return 0;
}
