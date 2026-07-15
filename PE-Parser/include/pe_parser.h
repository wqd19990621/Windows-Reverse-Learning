#ifndef PE_PARSER_H
#define PE_PARSER_H

#include "pe_common.h"

// ============================================================================
// PE 文件加载器
// ============================================================================

// 从磁盘加载 PE 文件到上下文中。成功返回 TRUE。
BOOL LoadPeFile(PE_CONTEXT* ctx, CONST CHAR* filePath);

// 释放与 PE 上下文关联的所有资源
void FreePeFile(PE_CONTEXT* ctx);

// ============================================================================
// 地址转换 (RVA <-> FOA)
// ============================================================================

// 将相对虚拟地址转换为文件偏移地址
DWORD RvaToFoa(const PE_CONTEXT* ctx, DWORD dwRva);

// 将文件偏移地址转换为相对虚拟地址
DWORD FoaToRva(const PE_CONTEXT* ctx, DWORD dwFoa);

// 查找包含给定 RVA 的节。返回节索引或 -1。
int FindSectionByRva(const PE_CONTEXT* ctx, DWORD dwRva);

// ============================================================================
// 显示函数 - 打印 PE 结构信息
// ============================================================================

void ShowPeInfo(const PE_CONTEXT* ctx);
void ShowDosHeader(const PE_CONTEXT* ctx);
void ShowNtHeader(const PE_CONTEXT* ctx);
void ShowSections(const PE_CONTEXT* ctx);
void ShowImports(const PE_CONTEXT* ctx);
void ShowExports(const PE_CONTEXT* ctx);
void ShowRelocations(const PE_CONTEXT* ctx);

// ============================================================================
// 导出函数查找
// ============================================================================

// 按名称查找导出函数 RVA。如果未找到则返回 0。
DWORD FindExportByName(const PE_CONTEXT* ctx, CONST CHAR* funcName);

// 按序号（索引）查找导出函数 RVA。如果未找到则返回 0。
DWORD FindExportByOrdinal(const PE_CONTEXT* ctx, DWORD dwOrdinal);

// ============================================================================
// 交互式地址转换命令
// ============================================================================

void CmdConvertRvaToFoa(const PE_CONTEXT* ctx, CONST CHAR* param);
void CmdConvertFoaToRva(const PE_CONTEXT* ctx, CONST CHAR* param);

#endif // PE_PARSER_H
