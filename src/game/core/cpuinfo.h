// The CPU probe (CpuInfo_Ctor 0x55e470), SystemInfo's base: without EFLAGS.ID (no CPUID) it keeps the
// vendor "AnonymousCPU" and zero features; else it asks CPUID leaf 0 for the vendor, leaf 1 for the
// signature and feature flags, leaf 0x80000001 for the extended flags (bit 31, 3DNow!), and sets
// hasTsc for family 5 or later (the family is taken as 4 when the signature is 0) with the TSC bit
// (features bit 4).
//
// The originals' quirks are kept:
// - Leaves 1 and 0x80000001 are asked whatever leaf 0's highest leaf is.
// - The default vendor's 13 bytes are copied over +0x10..+0x1c, one byte past the 0x1c the
//   system-info object gives it (its constructor zeroes that byte's dword afterwards); CPUID's
//   vendor overwrites only +0x10..+0x1b.
// - has3dNow (+0xc) is left as it was when there is no CPUID.
#pragma once

#include "../../../re/types.h"

struct CpuInfo {
    u32 signature;  // CPUID 1 EAX
    u32 features;   // CPUID 1 EDX
    u32 hasTsc;     // family >= 5 and features bit 4
    u32 has3dNow;   // CPUID 0x80000001 EDX bit 31
    char vendor[0xd];  // "AnonymousCPU", or CPUID 0's EBX EDX ECX
};

struct CpuInfoEnv {
    u32(__cdecl *hasCpuid)();                  // EFLAGS.ID can be toggled
    void(__cdecl *cpuid)(u32 leaf, u32 *regs);  // EAX EBX ECX EDX
};
// nullptr restores the host's (the instructions themselves).
void CpuInfo_SetEnv(const CpuInfoEnv *e);
const CpuInfoEnv *CpuInfo_GetEnv();

extern "C" {
// 0x55e470 (thiscall, no return value).
void __fastcall CpuInfo_Ctor(CpuInfo *c);
}
