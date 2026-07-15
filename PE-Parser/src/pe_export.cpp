#include "pe_parser.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// ShowExports - 显示导出表及函数列表
//
// 修复: 使用 GetDataDirectory() 实现 32/64 位安全访问。
// ============================================================================
void ShowExports(const PE_CONTEXT* ctx)
{
    if (!IsPeLoaded(ctx)) return;

    // 32/64 位安全访问导出数据目录
    PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, IMAGE_DIRECTORY_ENTRY_EXPORT);
    if (pDir == NULL || pDir->VirtualAddress == 0 || pDir->Size == 0)
    {
        PRINT_ERROR("错误 -> 此 PE 文件没有导出表\r\n");
        return;
    }

    // 定位导出目录
    DWORD dwExportFoa = RvaToFoa(ctx, pDir->VirtualAddress);
    if (dwExportFoa == 0)
    {
        PRINT_ERROR("错误 -> 导出表 RVA 到 FOA 转换失败\r\n");
        return;
    }

    PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)(ctx->lpFileBuffer + dwExportFoa);

    PRINT_TITLE("\n\n==== 导出目录信息 ====\n\n");

    // --- 导出目录表 ---
    PRINT_INFO("  0000h    Characteristics         ->  0x%08X        // 保留（未使用）\r\n", pExport->Characteristics);
    PRINT_INFO("  0004h    TimeDateStamp           ->  0x%08X        // 导出时间戳\r\n", pExport->TimeDateStamp);
    PRINT_INFO("  0008h    MajorVersion            ->  0x%04X           // 主版本（未使用）\r\n", pExport->MajorVersion);
    PRINT_INFO("  000Ah    MinorVersion            ->  0x%04X           // 副版本（未使用）\r\n", pExport->MinorVersion);

    // DLL 名称
    PRINT_ERROR("  000Ch    Name                    ->  0x%08X        // DLL 名称 RVA\r\n", pExport->Name);
    DWORD dwDllNameFoa = RvaToFoa(ctx, pExport->Name);
    if (dwDllNameFoa != 0)
    {
        CONST CHAR* szDllName = (CONST CHAR*)(ctx->lpFileBuffer + dwDllNameFoa);
        PRINT_ERROR("  000Ch    Name                    ->  %s              // DLL 名称\r\n", szDllName);
    }

    PRINT_ERROR("  0010h    Base                    ->  0x%08X        // 序号基值\r\n", pExport->Base);
    PRINT_ERROR("  0014h    NumberOfFunctions       ->  0x%08X        // 导出函数总数\r\n", pExport->NumberOfFunctions);
    PRINT_ERROR("  0018h    NumberOfNames           ->  0x%08X        // 命名导出数量\r\n", pExport->NumberOfNames);
    PRINT_ERROR("  001Ch    AddressOfFunctions      ->  0x%08X        // 函数地址表 RVA\r\n", pExport->AddressOfFunctions);
    PRINT_ERROR("  0020h    AddressOfNames          ->  0x%08X        // 函数名称表 RVA\r\n", pExport->AddressOfNames);
    PRINT_ERROR("  0024h    AddressOfNameOrdinals   ->  0x%08X        // 序号表 RVA\r\n\n", pExport->AddressOfNameOrdinals);

    // --- 导出函数表 ---
    PRINT_TITLE("\n\n==== 导出函数表 ====\n\n");
    PRINT_INFO("序号\t\tRVA\t\t名称\r\n");
    PRINT_INFO("-------------------------------------------------------------------------------------------------------\r\n");

    // 定位三个并行数组
    PDWORD pAddrOfFunctions = (PDWORD)(ctx->lpFileBuffer + RvaToFoa(ctx, pExport->AddressOfFunctions));
    PDWORD pAddrOfNames     = (PDWORD)(ctx->lpFileBuffer + RvaToFoa(ctx, pExport->AddressOfNames));
    PWORD  pAddrOfOrdinals  = (PWORD)(ctx->lpFileBuffer + RvaToFoa(ctx, pExport->AddressOfNameOrdinals));

    for (DWORD i = 0; i < pExport->NumberOfFunctions; i++)
    {
        // 跳过空槽位（转发导出可能留下空白）
        if (pAddrOfFunctions[i] == 0)
            continue;

        // 尝试查找此函数的名称
        CONST CHAR* szFuncName = NULL;
        for (DWORD j = 0; j < pExport->NumberOfNames; j++)
        {
            if (pAddrOfOrdinals[j] == i)
            {
                DWORD dwNameFoa = RvaToFoa(ctx, pAddrOfNames[j]);
                if (dwNameFoa != 0)
                {
                    szFuncName = (CONST CHAR*)(ctx->lpFileBuffer + dwNameFoa);
                }
                break;
            }
        }

        // 检查是否为转发导出
        DWORD dwFuncRva = pAddrOfFunctions[i];
        BOOL bIsForwarded = FALSE;
        CONST CHAR* szForwarder = NULL;

        // 转发导出的 RVA 指向导出节的数据范围之内
        if (dwFuncRva >= pDir->VirtualAddress &&
            dwFuncRva < pDir->VirtualAddress + pDir->Size)
        {
            DWORD dwForwardFoa = RvaToFoa(ctx, dwFuncRva);
            if (dwForwardFoa != 0)
            {
                bIsForwarded = TRUE;
                szForwarder = (CONST CHAR*)(ctx->lpFileBuffer + dwForwardFoa);
            }
        }

        DWORD dwOrdinal = i + pExport->Base;

        if (szFuncName)
        {
            PRINT_INFO("%u\t\t0x%08X\t%s\r\n", dwOrdinal, dwFuncRva, szFuncName);
        }
        else if (bIsForwarded)
        {
            PRINT_INFO("%u\t\t0x%08X\t<转发至: %s>\r\n", dwOrdinal, dwFuncRva, szForwarder);
        }
        else
        {
            PRINT_INFO("%u\t\t0x%08X\t<无名称>\r\n", dwOrdinal, dwFuncRva);
        }
    }
    PRINT_INFO("-------------------------------------------------------------------------------------------------------\r\n");
}

// ============================================================================
// FindExportByName - 按名称查找导出函数
// ============================================================================
DWORD FindExportByName(const PE_CONTEXT* ctx, CONST CHAR* funcName)
{
    if (ctx == NULL || funcName == NULL || *funcName == '\0') return 0;

    PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, IMAGE_DIRECTORY_ENTRY_EXPORT);
    if (pDir == NULL || pDir->VirtualAddress == 0 || pDir->Size == 0) return 0;

    DWORD dwExportFoa = RvaToFoa(ctx, pDir->VirtualAddress);
    if (dwExportFoa == 0) return 0;

    PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)(ctx->lpFileBuffer + dwExportFoa);

    PDWORD pAddrOfFunctions = (PDWORD)(ctx->lpFileBuffer + RvaToFoa(ctx, pExport->AddressOfFunctions));
    PDWORD pAddrOfNames     = (PDWORD)(ctx->lpFileBuffer + RvaToFoa(ctx, pExport->AddressOfNames));
    PWORD  pAddrOfOrdinals  = (PWORD)(ctx->lpFileBuffer + RvaToFoa(ctx, pExport->AddressOfNameOrdinals));

    for (DWORD i = 0; i < pExport->NumberOfNames; i++)
    {
        DWORD dwNameFoa = RvaToFoa(ctx, pAddrOfNames[i]);
        if (dwNameFoa == 0) continue;

        CONST CHAR* szName = (CONST CHAR*)(ctx->lpFileBuffer + dwNameFoa);
        if (strcmp(funcName, szName) == 0)
        {
            WORD ordinalIndex = pAddrOfOrdinals[i];
            return pAddrOfFunctions[ordinalIndex];
        }
    }

    return 0;
}

// ============================================================================
// FindExportByOrdinal - 按序号查找导出函数
// ============================================================================
DWORD FindExportByOrdinal(const PE_CONTEXT* ctx, DWORD dwOrdinal)
{
    if (ctx == NULL) return 0;

    PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, IMAGE_DIRECTORY_ENTRY_EXPORT);
    if (pDir == NULL || pDir->VirtualAddress == 0 || pDir->Size == 0) return 0;

    DWORD dwExportFoa = RvaToFoa(ctx, pDir->VirtualAddress);
    if (dwExportFoa == 0) return 0;

    PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)(ctx->lpFileBuffer + dwExportFoa);

    PDWORD pAddrOfFunctions = (PDWORD)(ctx->lpFileBuffer + RvaToFoa(ctx, pExport->AddressOfFunctions));

    // dwOrdinal 是面向用户的序号；减去 Base 得到数组索引
    DWORD dwIndex = dwOrdinal - pExport->Base;
    if (dwIndex >= pExport->NumberOfFunctions) return 0;

    return pAddrOfFunctions[dwIndex];
}
