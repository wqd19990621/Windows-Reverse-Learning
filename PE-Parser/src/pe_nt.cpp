#include "pe_common.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

// ============================================================================
// 静态标志描述符表
// ============================================================================
static const FLAG_DESC_WORD kFileCharacteristics[] = {
    { IMAGE_FILE_RELOCS_STRIPPED,         "重定位信息已从文件中剥离" },
    { IMAGE_FILE_EXECUTABLE_IMAGE,        "文件是可执行的（无未解析的外部引用）" },
    { IMAGE_FILE_LINE_NUMS_STRIPPED,      "行号信息已从文件中剥离" },
    { IMAGE_FILE_LOCAL_SYMS_STRIPPED,     "本地符号已从文件中剥离" },
    { IMAGE_FILE_AGGRESIVE_WS_TRIM,       "积极缩减工作集" },
    { IMAGE_FILE_LARGE_ADDRESS_AWARE,     "应用程序可处理大于 2GB 的地址" },
    { IMAGE_FILE_BYTES_REVERSED_LO,       "机器字的字节顺序已反转（低）" },
    { IMAGE_FILE_32BIT_MACHINE,           "32 位字机器" },
    { IMAGE_FILE_DEBUG_STRIPPED,          "调试信息已剥离到 .DBG 文件" },
    { IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP, "如果在可移动介质上，复制并从交换区运行" },
    { IMAGE_FILE_NET_RUN_FROM_SWAP,       "如果在网络上，复制并从交换区运行" },
    { IMAGE_FILE_SYSTEM,                  "系统文件" },
    { IMAGE_FILE_DLL,                     "文件是 DLL" },
    { IMAGE_FILE_UP_SYSTEM_ONLY,          "文件仅应在单处理器机器上运行" },
    { IMAGE_FILE_BYTES_REVERSED_HI,       "机器字的字节顺序已反转（高）" },
};

static const FLAG_DESC_WORD kDllCharacteristics[] = {
    { IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA,       "高熵 64 位 ASLR" },
    { IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE,           "ASLR (动态基址)" },
    { IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY,        "强制代码完整性检查" },
    { IMAGE_DLLCHARACTERISTICS_NX_COMPAT,              "NX 兼容 (DEP)" },
    { IMAGE_DLLCHARACTERISTICS_NO_ISOLATION,           "无隔离" },
    { IMAGE_DLLCHARACTERISTICS_NO_SEH,                 "无 SEH (结构化异常处理)" },
    { IMAGE_DLLCHARACTERISTICS_NO_BIND,                "无导入绑定" },
    { IMAGE_DLLCHARACTERISTICS_APPCONTAINER,           "AppContainer (UWP)" },
    { IMAGE_DLLCHARACTERISTICS_WDM_DRIVER,             "WDM 驱动程序" },
    { IMAGE_DLLCHARACTERISTICS_GUARD_CF,               "控制流保护 (CFG)" },
    { IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE,  "终端服务器感知" },
};

// ============================================================================
// 打印 32 位可选头
// ============================================================================
static void PrintOptionalHeader32(const PE_CONTEXT* ctx)
{
    PIMAGE_NT_HEADERS32 pNt32 = (PIMAGE_NT_HEADERS32)(ctx->lpFileBuffer + ctx->dwNtOffset);
    PIMAGE_OPTIONAL_HEADER32 pOpt = &pNt32->OptionalHeader;

    PRINT_INFO("  0002h    MajorLinkerVersion       ->  0x%02X         // 链接器主版本\r\n", pOpt->MajorLinkerVersion);
    PRINT_INFO("  0003h    MinorLinkerVersion       ->  0x%02X         // 链接器副版本\r\n", pOpt->MinorLinkerVersion);
    PRINT_INFO("  0004h    SizeOfCode               ->  0x%08X      // 代码段总大小\r\n", pOpt->SizeOfCode);
    PRINT_INFO("  0008h    SizeOfInitializedData    ->  0x%08X      // 已初始化数据总大小\r\n", pOpt->SizeOfInitializedData);
    PRINT_INFO("  000Ch    SizeOfUninitializedData  ->  0x%08X      // 未初始化数据总大小\r\n", pOpt->SizeOfUninitializedData);
    PRINT_ERROR("  0010h    AddressOfEntryPoint      ->  0x%08X      // 入口点 RVA\r\n", pOpt->AddressOfEntryPoint);
    PRINT_INFO("  0014h    BaseOfCode               ->  0x%08X      // 代码段起始 RVA\r\n", pOpt->BaseOfCode);
    PRINT_INFO("  0018h    BaseOfData               ->  0x%08X      // 数据段起始 RVA (仅 32 位)\r\n", pOpt->BaseOfData);
    PRINT_ERROR("  001Ch    ImageBase                ->  0x%08X      // 首选基址\r\n", pOpt->ImageBase);
    PRINT_ERROR("  0020h    SectionAlignment         ->  0x%08X      // 内存中的节对齐\r\n", pOpt->SectionAlignment);
    PRINT_ERROR("  0024h    FileAlignment            ->  0x%08X      // 文件中的节对齐\r\n", pOpt->FileAlignment);
    PRINT_INFO("  0028h    MajorOSVersion           ->  0x%04X         // 操作系统主版本\r\n", pOpt->MajorOperatingSystemVersion);
    PRINT_INFO("  002Ah    MinorOSVersion           ->  0x%04X         // 操作系统副版本\r\n", pOpt->MinorOperatingSystemVersion);
    PRINT_INFO("  002Ch    MajorImageVersion        ->  0x%04X         // 映像主版本\r\n", pOpt->MajorImageVersion);
    PRINT_INFO("  002Eh    MinorImageVersion        ->  0x%04X         // 映像副版本\r\n", pOpt->MinorImageVersion);
    PRINT_INFO("  0030h    MajorSubsystemVersion    ->  0x%04X         // 子系统主版本\r\n", pOpt->MajorSubsystemVersion);
    PRINT_INFO("  0032h    MinorSubsystemVersion    ->  0x%04X         // 子系统副版本\r\n", pOpt->MinorSubsystemVersion);
    PRINT_INFO("  0034h    Win32VersionValue        ->  0x%08X      // 保留（通常为 0）\r\n", pOpt->Win32VersionValue);
    PRINT_ERROR("  0038h    SizeOfImage              ->  0x%08X      // 内存中的映像总大小\r\n", pOpt->SizeOfImage);
    PRINT_ERROR("  003Ch    SizeOfHeaders            ->  0x%08X      // 头总大小\r\n", pOpt->SizeOfHeaders);
    PRINT_INFO("  0040h    CheckSum                 ->  0x%08X      // 文件校验和\r\n", pOpt->CheckSum);
    PRINT_ERROR("  0044h    Subsystem                ->  0x%04X         // 子系统类型: %s\r\n",
        pOpt->Subsystem, GetSubsystemName(pOpt->Subsystem));
    PRINT_ERROR("  0046h    DllCharacteristics       ->  0x%04X         // DLL 特征\r\n", pOpt->DllCharacteristics);

    PRINT_INFO("DLL 特征标志：\r\n");
    PrintFlagsWord(pOpt->DllCharacteristics, kDllCharacteristics,
        sizeof(kDllCharacteristics) / sizeof(kDllCharacteristics[0]));

    PRINT_INFO("  0048h    SizeOfStackReserve       ->  0x%08X      // 栈保留大小\r\n", pOpt->SizeOfStackReserve);
    PRINT_INFO("  004Ch    SizeOfStackCommit        ->  0x%08X      // 栈提交大小\r\n", pOpt->SizeOfStackCommit);
    PRINT_INFO("  0050h    SizeOfHeapReserve        ->  0x%08X      // 堆保留大小\r\n", pOpt->SizeOfHeapReserve);
    PRINT_INFO("  0054h    SizeOfHeapCommit         ->  0x%08X      // 堆提交大小\r\n", pOpt->SizeOfHeapCommit);
    PRINT_INFO("  0058h    LoaderFlags              ->  0x%08X      // 加载器标志（通常为 0）\r\n", pOpt->LoaderFlags);
    PRINT_ERROR("  005Ch    NumberOfRvaAndSizes      ->  0x%08X      // 数据目录项数量\r\n", pOpt->NumberOfRvaAndSizes);
}

// ============================================================================
// 打印 64 位可选头
// ============================================================================
static void PrintOptionalHeader64(const PE_CONTEXT* ctx)
{
    PIMAGE_NT_HEADERS64 pNt64 = (PIMAGE_NT_HEADERS64)(ctx->lpFileBuffer + ctx->dwNtOffset);
    PIMAGE_OPTIONAL_HEADER64 pOpt = &pNt64->OptionalHeader;

    PRINT_INFO("  0002h    MajorLinkerVersion       ->  0x%02X         // 链接器主版本\r\n", pOpt->MajorLinkerVersion);
    PRINT_INFO("  0003h    MinorLinkerVersion       ->  0x%02X         // 链接器副版本\r\n", pOpt->MinorLinkerVersion);
    PRINT_INFO("  0004h    SizeOfCode               ->  0x%08X      // 代码段总大小\r\n", pOpt->SizeOfCode);
    PRINT_INFO("  0008h    SizeOfInitializedData    ->  0x%08X      // 已初始化数据总大小\r\n", pOpt->SizeOfInitializedData);
    PRINT_INFO("  000Ch    SizeOfUninitializedData  ->  0x%08X      // 未初始化数据总大小\r\n", pOpt->SizeOfUninitializedData);
    PRINT_ERROR("  0010h    AddressOfEntryPoint      ->  0x%08X      // 入口点 RVA\r\n", pOpt->AddressOfEntryPoint);
    PRINT_INFO("  0014h    BaseOfCode               ->  0x%08X      // 代码段起始 RVA\r\n", pOpt->BaseOfCode);
    PRINT_INFO("  0018h    ImageBase                ->  0x%016llX  // 首选基址 (64 位)\r\n", pOpt->ImageBase);
    PRINT_INFO("  0020h    SectionAlignment         ->  0x%08X      // 内存中的节对齐\r\n", pOpt->SectionAlignment);
    PRINT_INFO("  0024h    FileAlignment            ->  0x%08X      // 文件中的节对齐\r\n", pOpt->FileAlignment);
    PRINT_INFO("  0028h    MajorOSVersion           ->  %d              // 操作系统主版本\r\n", pOpt->MajorOperatingSystemVersion);
    PRINT_INFO("  002Ah    MinorOSVersion           ->  %d              // 操作系统副版本\r\n", pOpt->MinorOperatingSystemVersion);
    PRINT_INFO("  002Ch    MajorImageVersion        ->  %d              // 映像主版本\r\n", pOpt->MajorImageVersion);
    PRINT_INFO("  002Eh    MinorImageVersion        ->  %d              // 映像副版本\r\n", pOpt->MinorImageVersion);
    PRINT_INFO("  0030h    MajorSubsystemVersion    ->  %d              // 子系统主版本\r\n", pOpt->MajorSubsystemVersion);
    PRINT_INFO("  0032h    MinorSubsystemVersion    ->  %d              // 子系统副版本\r\n", pOpt->MinorSubsystemVersion);
    PRINT_INFO("  0034h    Win32VersionValue        ->  0x%08X      // 保留（通常为 0）\r\n", pOpt->Win32VersionValue);
    PRINT_INFO("  0038h    SizeOfImage              ->  0x%08X      // 内存中的映像总大小\r\n", pOpt->SizeOfImage);
    PRINT_INFO("  003Ch    SizeOfHeaders            ->  0x%08X      // 头总大小\r\n", pOpt->SizeOfHeaders);
    PRINT_INFO("  0040h    CheckSum                 ->  0x%08X      // 文件校验和\r\n", pOpt->CheckSum);
    PRINT_INFO("  0044h    Subsystem                ->  0x%04X         // 子系统类型\r\n", pOpt->Subsystem);
    PRINT_INFO("  0046h    DllCharacteristics       ->  0x%04X         // DLL 特征\r\n", pOpt->DllCharacteristics);

    PRINT_INFO("DLL 特征标志：\r\n");
    PrintFlagsWord(pOpt->DllCharacteristics, kDllCharacteristics,
        sizeof(kDllCharacteristics) / sizeof(kDllCharacteristics[0]));

    PRINT_INFO("  0048h    SizeOfStackReserve       ->  0x%016llX  // 栈保留大小\r\n", pOpt->SizeOfStackReserve);
    PRINT_INFO("  0050h    SizeOfStackCommit        ->  0x%016llX  // 栈提交大小\r\n", pOpt->SizeOfStackCommit);
    PRINT_INFO("  0058h    SizeOfHeapReserve        ->  0x%016llX  // 堆保留大小\r\n", pOpt->SizeOfHeapReserve);
    PRINT_INFO("  0060h    SizeOfHeapCommit         ->  0x%016llX  // 堆提交大小\r\n", pOpt->SizeOfHeapCommit);
    PRINT_INFO("  0068h    LoaderFlags              ->  0x%08X      // 加载器标志（通常为 0）\r\n", pOpt->LoaderFlags);
    PRINT_INFO("  006Ch    NumberOfRvaAndSizes      ->  %d              // 数据目录项数量\r\n", pOpt->NumberOfRvaAndSizes);
}

// ============================================================================
// 打印数据目录条目（通过 GetDataDirectory 兼容 32/64 位）
// ============================================================================
static void PrintDataDirectories(const PE_CONTEXT* ctx)
{
    PRINT_INFO("\n数据目录条目：\r\n");
    PRINT_INFO("----------------------------------------------------\r\n");

    for (DWORD i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++)
    {
        PIMAGE_DATA_DIRECTORY pDir = GetDataDirectory(ctx, i);
        if (pDir && pDir->VirtualAddress != 0 && pDir->Size != 0)
        {
            PRINT_INFO("  [%02d] %-24s RVA: 0x%08X  大小: 0x%X\r\n",
                i, GetDataDirName(i), pDir->VirtualAddress, pDir->Size);
        }
    }
    PRINT_INFO("----------------------------------------------------\r\n");
}

// ============================================================================
// ShowNtHeader - 显示 NT 头（FileHeader + OptionalHeader）
// ============================================================================
void ShowNtHeader(const PE_CONTEXT* ctx)
{
    if (!IsPeLoaded(ctx)) return;

    if (ctx->pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        PRINT_ERROR("错误 -> 无效的 PE 签名 (期望: 0x00004550, 实际: 0x%08X)\r\n",
            ctx->pNtHeaders->Signature);
        return;
    }

    PRINT_TITLE("\n\n==== NT 头信息 ====\n\n");

    // --- 1. 签名 ---
    PRINT_INFO("------------------------\r\n");
    PRINT_INFO("1. 签名\r\n");
    PRINT_INFO("------------------------\r\n\n");
    PRINT_ERROR("  0000h    Signature       ->  0x%08X        // PE 签名 (\"PE\\0\\0\")\r\n",
        ctx->pNtHeaders->Signature);
    PRINT_INFO("\n");

    // --- 2. 文件头 ---
    PRINT_INFO("------------------------\r\n");
    PRINT_INFO("2. 文件头 (COFF)\r\n");
    PRINT_INFO("------------------------\r\n\n");

    PRINT_ERROR("  0000h    Machine              ->  0x%04X         // 目标 CPU: %s\r\n",
        ctx->pNtHeaders->FileHeader.Machine, GetMachineName(ctx->pNtHeaders->FileHeader.Machine));
    PRINT_ERROR("  0002h    NumberOfSections     ->  0x%04X         // 节数量\r\n",
        ctx->pNtHeaders->FileHeader.NumberOfSections);

    // 时间戳，附带人类可读的转换
    time_t timestamp = (time_t)ctx->pNtHeaders->FileHeader.TimeDateStamp;
    struct tm localTime = { 0 };
    localtime_s(&localTime, &timestamp);
    CHAR timeBuffer[64] = { 0 };
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &localTime);

    PRINT_INFO("  0004h    TimeDateStamp        ->  0x%08X      // 时间戳: %s\r\n",
        ctx->pNtHeaders->FileHeader.TimeDateStamp, timeBuffer);
    PRINT_INFO("  0008h    PointerToSymbolTable ->  0x%08X      // COFF 符号表偏移\r\n",
        ctx->pNtHeaders->FileHeader.PointerToSymbolTable);
    PRINT_INFO("  000Ch    NumberOfSymbols      ->  0x%08X      // COFF 符号数量\r\n",
        ctx->pNtHeaders->FileHeader.NumberOfSymbols);
    PRINT_ERROR("  0010h    SizeOfOptionalHeader ->  0x%04X         // 可选头大小 (%s)\r\n",
        ctx->pNtHeaders->FileHeader.SizeOfOptionalHeader,
        ctx->bIs64Bit ? "64 位 = 0xF0" : "32 位 = 0xE0");
    PRINT_INFO("  0012h    Characteristics      ->  0x%04X         // 文件特征\r\n",
        ctx->pNtHeaders->FileHeader.Characteristics);

    PRINT_INFO("文件特征标志：\r\n");
    PrintFlagsWord(ctx->pNtHeaders->FileHeader.Characteristics, kFileCharacteristics,
        sizeof(kFileCharacteristics) / sizeof(kFileCharacteristics[0]));
    PRINT_INFO("\n");

    // --- 3. 可选头 ---
    PRINT_INFO("------------------------\r\n");
    PRINT_INFO("3. 可选头\r\n");
    PRINT_INFO("------------------------\r\n\n");

    WORD magic = ctx->pNtHeaders->OptionalHeader.Magic;
    PRINT_ERROR("  0000h    Magic             ->  0x%04X         // %s\r\n",
        magic, ctx->bIs64Bit ? "PE32+ (64 位)" : "PE32 (32 位)");

    if (ctx->bIs64Bit)
    {
        PrintOptionalHeader64(ctx);
    }
    else
    {
        PrintOptionalHeader32(ctx);
    }

    // 打印非空的数据目录条目
    PrintDataDirectories(ctx);
    PRINT_INFO("\n");
}
