#include "pe_parser.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// 用于信息摘要的数据目录显示名称
// ============================================================================
static const struct {
    int  index;
    CONST CHAR* name;
} kDirInfoTable[] = {
    { IMAGE_DIRECTORY_ENTRY_EXPORT,       "导出表"       },
    { IMAGE_DIRECTORY_ENTRY_IMPORT,       "导入表"       },
    { IMAGE_DIRECTORY_ENTRY_RESOURCE,     "资源表"       },
    { IMAGE_DIRECTORY_ENTRY_EXCEPTION,    "异常表"       },
    { IMAGE_DIRECTORY_ENTRY_SECURITY,     "安全表"       },
    { IMAGE_DIRECTORY_ENTRY_BASERELOC,    "基址重定位表" },
    { IMAGE_DIRECTORY_ENTRY_DEBUG,        "调试表"       },
    { IMAGE_DIRECTORY_ENTRY_TLS,          "TLS 表"       },
    { IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,  "加载配置表"   },
    { IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, "绑定导入表"   },
    { IMAGE_DIRECTORY_ENTRY_IAT,          "IAT"          },
    { IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, "延迟导入表"   },
    { IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,".NET 描述符" },
};

// ============================================================================
// ShowPeInfo - 显示 PE 文件基本信息摘要
// ============================================================================
void ShowPeInfo(const PE_CONTEXT* ctx)
{
    if (!IsPeLoaded(ctx)) return;

    PRINT_TITLE("\n\n==== PE 文件基本信息 ====\n\n");

    // 文件格式
    PRINT_INFO("文件格式:   %s\r\n", ctx->bIs64Bit ? "PE32+ (64 位)" : "PE32 (32 位)");

    // EXE 或 DLL
    CONST CHAR* szFileType = "未知";
    if (ctx->pNtHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL)
        szFileType = "DLL (动态链接库)";
    else if (ctx->pNtHeaders->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)
        szFileType = "EXE (可执行文件)";
    PRINT_INFO("文件类型:   %s\r\n", szFileType);

    // CPU 架构
    PRINT_INFO("CPU:        %s\r\n", GetMachineName(ctx->pNtHeaders->FileHeader.Machine));

    // 入口点
    PRINT_INFO("入口 RVA:   0x%08X\r\n",
        ctx->pNtHeaders->OptionalHeader.AddressOfEntryPoint);

    // 映像基址
    ULONGLONG imageBase = GetImageBase(ctx);
    if (ctx->bIs64Bit)
        PRINT_INFO("映像基址:   0x%016llX\r\n", imageBase);
    else
        PRINT_INFO("映像基址:   0x%08X\r\n", (DWORD)imageBase);

    // 节数量
    PRINT_INFO("节数量:      %d\r\n", ctx->pNtHeaders->FileHeader.NumberOfSections);

    // 子系统
    PRINT_INFO("子系统:      %s\r\n",
        GetSubsystemName(ctx->pNtHeaders->OptionalHeader.Subsystem));

    // 文件 / 映像大小
    PRINT_INFO("文件大小:    %u 字节 (0x%X)\r\n", ctx->dwFileSize, ctx->dwFileSize);
    PRINT_INFO("映像大小:    %u 字节 (0x%X)\r\n",
        GetSizeOfImage(ctx), GetSizeOfImage(ctx));

    // 数据目录摘要
    PRINT_INFO("\r\n存在的数据目录：\r\n");
    PRINT_INFO("----------------------------------------------------\r\n");

    int dirCount = 0;
    for (size_t i = 0; i < sizeof(kDirInfoTable) / sizeof(kDirInfoTable[0]); i++)
    {
        PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, kDirInfoTable[i].index);
        if (pDir && pDir->VirtualAddress != 0 && pDir->Size != 0)
        {
            PRINT_INFO("  [%02d] %-18s RVA: 0x%08X  大小: 0x%X\r\n",
                kDirInfoTable[i].index,
                kDirInfoTable[i].name,
                pDir->VirtualAddress,
                pDir->Size);
            dirCount++;
        }
    }

    if (dirCount == 0)
    {
        PRINT_INFO("  (无)\r\n");
    }

    PRINT_INFO("----------------------------------------------------\r\n\n");
}
