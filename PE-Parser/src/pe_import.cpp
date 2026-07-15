#include "pe_parser.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// 打印某个导入描述符的 INT/IAT 条目
// ============================================================================
static void PrintThunkEntries(const PE_CONTEXT* ctx, PIMAGE_IMPORT_DESCRIPTOR pDesc)
{
    // 如果有 OriginalFirstThunk (INT) 则使用它，否则使用 FirstThunk (IAT)
    DWORD dwThunkRva = (pDesc->OriginalFirstThunk != 0)
        ? pDesc->OriginalFirstThunk
        : pDesc->FirstThunk;

    if (dwThunkRva == 0) return;

    DWORD dwThunkFoa = RvaToFoa(ctx, dwThunkRva);
    if (dwThunkFoa == 0)
    {
        PRINT_ERROR("  错误 -> INT/IAT RVA 到 FOA 转换失败\r\n");
        return;
    }

    PDWORD pThunkData = (PDWORD)(ctx->lpFileBuffer + dwThunkFoa);

    PRINT_TITLE("\n  [导入条目]\r\n");

    for (DWORD j = 0; ; j++)
    {
        DWORD dwThunkValue = pThunkData[j];
        if (dwThunkValue == 0)
            break;  // 以空结尾的数组

        // 对于 64 位 PE，第 63 位表示按序号导入
        // 对于 32 位 PE，第 31 位表示按序号导入
        BOOL bIsOrdinal = (ctx->bIs64Bit && (dwThunkValue & 0x8000000000000000ULL))
            || (!ctx->bIs64Bit && (dwThunkValue & 0x80000000));

        if (bIsOrdinal)
        {
            // 按序号导入
            DWORD dwOrdinal;
            if (ctx->bIs64Bit)
                dwOrdinal = (DWORD)(dwThunkValue & 0x7FFFFFFFFFFFFFFFULL);
            else
                dwOrdinal = dwThunkValue & 0x7FFFFFFF;

            PRINT_INFO("    [%3u] 0x%08X -> 序号: %u (0x%04X)\r\n",
                j, dwThunkValue, dwOrdinal, dwOrdinal);
        }
        else
        {
            // 按名称导入
            DWORD dwNameFoa = RvaToFoa(ctx, dwThunkValue);
            if (dwNameFoa != 0)
            {
                PIMAGE_IMPORT_BY_NAME pByName = (PIMAGE_IMPORT_BY_NAME)(ctx->lpFileBuffer + dwNameFoa);
                PRINT_INFO("    [%3u] 0x%08X -> 名称: %s (提示: 0x%04X)\r\n",
                    j, dwThunkValue, pByName->Name, pByName->Hint);
            }
            else
            {
                PRINT_INFO("    [%3u] 0x%08X -> (名称 RVA 转换失败)\r\n", j, dwThunkValue);
            }
        }
    }
}

// ============================================================================
// ShowImports - 显示导入表
//
// 修复: 使用 GetDataDirectory() 实现 32/64 位安全的 DataDirectory 访问。
// 之前使用 g_pNtHeaders->OptionalHeader.DataDirectory[]，
// 当编译为 64 位但文件为 32 位时会读取错误的偏移量。
// ============================================================================
void ShowImports(const PE_CONTEXT* ctx)
{
    if (!IsPeLoaded(ctx)) return;

    // 32/64 位安全访问导入数据目录
    PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, IMAGE_DIRECTORY_ENTRY_IMPORT);
    if (pDir == NULL || pDir->VirtualAddress == 0 || pDir->Size == 0)
    {
        PRINT_ERROR("错误 -> 此 PE 文件没有导入表\r\n");
        return;
    }

    // 定位导入描述符表
    DWORD dwImportFoa = RvaToFoa(ctx, pDir->VirtualAddress);
    if (dwImportFoa == 0)
    {
        PRINT_ERROR("错误 -> 导入表 RVA 到 FOA 转换失败\r\n");
        return;
    }

    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(ctx->lpFileBuffer + dwImportFoa);

    PRINT_TITLE("\n\n==== 导入目录信息 ====\n\n");

    for (DWORD i = 0; pImportDesc[i].Name != 0; i++)
    {
        // 获取 DLL 名称
        DWORD dwDllNameFoa = RvaToFoa(ctx, pImportDesc[i].Name);
        if (dwDllNameFoa == 0) continue;

        CONST CHAR* szDllName = (CONST CHAR*)(ctx->lpFileBuffer + dwDllNameFoa);

        PRINT_ERROR("\n[%u] %s\r\n", i + 1, szDllName);
        PRINT_INFO("----------------------------------------\r\n");
        PRINT_ERROR("  0000h    OriginalFirstThunk  ->  0x%08X        // INT RVA\r\n", pImportDesc[i].OriginalFirstThunk);
        PRINT_INFO("  0004h    TimeDateStamp       ->  0x%08X        // 时间戳\r\n", pImportDesc[i].TimeDateStamp);
        PRINT_INFO("  0008h    ForwarderChain      ->  0x%08X        // 转发链\r\n", pImportDesc[i].ForwarderChain);
        PRINT_ERROR("  000Ch    Name                ->  0x%08X        // DLL 名称 RVA\r\n", pImportDesc[i].Name);
        PRINT_ERROR("  0010h    FirstThunk          ->  0x%08X        // IAT RVA\r\n", pImportDesc[i].FirstThunk);

        // 解析 INT/IAT 条目
        PrintThunkEntries(ctx, &pImportDesc[i]);

        PRINT_INFO("----------------------------------------\r\n");
    }
}
