# UPX Unpacking Technical Analysis

## Target: Windows Minesweeper (saolei.exe)

### Shell Identification

| Tool | Original (saolei.exe) | Packed (ke_saolei.exe) |
|------|----------------------|------------------------|
| **DIE** | Microsoft Visual C++ | UPX(3.05)[-] |
| **Section Names** | .text / .data / .rsrc | UPX0 / UPX1 / .rsrc |
| **File Size** | 119,808 bytes | 97,280 bytes (compressed) |

---

## Unpacking Method: ESP Law

### Principle

UPX stub starts with `pushad` (saves all registers) and ends with `popad` (restores all registers). The `pushad` pushes registers onto the stack; by setting a **hardware access breakpoint** on the ESP value right after `pushad`, execution breaks at the corresponding `popad`, just before the jump to OEP.

### Steps

```
1. Load ke_saolei.exe in x64dbg
2. Entry point: pushad        → typical UPX signature
3. Step over (F8)             → ESP now points to saved registers
4. Hardware BP on [ESP]       → DWord, Access
5. Run (F9)                   → break on popad
6. Step through               → jmp to OEP at 0x01003E21
```

**Result: OEP = 0x00003E21 (RVA) — matches original entry point** ✅

---

## PE Header Comparison (Three-Way)

### Basic Info

| Field | Original | Packed | Unpacked |
|-------|----------|--------|----------|
| **Machine** | x86 (0x014C) | x86 (0x014C) | x86 (0x014C) |
| **Sections** | 3 | 3 | 4 |
| **EntryPoint RVA** | 0x00003E21 | 0x00021CA0 | **0x00003E21** ✅ |
| **ImageBase** | 0x01000000 | 0x01000000 | **0x01000000** ✅ |
| **Subsystem** | GUI (0x0002) | GUI (0x0002) | GUI (0x0002) ✅ |
| **LinkerVersion** | 7.0 | 7.0 | 7.0 ✅ |
| **FileAlignment** | 0x200 | 0x200 | 0x200 ✅ |
| **SectionAlignment** | 0x1000 | 0x1000 | 0x1000 ✅ |

### Optional Header Details

| Field | Original | Packed | Unpacked |
|-------|----------|--------|----------|
| SizeOfCode | 0x00003C00 | - | 0x00011000 |
| SizeOfInitData | 0x00019400 | - | 0x00007000 |
| SizeOfUninitData | 0x00000000 | - | 0x00010000 |
| BaseOfCode | 0x00001000 | - | 0x00011000 |
| BaseOfData | 0x00005000 | - | 0x00022000 |
| SizeOfImage | 0x00020000 | 0x00029000 | 0x0002A000 |
| SizeOfHeaders | - | - | 0x00000400 |

### Section Table

#### Original (saolei.exe)

| # | Name | VirtSize | VirtAddr | RawSize | RawOffset | Characteristics |
|---|------|----------|----------|---------|-----------|-----------------|
| 0 | `.text` | - | 0x00001000 | - | - | CODE+EXECUTE+READ |
| 1 | `.data` | - | 0x00005000 | - | - | INIT_DATA+READ+WRITE |
| 2 | `.rsrc` | - | 0x00006000 | - | - | INIT_DATA+READ |

*Total sections: 3 | Total virtual size: 0x1D726 (120,614) | Total raw size: 0x1D000 (118,784)*

#### Packed (ke_saolei.exe)

| # | Name | VirtSize | VirtAddr | RawSize | RawOffset | Characteristics |
|---|------|----------|----------|---------|-----------|-----------------|
| 0 | `UPX0` | - | 0x00001000 | - | - | UNINIT_DATA+EXECUTE+READ+WRITE |
| 1 | `UPX1` | - | 0x00011000 | - | - | INIT_DATA+EXECUTE+READ+WRITE |
| 2 | `.rsrc` | - | 0x00022000 | - | - | INIT_DATA+READ+WRITE |

*Total sections: 3 | Total raw size: 0x17C00 (97,280)*

#### Unpacked (ke_saolei_dump_SCY.exe) — After Fix

| # | Name | VirtSize | VirtAddr | RawSize | RawOffset | Characteristics |
|---|------|----------|----------|---------|-----------|-----------------|
| 0 | `UPX0` → `.text` | 0x00010000 | 0x00001000 | 0x00010000 | 0x00000400 | CODE+UNINIT+EXECUTE+READ+WRITE |
| 1 | `UPX1` → `.rdata` | 0x00011000 | 0x00011000 | 0x00011000 | 0x00010400 | INIT_DATA+EXECUTE+READ+WRITE |
| 2 | `.rsrc` | 0x00007000 | 0x00022000 | 0x00006C00 | 0x00021400 | INIT_DATA+READ+WRITE |
| 3 | `.SCY` 🆕 | 0x00001000 | 0x00029000 | 0x00000A00 | 0x00028000 | CODE+INIT_DATA+EXECUTE+READ+WRITE |

*Total sections: 4 | Total virtual size: 0x29000 (167,936) | Total raw size: 0x28A00 (166,400)*

> ⚠️ Section names were manually restored from `UPX0/UPX1` to `.text/.rdata` using CFF Explorer.
> The `.SCY` section is added by Scylla for relocated IAT entries.

---

## IAT Analysis

### Packed Program Issues

The packed program has IAT parse errors:
- RVA to FOA conversion fails for import table entries
- Because UPX sections store data compressed; IAT only exists in-memory after decompression

### Unpacked Program

After Scylla repair:

| DLL | Functions | Status |
|-----|-----------|--------|
| KERNEL32.dll | ~20 | ⚠️ Ordinal works, name RVA failed |
| USER32.dll | ~46 | ⚠️ Ordinal works, name RVA failed |
| GDI32.dll | ~6 | ⚠️ Ordinal works, name RVA failed |
| ADVAPI32.dll | ~3 | ⚠️ Ordinal works, name RVA failed |
| COMCTL32.dll | ~2 | ⚠️ Ordinal works, name RVA failed |
| SHELL32.dll | ~56 | ⚠️ Ordinal works, name RVA failed |
| WINMM.dll | ~1 | ⚠️ Ordinal works, name RVA failed |
| MSVCRT.dll | ~16 | ⚠️ Ordinal works, name RVA failed |

> ⚠️ The "(名称RVA转换失败)" errors mean the PE parser cannot resolve function name strings through the import name RVA chain. However, the **ordinal-based imports are intact**, which is sufficient for the program to run correctly.

---

## Data Directory Comparison

| Directory | Original (RVA/Size) | Packed (RVA/Size) | Unpacked (RVA/Size) |
|-----------|-------------------|-------------------|---------------------|
| Import | 0x0000415C / 0xB4 | 0x00028850 / 0x1F0 | 0x000291B8 / 0xB4 |
| Resource | 0x00006000 / 0x18ED4 | 0x00022000 / 0x6850 | 0x00022000 / 0x6850 |
| Debug | 0x000011D0 / 0x1C | — | — |
| Bound Import | 0x00000248 / 0xA8 | — | — |
| IAT | 0x00001000 / 0x1B8 | — | — |

---

## Changelog

### What Was Done (Step by Step)

1. ✅ **DIE Analysis** — Confirmed UPX 3.05 shell on packed binary
2. ✅ **ESP Law OEP** — Found OEP at 0x00003E21 via hardware access breakpoint
3. ✅ **Memory Dump** — Scylla dumped process memory at OEP
4. ✅ **IAT Repair** — Scylla IAT Autosearch + Get Imports + Fix Dump
5. ✅ **Section Rename** — CFF Explorer: UPX0→.text, UPX1→.rdata
6. ✅ **Run Test** — Unpacked program launches and operates normally

### Residual Issues (Cosmetic, Non-Functional)

| Issue | Cause | Impact |
|-------|-------|--------|
| File larger (162 KB vs 117 KB) | Raw section sizes inflated; extra Scylla section | None |
| Extra `.SCY` section | Scylla creates this for relocated IAT | None |
| IAT name resolution fails | Scylla rebuilds IAT with ordinals | None (ordinals work) |
| Section flags differ | Memory dump inherits runtime permissions | None |

---

## Key Learnings

1. **ESP law is the gold standard** for simple compression shells like UPX/ASPack
2. **Scylla IAT repair** is not byte-perfect — it creates a working import table but not an identical one
3. **Section name preservation** matters for tools like DIE — rename UPX0/UPX1 back for clean detection
4. **PE reconstruction** from a memory dump always introduces structural differences vs. the original compiled binary

---

## References

- [UPX: the Ultimate Packer for eXecutables](https://upx.github.io/)
- [x64dbg Debugger](https://x64dbg.com/)
- [Scylla - Imports Reconstruction](https://github.com/NtQuery/Scylla)
- [PE Format Specification (Microsoft)](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format)
