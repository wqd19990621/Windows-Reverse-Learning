#include "pe_common.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// 节标志描述符
// ============================================================================
static const FLAG_DESC kSectionFlags[] = {
    { IMAGE_SCN_CNT_CODE,               "包含可执行代码" },
    { IMAGE_SCN_CNT_INITIALIZED_DATA,   "包含已初始化数据" },
    { IMAGE_SCN_CNT_UNINITIALIZED_DATA, "包含未初始化数据" },
    { IMAGE_SCN_LNK_NRELOC_OVFL,        "包含扩展重定位" },
    { IMAGE_SCN_MEM_DISCARDABLE,        "可丢弃" },
    { IMAGE_SCN_MEM_NOT_CACHED,         "不可缓存" },
    { IMAGE_SCN_MEM_NOT_PAGED,          "不可分页" },
    { IMAGE_SCN_MEM_SHARED,             "共享内存" },
    { IMAGE_SCN_MEM_EXECUTE,            "可执行" },
    { IMAGE_SCN_MEM_READ,               "可读" },
    { IMAGE_SCN_MEM_WRITE,              "可写" },
};

// ============================================================================
// 打印单个节的详细信息
// ============================================================================
static void PrintOneSection(PIMAGE_SECTION_HEADER pSection, int index)
{
    CHAR szName[9] = { 0 };
    GetSectionName(pSection, szName);

    PRINT_TITLE("  节 [%d]: %s\r\n", index, szName);
    PRINT_ERROR("  0000h    Name                  ->  \"%s\"      // 节名称\r\n", szName);
    PRINT_ERROR("  0008h    VirtualSize           ->  0x%08X      // 内存中的虚拟大小\r\n", pSection->Misc.VirtualSize);
    PRINT_ERROR("  000Ch    VirtualAddress        ->  0x%08X      // 内存中的起始 RVA\r\n", pSection->VirtualAddress);
    PRINT_ERROR("  0010h    SizeOfRawData         ->  0x%08X      // 文件中的大小\r\n", pSection->SizeOfRawData);
    PRINT_ERROR("  0014h    PointerToRawData      ->  0x%08X      // 文件偏移 (FOA)\r\n", pSection->PointerToRawData);
    PRINT_INFO("  0018h    PointerToRelocations  ->  0x%08X      // 重定位表指针\r\n", pSection->PointerToRelocations);
    PRINT_INFO("  001Ch    PointerToLinenumbers  ->  0x%08X      // 行号表指针\r\n", pSection->PointerToLinenumbers);
    PRINT_INFO("  0020h    NumberOfRelocations   ->  0x%04X         // 重定位条目数\r\n", pSection->NumberOfRelocations);
    PRINT_INFO("  0022h    NumberOfLinenumbers   ->  0x%04X         // 行号条目数\r\n", pSection->NumberOfLinenumbers);
    PRINT_ERROR("  0024h    Characteristics       ->  0x%08X      // 节标志\r\n", pSection->Characteristics);

    PRINT_INFO("  节标志: ");
    for (size_t j = 0; j < sizeof(kSectionFlags) / sizeof(kSectionFlags[0]); j++)
    {
        if (pSection->Characteristics & kSectionFlags[j].flag)
        {
            PRINT_INFO(" %s;", kSectionFlags[j].desc);
        }
    }
    PRINT_INFO("\r\n");
    PRINT_INFO("----------------------------------------\r\n");
}

// ============================================================================
// ShowSections - 显示所有节头
// ============================================================================
void ShowSections(const PE_CONTEXT* ctx)
{
    if (!IsPeLoaded(ctx)) return;

    DWORD totalVirtualSize = 0;
    DWORD totalRawSize = 0;
    WORD nSections = ctx->pNtHeaders->FileHeader.NumberOfSections;

    PRINT_TITLE("\n\n==== 节头信息 ====\n\n");

    for (WORD i = 0; i < nSections; i++)
    {
        PIMAGE_SECTION_HEADER pSection = &ctx->pSectionHeader[i];
        totalVirtualSize += pSection->Misc.VirtualSize;
        totalRawSize += pSection->SizeOfRawData;
        PrintOneSection(pSection, i);
    }

    // 摘要
    PRINT_INFO("\n==== 摘要 ====\r\n");
    PRINT_INFO("节总数:                    %d\r\n", nSections);
    PRINT_INFO("虚拟总大小:                %u (0x%X) 字节\r\n", totalVirtualSize, totalVirtualSize);
    PRINT_INFO("原始数据总大小:            %u (0x%X) 字节\r\n\n", totalRawSize, totalRawSize);
}
