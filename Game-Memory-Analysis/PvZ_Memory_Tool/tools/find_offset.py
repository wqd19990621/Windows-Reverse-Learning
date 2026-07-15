#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
find_offset.py - 植物大战僵尸内存特征码搜索工具

使用 pymem 库搜索游戏内存中的特征码（AoB / Array of Bytes），
辅助定位偏移地址，当游戏版本更新导致硬编码地址失效时使用。

依赖: pymem (pip install pymem)

用法:
    python find_offset.py                    # 搜索预定义特征码
    python find_offset.py --pattern "7E 14"  # 搜索自定义特征码
    python find_offset.py --dump 0x400000 0x1000  # 导出内存区域为十六进制
"""

import sys
import struct
from typing import Optional, List, Tuple

try:
    import pymem
    import pymem.process
except ImportError:
    print("[!] 请先安装 pymem: pip install pymem")
    sys.exit(1)

# ============================================================================
# 预定义特征码 (Sig / Pattern)
# ============================================================================
SIGNATURES = {
    # 植物冷却CD: jle 4875FC -> 7E 14
    "plant_cd_jle": {
        "pattern": b"\x7E\x14",
        "description": "植物冷却CD检查 (jle short)",
        "expected_addr": 0x004875E6,
        "module": "PlantsVsZombies.exe",
    },
    # 大嘴花CD: jne +0x5F -> 75 5F
    "chomper_cd_jne": {
        "pattern": b"\x75\x5F",
        "description": "大嘴花吞噬CD检查 (jne short)",
        "expected_addr": 0x004616E5,
        "module": "PlantsVsZombies.exe",
    },
    # 植物无敌: add [esi+40], -04 -> 83 46 40 FC
    "plant_invincible_add": {
        "pattern": b"\x83\x46\x40\xFC",
        "description": "植物扣血指令 (add [esi+40], -04)",
        "expected_addr": 0x00530040,
        "module": "PlantsVsZombies.exe",
    },
    # 阳光基址引用 (mov reg, [6A9EC0])
    # 特征: C7 05 ?? ?? ?? ?? C0 9E 6A 00 (部分)
    "sunlight_base_ref": {
        "pattern": b"\xC0\x9E\x6A\x00",
        "description": "阳光全局指针引用 (0x6A9EC0)",
        "expected_addr": None,  # 多处引用
        "module": "PlantsVsZombies.exe",
    },
}


def find_process(process_name: str = "PlantsVsZombies.exe") -> Optional[pymem.Pymem]:
    """查找游戏进程"""
    try:
        pm = pymem.Pymem(process_name)
        print(f"[+] 已连接到进程: {process_name} (PID: {pm.process_id})")
        return pm
    except pymem.exception.ProcessNotFound:
        print(f"[!] 未找到进程: {process_name}")
        return None


def pattern_scan(pm: pymem.Pymem, pattern: bytes, module_name: str) -> List[int]:
    """在指定模块中搜索特征码，返回所有匹配地址"""
    module = pymem.process.module_from_name(pm.process_handle, module_name)
    if not module:
        print(f"[!] 未找到模块: {module_name}")
        return []

    base = module.lpBaseOfDll
    size = module.SizeOfImage

    print(f"[*] 扫描模块: {module_name}")
    print(f"    基址: 0x{base:08X}, 大小: 0x{size:X}")

    # 读取整个模块内存
    try:
        data = pm.read_bytes(base, size)
    except Exception as e:
        print(f"[!] 读取模块内存失败: {e}")
        return []

    # 搜索所有匹配
    results = []
    offset = 0
    while True:
        pos = data.find(pattern, offset)
        if pos == -1:
            break
        addr = base + pos
        results.append(addr)
        print(f"    [+] 匹配 @ 0x{addr:08X} (偏移 0x{pos:X})")
        offset = pos + 1

    return results


def dump_memory(pm: pymem.Pymem, address: int, size: int, filename: str = "memory_dump.bin"):
    """导出指定内存区域"""
    try:
        data = pm.read_bytes(address, size)
        with open(filename, "wb") as f:
            f.write(data)
        print(f"[+] 内存已导出到: {filename} ({size} 字节)")

        # 同时输出十六进制预览
        print(f"\n十六进制预览 (前256字节):\n")
        for i in range(0, min(256, size), 16):
            hex_str = " ".join(f"{b:02X}" for b in data[i:i+16])
            ascii_str = "".join(chr(b) if 32 <= b < 127 else "." for b in data[i:i+16])
            print(f"  0x{address+i:08X}:  {hex_str:<48}  {ascii_str}")
    except Exception as e:
        print(f"[!] 导出内存失败: {e}")


def main():
    import argparse

    parser = argparse.ArgumentParser(description="植物大战僵尸内存特征码搜索工具")
    parser.add_argument("--pattern", type=str, help="自定义特征码 (十六进制字符串, 如 '7E 14')")
    parser.add_argument("--module", type=str, default="PlantsVsZombies.exe", help="目标模块名")
    parser.add_argument("--dump", type=str, nargs=2, metavar=("ADDR", "SIZE"),
                        help="导出内存区域 (十六进制地址和大小)")
    parser.add_argument("--all", action="store_true", help="搜索所有预定义特征码")
    args = parser.parse_args()

    # 连接进程
    pm = find_process()
    if not pm:
        return

    if args.dump:
        addr = int(args.dump[0], 16)
        size = int(args.dump[1], 16)
        dump_memory(pm, addr, size)
        return

    if args.pattern:
        # 自定义特征码
        hex_str = args.pattern.replace(" ", "")
        pattern = bytes.fromhex(hex_str)
        print(f"[*] 搜索自定义特征码: {args.pattern}")
        pattern_scan(pm, pattern, args.module)
        return

    # 搜索所有预定义特征码
    print("\n" + "=" * 60)
    print("  植物大战僵尸 - 内存特征码扫描")
    print("=" * 60 + "\n")

    for name, sig in SIGNATURES.items():
        print(f"\n--- {name}: {sig['description']} ---")
        results = pattern_scan(pm, sig["pattern"], sig["module"])
        if sig["expected_addr"] and results:
            matches = [a for a in results if a == sig["expected_addr"]]
            if matches:
                print(f"    [✓] 预期地址 0x{sig['expected_addr']:08X} 已确认")
            else:
                print(f"    [!] 预期地址 0x{sig['expected_addr']:08X} 未匹配，偏移可能已变化")
                print(f"    [*] 可能的候选地址: {[f'0x{a:08X}' for a in results]}")

    print("\n" + "=" * 60)
    print("  扫描完成")
    print("=" * 60)


if __name__ == "__main__":
    main()
