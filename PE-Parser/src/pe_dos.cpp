#include "pe_common.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// ShowDosHeader - 显示 DOS 头字段
// ============================================================================
void ShowDosHeader(const PE_CONTEXT* ctx)
{
    if (!IsPeLoaded(ctx)) return;

    if (ctx->pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        PRINT_ERROR("错误 -> 无效的 DOS 签名 (期望: 0x5A4D, 实际: 0x%04X)\r\n",
            ctx->pDosHeader->e_magic);
        return;
    }

    PRINT_TITLE("\n\n==== DOS 头信息 ====\n\n");

    PIMAGE_DOS_HEADER pDos = ctx->pDosHeader;

    // 定义所有 DOS 头字段，包含偏移、名称和格式
    // e_magic 以红色高亮显示（错误颜色），因为它是关键的字段
    PRINT_ERROR("  0000h    e_magic       ->  0x%04X        // DOS 签名 (\"MZ\" = 0x5A4D)\r\n", pDos->e_magic);
    PRINT_INFO("  0002h    e_cblp        ->  0x%04X        // 文件最后一页的字节数\r\n", pDos->e_cblp);
    PRINT_INFO("  0004h    e_cp          ->  0x%04X        // 文件的页数\r\n", pDos->e_cp);
    PRINT_INFO("  0006h    e_crlc        ->  0x%04X        // 重定位数量\r\n", pDos->e_crlc);
    PRINT_INFO("  0008h    e_cparhdr     ->  0x%04X        // 以段落为单位的头大小\r\n", pDos->e_cparhdr);
    PRINT_INFO("  000Ah    e_minalloc    ->  0x%04X        // 最小额外段落数\r\n", pDos->e_minalloc);
    PRINT_INFO("  000Ch    e_maxalloc    ->  0x%04X        // 最大额外段落数\r\n", pDos->e_maxalloc);
    PRINT_INFO("  000Eh    e_ss          ->  0x%04X        // 初始 SS 寄存器值\r\n", pDos->e_ss);
    PRINT_INFO("  0010h    e_sp          ->  0x%04X        // 初始 SP 寄存器值\r\n", pDos->e_sp);
    PRINT_INFO("  0012h    e_csum        ->  0x%04X        // 校验和\r\n", pDos->e_csum);
    PRINT_INFO("  0014h    e_ip          ->  0x%04X        // 初始 IP 寄存器值\r\n", pDos->e_ip);
    PRINT_INFO("  0016h    e_cs          ->  0x%04X        // 初始 CS 寄存器值\r\n", pDos->e_cs);
    PRINT_INFO("  0018h    e_lfarlc      ->  0x%04X        // 重定位表文件偏移\r\n", pDos->e_lfarlc);
    PRINT_INFO("  001Ah    e_ovno        ->  0x%04X        // 覆盖编号\r\n", pDos->e_ovno);

    // 保留字 e_res[4] 位于偏移 0x1C - 0x23
    for (int i = 0; i < 4; i++)
    {
        PRINT_INFO("  %04Xh    e_res[%d]     ->  0x%04X        // 保留\r\n",
            0x001C + i * 2, i, pDos->e_res[i]);
    }

    PRINT_INFO("  0024h    e_oemid       ->  0x%04X        // OEM 标识符\r\n", pDos->e_oemid);
    PRINT_INFO("  0026h    e_oeminfo     ->  0x%04X        // OEM 信息\r\n", pDos->e_oeminfo);

    // 保留字 e_res2[10] 位于偏移 0x28 - 0x3B
    for (int i = 0; i < 10; i++)
    {
        PRINT_INFO("  %04Xh    e_res2[%d]    ->  0x%04X        // 保留\r\n",
            0x0028 + i * 2, i, pDos->e_res2[i]);
    }

    // e_lfanew 是最重要的字段 - 以红色高亮显示
    PRINT_ERROR("  003Ch    e_lfanew      ->  0x%08X        // NT 头文件偏移\r\n\n", pDos->e_lfanew);
}
