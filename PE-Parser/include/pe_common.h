#ifndef PE_COMMON_H
#define PE_COMMON_H

#include <Windows.h>

// ============================================================================
// 控制台颜色宏
// ============================================================================
#define CLR_RESET   "\x1b[0m"
#define CLR_TITLE   "\x1b[1;33m"   // 黄色
#define CLR_MENU    "\x1b[1;36m"   // 青色
#define CLR_INPUT   "\x1b[1;32m"   // 绿色
#define CLR_ERROR   "\x1b[1;31m"   // 红色
#define CLR_INFO    "\x1b[1;37m"   // 白色

#define PRINT_TITLE(fmt, ...)  printf(CLR_TITLE fmt CLR_RESET, ##__VA_ARGS__)
#define PRINT_MENU(fmt, ...)   printf(CLR_MENU  fmt CLR_RESET, ##__VA_ARGS__)
#define PRINT_INPUT(fmt, ...)  printf(CLR_INPUT fmt CLR_RESET, ##__VA_ARGS__)
#define PRINT_ERROR(fmt, ...)  printf(CLR_ERROR fmt CLR_RESET, ##__VA_ARGS__)
#define PRINT_INFO(fmt, ...)   printf(CLR_INFO  fmt CLR_RESET, ##__VA_ARGS__)

// ============================================================================
// PE 上下文结构体 - 保存所有已加载 PE 文件的状态
// ============================================================================
typedef struct _PE_CONTEXT {
    PBYTE                  lpFileBuffer;      // 完整文件缓冲区
    DWORD                  dwFileSize;        // 文件大小
    PIMAGE_DOS_HEADER      pDosHeader;        // DOS 头
    PIMAGE_NT_HEADERS      pNtHeaders;        // NT 头（原生指针类型）
    PIMAGE_SECTION_HEADER  pSectionHeader;     // 节头数组
    DWORD                  dwNtOffset;        // NT 头的文件偏移
    BOOL                   bIs64Bit;          // TRUE = PE32+, FALSE = PE32
} PE_CONTEXT;

// ============================================================================
// 特征 / 标志描述符辅助结构体
// ============================================================================
typedef struct _FLAG_DESC {
    DWORD       flag;
    CONST CHAR* desc;
} FLAG_DESC;

typedef struct _FLAG_DESC_WORD {
    WORD        flag;
    CONST CHAR* desc;
} FLAG_DESC_WORD;

// ============================================================================
// 工具函数声明
// ============================================================================

// 从 Machine 字段获取人类可读的架构名称
CONST CHAR* GetMachineName(WORD machine);

// 获取人类可读的子系统名称
CONST CHAR* GetSubsystemName(WORD subsystem);

// 获取人类可读的数据目录条目名称
CONST CHAR* GetDataDirName(DWORD index);

// 打印位字段中设置的标志（DWORD 版本）
void PrintFlagsDword(DWORD bitfield, const FLAG_DESC* flags, size_t count);

// 打印位字段中设置的标志（WORD 版本）
void PrintFlagsWord(WORD bitfield, const FLAG_DESC_WORD* flags, size_t count);

// 安全提取节名称（最多 8 个字符，以空字符结尾）
void GetSectionName(PIMAGE_SECTION_HEADER pSection, CHAR outName[9]);

// 验证 PE 文件是否已加载且有效
BOOL IsPeLoaded(const PE_CONTEXT* ctx);

// 验证 PE 是否具有特定数据目录（非零 RVA 和 Size）
BOOL HasDataDirectory(const PE_CONTEXT* ctx, DWORD dirIndex);

// ============================================================================
// 32/64 位感知的数据目录访问
// ============================================================================

// 获取 DataDirectory 条目的指针，同时处理 PE32 和 PE32+
// 如果索引超出范围则返回 NULL
PIMAGE_DATA_DIRECTORY GetDataDirectory(const PE_CONTEXT* ctx, DWORD dirIndex);

// 获取 ImageBase 值（处理 32/64 位）
ULONGLONG GetImageBase(const PE_CONTEXT* ctx);

// 获取 SizeOfImage 值（处理 32/64 位）
DWORD GetSizeOfImage(const PE_CONTEXT* ctx);

#endif // PE_COMMON_H
