#include "console.h"
#include "pe_common.h"

#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// ShowMenu - 显示主交互菜单
// ============================================================================
void ShowMenu()
{
    system("cls");

    PRINT_TITLE("\n=== PE 文件分析工具 ===\n\n");

    PRINT_MENU("命令：\r\n");
    PRINT_MENU("  load <文件路径>  - 加载 PE 文件\r\n");
    PRINT_MENU("  info             - 显示 PE 基本信息\r\n");
    PRINT_MENU("  dos              - 显示 DOS 头\r\n");
    PRINT_MENU("  nt               - 显示 NT 头（文件头 + 可选头）\r\n");
    PRINT_MENU("  section          - 显示节头\r\n");
    PRINT_MENU("  import           - 显示导入表\r\n");
    PRINT_MENU("  export           - 显示导出表\r\n");
    PRINT_MENU("  getprocname <名> - 按名称查找导出函数\r\n");
    PRINT_MENU("  getprocindex <号>- 按序号查找导出函数\r\n");
    PRINT_MENU("  relocation       - 显示基址重定位表\r\n");
    PRINT_MENU("  rva <地址>       - 将 RVA 转换为 FOA\r\n");
    PRINT_MENU("  foa <地址>       - 将 FOA 转换为 RVA\r\n");
    PRINT_MENU("  clear            - 清屏\r\n");
    PRINT_MENU("  help             - 显示帮助\r\n");
    PRINT_MENU("  exit             - 退出程序\r\n\n");

    PRINT_INPUT("请输入命令 > ");
}

// ============================================================================
// ShowHelp - 显示详细命令帮助
// ============================================================================
void ShowHelp()
{
    PRINT_TITLE("\n\n==== PE 文件分析工具 - 帮助 ====\n\n");
    PRINT_INFO("命令列表及说明：\r\n");
    PRINT_INFO("----------------------------------------------------\r\n");
    PRINT_INFO("  load <路径>          加载 PE 文件\r\n");
    PRINT_INFO("                       示例: load C:\\test.exe\r\n");
    PRINT_INFO("  info                 显示 PE 基本信息（格式、CPU、入口点等）\r\n");
    PRINT_INFO("  dos                  详细显示 DOS 头字段\r\n");
    PRINT_INFO("  nt                   显示 NT 头（FileHeader + OptionalHeader）\r\n");
    PRINT_INFO("  section              显示所有节头及属性\r\n");
    PRINT_INFO("  import               显示导入表（DLL 及导入函数）\r\n");
    PRINT_INFO("  export               显示导出表（导出函数）\r\n");
    PRINT_INFO("  getprocname <名称>   按名称查找导出函数 RVA\r\n");
    PRINT_INFO("                       示例: getprocname CreateFileA\r\n");
    PRINT_INFO("  getprocindex <序号>  按序号查找导出函数 RVA\r\n");
    PRINT_INFO("                       示例: getprocindex 100\r\n");
    PRINT_INFO("  relocation           显示基址重定位表\r\n");
    PRINT_INFO("  rva <地址>           将 RVA 转换为 FOA\r\n");
    PRINT_INFO("                       示例: rva 1000 或 rva 0x1000\r\n");
    PRINT_INFO("  foa <地址>           将 FOA 转换为 RVA\r\n");
    PRINT_INFO("                       示例: foa 1000 或 foa 0x1000\r\n");
    PRINT_INFO("  clear                清屏\r\n");
    PRINT_INFO("  help                 显示此帮助信息\r\n");
    PRINT_INFO("  exit                 退出程序\r\n");
    PRINT_INFO("----------------------------------------------------\r\n");
    PRINT_INFO("提示：\r\n");
    PRINT_INFO("  1. 请始终先使用 'load' 命令加载 PE 文件\r\n");
    PRINT_INFO("  2. 地址可以是十进制或十六进制（加 0x 前缀）\r\n");
    PRINT_INFO("  3. 同时支持 32 位 (PE32) 和 64 位 (PE32+) PE 文件\r\n");
    PRINT_INFO("  4. 导出序号从 Base 值开始，而非从零开始\r\n");
    PRINT_INFO("\r\n");
}

// ============================================================================
// ClearScreen - 清除控制台
// ============================================================================
void ClearScreen()
{
    system("cls");
    PRINT_INFO("屏幕已清除。\r\n");
}
