#include "pe_parser.h"
#include "pe_common.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// LoadPeFile - 从磁盘加载 PE 文件
// ============================================================================
BOOL LoadPeFile(PE_CONTEXT* ctx, CONST CHAR* filePath)
{
    if (ctx == NULL || filePath == NULL || *filePath == '\0')
        return FALSE;

    // 释放之前加载的文件
    FreePeFile(ctx);

    // 初始化上下文
    memset(ctx, 0, sizeof(PE_CONTEXT));

    // 打开文件
    HANDLE hFile = CreateFileA(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        PRINT_ERROR("错误 -> 无法打开文件 [%s] (错误代码: %d)\r\n", filePath, GetLastError());
        return FALSE;
    }

    // 获取文件大小
    LARGE_INTEGER liFileSize = { 0 };
    if (!GetFileSizeEx(hFile, &liFileSize) || liFileSize.QuadPart == 0)
    {
        PRINT_ERROR("错误 -> 获取文件大小失败 (错误代码: %d)\r\n", GetLastError());
        CloseHandle(hFile);
        return FALSE;
    }
    ctx->dwFileSize = (DWORD)liFileSize.QuadPart;

    // 分配内存
    ctx->lpFileBuffer = (PBYTE)malloc(ctx->dwFileSize);
    if (!ctx->lpFileBuffer)
    {
        PRINT_ERROR("错误 -> 内存分配失败 (大小: %u)\r\n", ctx->dwFileSize);
        CloseHandle(hFile);
        return FALSE;
    }

    // 读取文件
    DWORD dwBytesRead = 0;
    if (!ReadFile(hFile, ctx->lpFileBuffer, ctx->dwFileSize, &dwBytesRead, NULL)
        || dwBytesRead != ctx->dwFileSize)
    {
        PRINT_ERROR("错误 -> 文件读取失败 (预期: %u, 实际: %u)\r\n",
            ctx->dwFileSize, dwBytesRead);
        FreePeFile(ctx);
        CloseHandle(hFile);
        return FALSE;
    }
    CloseHandle(hFile);

    // --- 验证 DOS 头 ---
    ctx->pDosHeader = (PIMAGE_DOS_HEADER)ctx->lpFileBuffer;
    if (ctx->pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        PRINT_ERROR("错误 -> 无效的 DOS 签名 (0x%04X)\r\n", ctx->pDosHeader->e_magic);
        FreePeFile(ctx);
        return FALSE;
    }

    // --- 验证 NT 头偏移 ---
    ctx->dwNtOffset = ctx->pDosHeader->e_lfanew;
    if (ctx->dwNtOffset < sizeof(IMAGE_DOS_HEADER)
        || ctx->dwNtOffset + sizeof(IMAGE_NT_HEADERS64) > ctx->dwFileSize)
    {
        PRINT_ERROR("错误 -> 无效的 NT 头偏移 (0x%08X)\r\n", ctx->dwNtOffset);
        FreePeFile(ctx);
        return FALSE;
    }

    // --- 验证 NT 签名 ---
    ctx->pNtHeaders = (PIMAGE_NT_HEADERS)(ctx->lpFileBuffer + ctx->dwNtOffset);
    if (ctx->pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        PRINT_ERROR("错误 -> 无效的 PE 签名 (0x%08X)\r\n", ctx->pNtHeaders->Signature);
        FreePeFile(ctx);
        return FALSE;
    }

    // --- 判断 32 位 vs 64 位 ---
    WORD magic = ctx->pNtHeaders->OptionalHeader.Magic;
    if (magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC && magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        PRINT_ERROR("错误 -> 不支持的 OptionalHeader 魔数 (0x%04X)\r\n", magic);
        FreePeFile(ctx);
        return FALSE;
    }
    ctx->bIs64Bit = (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);

    // --- 定位节头 ---
    // 节头起始位置 = NT 偏移 + 4 (PE 签名) + 20 (FileHeader) + SizeOfOptionalHeader
    DWORD dwSectionOffset = ctx->dwNtOffset
        + sizeof(DWORD)                    // "PE\0\0" 签名
        + IMAGE_SIZEOF_FILE_HEADER         // 20 字节
        + ctx->pNtHeaders->FileHeader.SizeOfOptionalHeader;

    ctx->pSectionHeader = (PIMAGE_SECTION_HEADER)(ctx->lpFileBuffer + dwSectionOffset);

    PRINT_INFO("PE 文件加载成功 -> %s\r\n", filePath);
    PRINT_INFO("文件大小 -> 0x%08X (%u 字节)\r\n", ctx->dwFileSize, ctx->dwFileSize);
    PRINT_INFO("格式 -> %s\r\n", ctx->bIs64Bit ? "PE32+ (64 位)" : "PE32 (32 位)");

    return TRUE;
}

// ============================================================================
// FreePeFile - 释放所有 PE 资源
// ============================================================================
void FreePeFile(PE_CONTEXT* ctx)
{
    if (ctx == NULL) return;

    if (ctx->lpFileBuffer)
    {
        free(ctx->lpFileBuffer);
        ctx->lpFileBuffer = NULL;
    }

    ctx->dwFileSize = 0;
    ctx->pDosHeader = NULL;
    ctx->pNtHeaders = NULL;
    ctx->pSectionHeader = NULL;
    ctx->dwNtOffset = 0;
    ctx->bIs64Bit = FALSE;
}

// ============================================================================
// RvaToFoa - 将相对虚拟地址转换为文件偏移地址
// ============================================================================
DWORD RvaToFoa(const PE_CONTEXT* ctx, DWORD dwRva)
{
    if (ctx == NULL || ctx->pSectionHeader == NULL || ctx->lpFileBuffer == NULL)
        return 0;

    // 头区域内的 RVA 直接映射到文件偏移
    // SizeOfHeaders 包含 DOS 头 + PE 头 + 节头
    DWORD dwSizeOfHeaders = ctx->pNtHeaders->OptionalHeader.SizeOfHeaders;
    if (dwRva < dwSizeOfHeaders)
    {
        return dwRva;
    }

    // 搜索各个节
    for (WORD i = 0; i < ctx->pNtHeaders->FileHeader.NumberOfSections; i++)
    {
        DWORD dwStartRva = ctx->pSectionHeader[i].VirtualAddress;
        DWORD dwEndRva   = dwStartRva + ctx->pSectionHeader[i].Misc.VirtualSize;

        if (dwRva >= dwStartRva && dwRva < dwEndRva)
        {
            DWORD dwOffset = dwRva - dwStartRva;
            return ctx->pSectionHeader[i].PointerToRawData + dwOffset;
        }
    }

    return 0;
}

// ============================================================================
// FoaToRva - 将文件偏移地址转换为相对虚拟地址
// ============================================================================
DWORD FoaToRva(const PE_CONTEXT* ctx, DWORD dwFoa)
{
    if (ctx == NULL || ctx->pSectionHeader == NULL || ctx->lpFileBuffer == NULL)
        return 0;

    // 头区域内的文件偏移直接映射到 RVA
    DWORD dwSizeOfHeaders = ctx->pNtHeaders->OptionalHeader.SizeOfHeaders;
    if (dwFoa < dwSizeOfHeaders)
    {
        return dwFoa;
    }

    // 搜索各个节
    for (WORD i = 0; i < ctx->pNtHeaders->FileHeader.NumberOfSections; i++)
    {
        DWORD dwStartFoa = ctx->pSectionHeader[i].PointerToRawData;
        DWORD dwEndFoa   = dwStartFoa + ctx->pSectionHeader[i].SizeOfRawData;

        if (dwFoa >= dwStartFoa && dwFoa < dwEndFoa)
        {
            DWORD dwOffset = dwFoa - dwStartFoa;
            return ctx->pSectionHeader[i].VirtualAddress + dwOffset;
        }
    }

    return 0;
}

// ============================================================================
// FindSectionByRva - 定位某个 RVA 所在的节
// ============================================================================
int FindSectionByRva(const PE_CONTEXT* ctx, DWORD dwRva)
{
    if (ctx == NULL || ctx->pSectionHeader == NULL)
        return -1;

    for (WORD i = 0; i < ctx->pNtHeaders->FileHeader.NumberOfSections; i++)
    {
        DWORD dwStartRva = ctx->pSectionHeader[i].VirtualAddress;
        DWORD dwEndRva   = dwStartRva + ctx->pSectionHeader[i].Misc.VirtualSize;

        if (dwRva >= dwStartRva && dwRva < dwEndRva)
            return (int)i;
    }

    return -1;
}
