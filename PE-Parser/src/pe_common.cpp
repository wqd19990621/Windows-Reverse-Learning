#include "pe_common.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// 架构名称查找
// ============================================================================
CONST CHAR* GetMachineName(WORD machine)
{
    switch (machine)
    {
    case IMAGE_FILE_MACHINE_I386:     return "x86 (Intel 386)";
    case IMAGE_FILE_MACHINE_AMD64:    return "x64 (AMD64)";
    case IMAGE_FILE_MACHINE_IA64:     return "IA64 (Intel Itanium)";
    case IMAGE_FILE_MACHINE_ARMNT:    return "ARM Thumb-2 (ARMv7)";
    case IMAGE_FILE_MACHINE_ARM64:    return "ARM64 (AArch64)";
    case IMAGE_FILE_MACHINE_THUMB:    return "ARM Thumb";
    case IMAGE_FILE_MACHINE_MIPS16:   return "MIPS16";
    case IMAGE_FILE_MACHINE_MIPSFPU:  return "MIPS FPU";
    case IMAGE_FILE_MACHINE_MIPSFPU16:return "MIPS16 FPU";
    case IMAGE_FILE_MACHINE_POWERPC:  return "PowerPC (大端序)";
    case IMAGE_FILE_MACHINE_POWERPCFP:return "PowerPC FPU";
    case IMAGE_FILE_MACHINE_R4000:    return "MIPS R4000 (小端序)";
    case IMAGE_FILE_MACHINE_SH3:      return "Hitachi SH3";
    case IMAGE_FILE_MACHINE_SH4:      return "Hitachi SH4";
    case IMAGE_FILE_MACHINE_SH5:      return "Hitachi SH5";
    case IMAGE_FILE_MACHINE_ALPHA:    return "DEC Alpha AXP";
    case IMAGE_FILE_MACHINE_ALPHA64:  return "DEC Alpha AXP 64 位";
    case IMAGE_FILE_MACHINE_TRICORE:  return "Infineon TriCore";
    case IMAGE_FILE_MACHINE_CEF:      return "通用可执行格式";
    case IMAGE_FILE_MACHINE_EBC:      return "EFI 字节码";
    case IMAGE_FILE_MACHINE_AM33:     return "Matsushita AM33";
    case IMAGE_FILE_MACHINE_M32R:     return "Mitsubishi M32R";
    case IMAGE_FILE_MACHINE_ARM:      return "ARM (小端序)";
    case IMAGE_FILE_MACHINE_CEE:      return "CEE (Microsoft .NET)";
    default:                          return "未知";
    }
}

// ============================================================================
// 子系统名称查找
// ============================================================================
CONST CHAR* GetSubsystemName(WORD subsystem)
{
    switch (subsystem)
    {
    case IMAGE_SUBSYSTEM_UNKNOWN:                  return "未知";
    case IMAGE_SUBSYSTEM_NATIVE:                   return "原生 (驱动程序)";
    case IMAGE_SUBSYSTEM_WINDOWS_GUI:              return "Windows GUI";
    case IMAGE_SUBSYSTEM_WINDOWS_CUI:              return "Windows 控制台 (CUI)";
    case IMAGE_SUBSYSTEM_OS2_CUI:                  return "OS/2 控制台";
    case IMAGE_SUBSYSTEM_POSIX_CUI:                return "POSIX 控制台";
    case IMAGE_SUBSYSTEM_NATIVE_WINDOWS:           return "原生 Win9x 驱动";
    case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:           return "Windows CE GUI";
    case IMAGE_SUBSYSTEM_EFI_APPLICATION:          return "EFI 应用程序";
    case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:  return "EFI 启动服务驱动";
    case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:       return "EFI 运行时驱动";
    case IMAGE_SUBSYSTEM_EFI_ROM:                  return "EFI ROM";
    case IMAGE_SUBSYSTEM_XBOX:                     return "Xbox";
    case IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION: return "Windows 启动应用程序";
    case IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG:        return "Xbox 代码目录";
    default:                                       return "未知";
    }
}

// ============================================================================
// 数据目录名称查找
// ============================================================================
CONST CHAR* GetDataDirName(DWORD index)
{
    switch (index)
    {
    case IMAGE_DIRECTORY_ENTRY_EXPORT:        return "导出目录";
    case IMAGE_DIRECTORY_ENTRY_IMPORT:        return "导入目录";
    case IMAGE_DIRECTORY_ENTRY_RESOURCE:      return "资源目录";
    case IMAGE_DIRECTORY_ENTRY_EXCEPTION:     return "异常目录";
    case IMAGE_DIRECTORY_ENTRY_SECURITY:      return "安全目录";
    case IMAGE_DIRECTORY_ENTRY_BASERELOC:     return "基址重定位表";
    case IMAGE_DIRECTORY_ENTRY_DEBUG:         return "调试目录";
    case IMAGE_DIRECTORY_ENTRY_ARCHITECTURE:  return "架构特定数据";
    case IMAGE_DIRECTORY_ENTRY_GLOBALPTR:     return "全局指针寄存器";
    case IMAGE_DIRECTORY_ENTRY_TLS:           return "TLS 目录";
    case IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG:   return "加载配置目录";
    case IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT:  return "绑定导入目录";
    case IMAGE_DIRECTORY_ENTRY_IAT:           return "导入地址表";
    case IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT:  return "延迟导入目录";
    case IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR:return "COM / .NET 描述符";
    default:                                  return "未知";
    }
}

// ============================================================================
// 标志打印辅助函数
// ============================================================================
void PrintFlagsDword(DWORD bitfield, const FLAG_DESC* flags, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        if (bitfield & flags[i].flag)
        {
            PRINT_INFO("    标志 -> 0x%08X  信息 -> %s\r\n", flags[i].flag, flags[i].desc);
        }
    }
}

void PrintFlagsWord(WORD bitfield, const FLAG_DESC_WORD* flags, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        if (bitfield & flags[i].flag)
        {
            PRINT_INFO("    标志 -> 0x%04X  信息 -> %s\r\n", flags[i].flag, flags[i].desc);
        }
    }
}

// ============================================================================
// 节名称提取器
// ============================================================================
void GetSectionName(PIMAGE_SECTION_HEADER pSection, CHAR outName[9])
{
    memcpy(outName, pSection->Name, IMAGE_SIZEOF_SHORT_NAME);
    outName[IMAGE_SIZEOF_SHORT_NAME] = '\0';
}

// ============================================================================
// PE 验证辅助函数
// ============================================================================
BOOL IsPeLoaded(const PE_CONTEXT* ctx)
{
    if (ctx == NULL || ctx->pSectionHeader == NULL || ctx->lpFileBuffer == NULL)
    {
        PRINT_ERROR("错误 -> 请先使用 'load' 命令加载 PE 文件\r\n");
        return FALSE;
    }
    return TRUE;
}

BOOL HasDataDirectory(const PE_CONTEXT* ctx, DWORD dirIndex)
{
    PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, dirIndex);
    if (pDir == NULL) return FALSE;
    return (pDir->VirtualAddress != 0 && pDir->Size != 0);
}

// ============================================================================
// 32/64 位感知的 DataDirectory 访问
//
// 这是支持 32 位 PE 的关键修复。
// IMAGE_OPTIONAL_HEADER32 和 IMAGE_OPTIONAL_HEADER64 具有不同的布局：
//   - 32 位: DataDirectory 从 OptionalHeader 内偏移 96 (0x60) 处开始
//   - 64 位: DataDirectory 从 OptionalHeader 内偏移 112 (0x70) 处开始
//
// 使用 PIMAGE_NT_HEADERS（根据编译目标解析为 32 或 64 位）
// 来访问与编译目标不同的文件将会读取错误的偏移量。
// ============================================================================
PIMAGE_DATA_DIRECTORY GetDataDirectory(const PE_CONTEXT* ctx, DWORD dirIndex)
{
    if (ctx == NULL || ctx->lpFileBuffer == NULL || ctx->pNtHeaders == NULL)
        return NULL;

    if (dirIndex >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
        return NULL;

    // 根据文件的实际 PE 格式（而非编译目标）来判断
    WORD magic = ctx->pNtHeaders->OptionalHeader.Magic;

    if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        // PE32+ (64 位): DataDirectory 位于不同的偏移处
        PIMAGE_NT_HEADERS64 pNt64 = (PIMAGE_NT_HEADERS64)(ctx->lpFileBuffer + ctx->dwNtOffset);
        return &pNt64->OptionalHeader.DataDirectory[dirIndex];
    }
    else
    {
        // PE32 (32 位)
        PIMAGE_NT_HEADERS32 pNt32 = (PIMAGE_NT_HEADERS32)(ctx->lpFileBuffer + ctx->dwNtOffset);
        return &pNt32->OptionalHeader.DataDirectory[dirIndex];
    }
}

// ============================================================================
// 32/64 位感知的 ImageBase 访问
// ============================================================================
ULONGLONG GetImageBase(const PE_CONTEXT* ctx)
{
    if (ctx == NULL) return 0;

    if (ctx->bIs64Bit)
    {
        PIMAGE_NT_HEADERS64 pNt64 = (PIMAGE_NT_HEADERS64)(ctx->lpFileBuffer + ctx->dwNtOffset);
        return pNt64->OptionalHeader.ImageBase;
    }
    else
    {
        PIMAGE_NT_HEADERS32 pNt32 = (PIMAGE_NT_HEADERS32)(ctx->lpFileBuffer + ctx->dwNtOffset);
        return pNt32->OptionalHeader.ImageBase;
    }
}

// ============================================================================
// 32/64 位感知的 SizeOfImage 访问
// ============================================================================
DWORD GetSizeOfImage(const PE_CONTEXT* ctx)
{
    if (ctx == NULL) return 0;

    if (ctx->bIs64Bit)
    {
        PIMAGE_NT_HEADERS64 pNt64 = (PIMAGE_NT_HEADERS64)(ctx->lpFileBuffer + ctx->dwNtOffset);
        return pNt64->OptionalHeader.SizeOfImage;
    }
    else
    {
        PIMAGE_NT_HEADERS32 pNt32 = (PIMAGE_NT_HEADERS32)(ctx->lpFileBuffer + ctx->dwNtOffset);
        return pNt32->OptionalHeader.SizeOfImage;
    }
}
