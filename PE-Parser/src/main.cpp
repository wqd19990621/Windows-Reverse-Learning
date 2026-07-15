#include "pe_parser.h"
#include "console.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================================
// 命令处理器类型和命令表
// ============================================================================
typedef void (*CmdHandler)(const PE_CONTEXT* ctx, CONST CHAR* param);

typedef struct {
    CONST CHAR* cmd;
    CONST CHAR* desc;      // 简短描述，用于内联帮助
    CmdHandler  handler;
} CmdEntry;

// 命令处理器的前向声明
static void DoLoad(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoInfo(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoDos(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoNt(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoSection(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoImport(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoExport(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoGetProcName(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoGetProcIndex(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoRelocation(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoRvaToFoa(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoFoaToRva(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoClear(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoHelp(const PE_CONTEXT* ctx, CONST CHAR* param);
static void DoExit(const PE_CONTEXT* ctx, CONST CHAR* param);

// ============================================================================
// 命令表 - 将命令字符串映射到处理器
// ============================================================================
static const CmdEntry kCmdTable[] = {
    { "load",         "加载 PE 文件",                           DoLoad          },
    { "info",         "显示 PE 基本信息",                       DoInfo          },
    { "dos",          "显示 DOS 头",                            DoDos           },
    { "nt",           "显示 NT 头",                             DoNt            },
    { "section",      "显示节头",                               DoSection       },
    { "import",       "显示导入表",                             DoImport        },
    { "export",       "显示导出表",                             DoExport        },
    { "getprocname",  "按名称查找导出函数",                     DoGetProcName   },
    { "getprocindex", "按序号查找导出函数",                     DoGetProcIndex  },
    { "relocation",   "显示基址重定位表",                       DoRelocation    },
    { "rva",          "将 RVA 转换为 FOA",                      DoRvaToFoa      },
    { "foa",          "将 FOA 转换为 RVA",                      DoFoaToRva      },
    { "clear",        "清屏",                                   DoClear         },
    { "help",         "显示帮助",                               DoHelp          },
    { "exit",         "退出程序",                               DoExit          },
    { NULL,           NULL,                                      NULL            }
};

// ============================================================================
// 按名称查找命令处理器
// ============================================================================
static CmdHandler FindCmdHandler(CONST CHAR* cmd)
{
    for (const CmdEntry* entry = kCmdTable; entry->cmd != NULL; entry++)
    {
        if (strcmp(cmd, entry->cmd) == 0)
            return entry->handler;
    }
    return NULL;
}

// ============================================================================
// 从参数字符串解析十六进制地址（支持 "1000" 或 "0x1000"）
// ============================================================================
static BOOL ParseHexAddress(CONST CHAR* param, DWORD* outAddr)
{
    if (param == NULL || *param == '\0') return FALSE;

    // 先尝试 "0x..." 格式，再尝试纯十六进制
    if (sscanf(param, "0x%x", outAddr) == 1)
        return TRUE;
    if (sscanf(param, "%x", outAddr) == 1)
        return TRUE;

    return FALSE;
}

// ============================================================================
// 处理单条命令
// ============================================================================
static void ProcessCommand(PE_CONTEXT* ctx)
{
    CHAR cmdLine[256] = { 0 };
    CHAR cmd[32] = { 0 };
    CHAR param[256] = { 0 };

    // 读取输入
    if (fgets(cmdLine, sizeof(cmdLine), stdin) == NULL)
        return;

    // 去除尾部换行符
    size_t len = strlen(cmdLine);
    if (len > 0 && cmdLine[len - 1] == '\n')
        cmdLine[len - 1] = '\0';

    // 解析命令和参数
    // %31s 读取第一个词（命令），%255[^\n] 读取剩余部分作为参数
    int parsed = sscanf(cmdLine, "%31s %255[^\n]", cmd, param);

    if (parsed < 1 || cmd[0] == '\0')
        return;

    // 查找并执行处理器
    CmdHandler handler = FindCmdHandler(cmd);
    if (handler)
    {
        handler(ctx, (parsed >= 2) ? param : "");
    }
    else
    {
        PRINT_ERROR("\n错误 -> 未知命令: '%s'\r\n", cmd);
        PRINT_ERROR("输入 'help' 查看可用命令列表。\r\n");
    }
}

// ============================================================================
// 命令处理器
// ============================================================================

static void DoLoad(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    // LoadPeFile 需要一个非常量上下文来修改
    if (param == NULL || *param == '\0')
    {
        PRINT_ERROR("错误 -> 请指定 PE 文件路径。\r\n");
        PRINT_ERROR("示例: load C:\\test.exe\r\n");
        return;
    }
    LoadPeFile((PE_CONTEXT*)ctx, param);
}

static void DoInfo(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)param;
    ShowPeInfo(ctx);
}

static void DoDos(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)param;
    ShowDosHeader(ctx);
}

static void DoNt(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)param;
    ShowNtHeader(ctx);
}

static void DoSection(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)param;
    ShowSections(ctx);
}

static void DoImport(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)param;
    ShowImports(ctx);
}

static void DoExport(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)param;
    ShowExports(ctx);
}

static void DoGetProcName(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    if (!IsPeLoaded(ctx)) return;

    if (param == NULL || *param == '\0')
    {
        PRINT_ERROR("错误 -> 请指定函数名称。\r\n");
        PRINT_ERROR("示例: getprocname CreateFileA\r\n");
        return;
    }

    DWORD dwRva = FindExportByName(ctx, param);
    if (dwRva == 0)
    {
        PRINT_ERROR("错误 -> 未找到函数: '%s'\r\n", param);
        return;
    }

    DWORD dwFoa = RvaToFoa(ctx, dwRva);

    PRINT_INFO("\n导出函数已找到：\r\n");
    PRINT_INFO("  名称:    %s\r\n", param);
    PRINT_INFO("  RVA:     0x%08X\r\n", dwRva);
    PRINT_INFO("  FOA:     0x%08X\r\n", dwFoa);

    // 显示所属节
    int secIdx = FindSectionByRva(ctx, dwRva);
    if (secIdx >= 0)
    {
        CHAR secName[9] = { 0 };
        GetSectionName(&ctx->pSectionHeader[secIdx], secName);
        PRINT_INFO("  所在节:  %s\r\n", secName);

        // 检查是否为转发导出
        PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, IMAGE_DIRECTORY_ENTRY_EXPORT);
        if (pDir && dwRva >= pDir->VirtualAddress && dwRva < pDir->VirtualAddress + pDir->Size)
        {
            DWORD dwForwardFoa = RvaToFoa(ctx, dwRva);
            if (dwForwardFoa != 0)
            {
                CONST CHAR* szForwarder = (CONST CHAR*)(ctx->lpFileBuffer + dwForwardFoa);
                PRINT_INFO("  注意:  这是一个转发导出 -> %s\r\n", szForwarder);
            }
        }
    }
    PRINT_INFO("\r\n");
}

static void DoGetProcIndex(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    if (!IsPeLoaded(ctx)) return;

    if (param == NULL || *param == '\0')
    {
        PRINT_ERROR("错误 -> 请指定函数序号。\r\n");
        PRINT_ERROR("示例: getprocindex 100\r\n");
        return;
    }

    DWORD dwOrdinal = 0;
    if (sscanf(param, "%u", &dwOrdinal) != 1)
    {
        PRINT_ERROR("错误 -> 无效的序号格式。\r\n");
        PRINT_ERROR("示例: getprocindex 100\r\n");
        return;
    }

    DWORD dwRva = FindExportByOrdinal(ctx, dwOrdinal);
    if (dwRva == 0)
    {
        PRINT_ERROR("错误 -> 序号 %u 未找到函数\r\n", dwOrdinal);
        return;
    }

    DWORD dwFoa = RvaToFoa(ctx, dwRva);

    // 尝试获取函数名称
    CONST CHAR* szFuncName = NULL;
    PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, IMAGE_DIRECTORY_ENTRY_EXPORT);
    if (pDir && pDir->VirtualAddress != 0)
    {
        DWORD dwExportFoa = RvaToFoa(ctx, pDir->VirtualAddress);
        PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)(ctx->lpFileBuffer + dwExportFoa);
        PDWORD pAddrOfNames = (PDWORD)(ctx->lpFileBuffer + RvaToFoa(ctx, pExport->AddressOfNames));
        PWORD  pAddrOfOrdinals = (PWORD)(ctx->lpFileBuffer + RvaToFoa(ctx, pExport->AddressOfNameOrdinals));

        DWORD dwIndex = dwOrdinal - pExport->Base;
        for (DWORD i = 0; i < pExport->NumberOfNames; i++)
        {
            if (pAddrOfOrdinals[i] == dwIndex)
            {
                DWORD dwNameFoa = RvaToFoa(ctx, pAddrOfNames[i]);
                if (dwNameFoa != 0)
                    szFuncName = (CONST CHAR*)(ctx->lpFileBuffer + dwNameFoa);
                break;
            }
        }
    }

    PRINT_INFO("\r\n导出函数已找到：\r\n");
    if (szFuncName)
        PRINT_INFO("  名称:    %s\r\n", szFuncName);
    PRINT_INFO("  序号:    %u\r\n", dwOrdinal);
    PRINT_INFO("  RVA:     0x%08X\r\n", dwRva);
    PRINT_INFO("  FOA:     0x%08X\r\n", dwFoa);

    int secIdx = FindSectionByRva(ctx, dwRva);
    if (secIdx >= 0)
    {
        CHAR secName[9] = { 0 };
        GetSectionName(&ctx->pSectionHeader[secIdx], secName);
        PRINT_INFO("  所在节:  %s\r\n", secName);
    }
    PRINT_INFO("\r\n");
}

static void DoRelocation(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)param;
    ShowRelocations(ctx);
}

static void DoRvaToFoa(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    if (!IsPeLoaded(ctx)) return;

    if (param == NULL || *param == '\0')
    {
        PRINT_ERROR("错误 -> 请指定 RVA 地址。\r\n");
        PRINT_ERROR("示例: rva 1000 或 rva 0x1000\r\n");
        return;
    }

    DWORD dwRva = 0;
    if (!ParseHexAddress(param, &dwRva))
    {
        PRINT_ERROR("错误 -> 无效的地址格式。\r\n");
        PRINT_ERROR("示例: rva 1000 或 rva 0x1000\r\n");
        return;
    }

    DWORD dwFoa = RvaToFoa(ctx, dwRva);
    if (dwFoa == 0)
    {
        PRINT_ERROR("错误 -> RVA 到 FOA 转换失败 (RVA: 0x%08X)\r\n", dwRva);
        return;
    }

    PRINT_ERROR("\n转换结果：\r\n");
    PRINT_ERROR("  RVA: 0x%08X  ->  FOA: 0x%08X\r\n", dwRva, dwFoa);

    int secIdx = FindSectionByRva(ctx, dwRva);
    if (secIdx >= 0)
    {
        CHAR secName[9] = { 0 };
        GetSectionName(&ctx->pSectionHeader[secIdx], secName);
        PRINT_INFO("  所在节: %s\r\n", secName);
        PRINT_INFO("  节的 RVA 范围:  0x%08X - 0x%08X\r\n",
            ctx->pSectionHeader[secIdx].VirtualAddress,
            ctx->pSectionHeader[secIdx].VirtualAddress + ctx->pSectionHeader[secIdx].Misc.VirtualSize);
        PRINT_INFO("  节的 FOA 范围:  0x%08X - 0x%08X\r\n",
            ctx->pSectionHeader[secIdx].PointerToRawData,
            ctx->pSectionHeader[secIdx].PointerToRawData + ctx->pSectionHeader[secIdx].SizeOfRawData);
    }
    PRINT_INFO("\r\n");
}

static void DoFoaToRva(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    if (!IsPeLoaded(ctx)) return;

    if (param == NULL || *param == '\0')
    {
        PRINT_ERROR("错误 -> 请指定 FOA 地址。\r\n");
        PRINT_ERROR("示例: foa 1000 或 foa 0x1000\r\n");
        return;
    }

    DWORD dwFoa = 0;
    if (!ParseHexAddress(param, &dwFoa))
    {
        PRINT_ERROR("错误 -> 无效的地址格式。\r\n");
        PRINT_ERROR("示例: foa 1000 或 foa 0x1000\r\n");
        return;
    }

    DWORD dwRva = FoaToRva(ctx, dwFoa);
    if (dwRva == 0)
    {
        PRINT_ERROR("错误 -> FOA 到 RVA 转换失败 (FOA: 0x%08X)\r\n", dwFoa);
        return;
    }

    PRINT_ERROR("\n转换结果：\r\n");
    PRINT_ERROR("  FOA: 0x%08X  ->  RVA: 0x%08X\r\n", dwFoa, dwRva);

    int secIdx = FindSectionByRva(ctx, dwRva);
    if (secIdx >= 0)
    {
        CHAR secName[9] = { 0 };
        GetSectionName(&ctx->pSectionHeader[secIdx], secName);
        PRINT_INFO("  所在节: %s\r\n", secName);
    }
    PRINT_INFO("\r\n");
}

static void DoClear(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)ctx;
    (void)param;
    ClearScreen();
}

static void DoHelp(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)ctx;
    (void)param;
    ShowHelp();
}

static void DoExit(const PE_CONTEXT* ctx, CONST CHAR* param)
{
    (void)param;
    PRINT_INFO("正在退出...\r\n");
    FreePeFile((PE_CONTEXT*)ctx);
    exit(0);
}

// ============================================================================
// 主入口点
// ============================================================================
int main()
{
    PE_CONTEXT ctx = { 0 };

    while (1)
    {
        ShowMenu();
        ProcessCommand(&ctx);

        printf("\n");
        system("pause");
    }

    return 0;
}
