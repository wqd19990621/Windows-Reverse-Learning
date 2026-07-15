#include "pe_parser.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// 获取重定位类型描述
// ============================================================================
static CONST CHAR* GetRelocTypeDesc(BYTE type)
{
    switch (type)
    {
    case IMAGE_REL_BASED_ABSOLUTE:  return "ABSOLUTE (填充 / 跳过)";
    case IMAGE_REL_BASED_HIGH:      return "HIGH";
    case IMAGE_REL_BASED_LOW:       return "LOW";
    case IMAGE_REL_BASED_HIGHLOW:   return "HIGHLOW (32 位)";
    case IMAGE_REL_BASED_HIGHADJ:   return "HIGHADJ";
    case IMAGE_REL_BASED_DIR64:     return "DIR64 (64 位)";
    case IMAGE_REL_BASED_ARM_MOV32: return "ARM_MOV32";
    case IMAGE_REL_BASED_THUMB_MOV32: return "THUMB_MOV32";
    default:                        return "未知";
    }
}

// ============================================================================
// ShowRelocations - 显示基址重定位表
//
// 修复: 使用 GetDataDirectory() 实现 32/64 位安全访问。
// 修复: 为各个重定位条目添加了缺失的 PRINT_INFO 输出。
// 之前条目被解析但从未打印。
// ============================================================================
void ShowRelocations(const PE_CONTEXT* ctx)
{
    if (!IsPeLoaded(ctx)) return;

    // 32/64 位安全访问重定位数据目录
    PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, IMAGE_DIRECTORY_ENTRY_BASERELOC);
    if (pDir == NULL || pDir->VirtualAddress == 0 || pDir->Size == 0)
    {
        PRINT_ERROR("错误 -> 此 PE 文件没有重定位表\r\n");
        return;
    }

    // 定位基址重定位表
    DWORD dwRelocFoa = RvaToFoa(ctx, pDir->VirtualAddress);
    if (dwRelocFoa == 0)
    {
        PRINT_ERROR("错误 -> 重定位表 RVA 到 FOA 转换失败\r\n");
        return;
    }

    PIMAGE_BASE_RELOCATION pRelocBlock = (PIMAGE_BASE_RELOCATION)(ctx->lpFileBuffer + dwRelocFoa);

    PRINT_TITLE("\n\n==== 基址重定位表信息 ====\n\n");

    int blockIndex = 0;
    DWORD totalEntries = 0;

    while (pRelocBlock->VirtualAddress != 0 && pRelocBlock->SizeOfBlock != 0)
    {
        blockIndex++;

        DWORD entryCount = (pRelocBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
        PWORD pEntry = (PWORD)((PBYTE)pRelocBlock + sizeof(IMAGE_BASE_RELOCATION));

        PRINT_INFO("----------------------------------------------------\r\n");
        PRINT_ERROR("  块 #%d\r\n", blockIndex);
        PRINT_ERROR("  页 RVA  (VirtualAddress) : 0x%08X\r\n", pRelocBlock->VirtualAddress);
        PRINT_INFO("  块大小 (SizeOfBlock)     : %u (0x%X)\r\n", pRelocBlock->SizeOfBlock, pRelocBlock->SizeOfBlock);
        PRINT_INFO("  条目数量                 : %u\r\n", entryCount);
        PRINT_INFO("\r\n");
        PRINT_INFO("  序号  类型                   偏移         RVA            描述\r\n");
        PRINT_INFO("  ----  --------------------  --------  -------------  ---------------------------\r\n");

        for (DWORD i = 0; i < entryCount; i++)
        {
            WORD entry = pEntry[i];
            BYTE type = (BYTE)((entry >> 12) & 0xF);
            WORD offset = entry & 0xFFF;
            DWORD rva = pRelocBlock->VirtualAddress + offset;

            PRINT_INFO("  %3u   %-20s  0x%04X    0x%08X      %s\r\n",
                i + 1,
                GetRelocTypeDesc(type),
                offset,
                rva,
                GetRelocTypeDesc(type));
        }

        totalEntries += entryCount;
        PRINT_INFO("----------------------------------------------------\r\n");

        // 前进到下一个块
        pRelocBlock = (PIMAGE_BASE_RELOCATION)((PBYTE)pRelocBlock + pRelocBlock->SizeOfBlock);
    }

    PRINT_INFO("\r\n总块数: %d\r\n", blockIndex);
    PRINT_INFO("总条目数: %u\r\n\n", totalEntries);
}
