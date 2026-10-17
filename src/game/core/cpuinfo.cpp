// The CPU probe: see cpuinfo.h.
#include "cpuinfo.h"

#include <cstring>
#include <intrin.h>

namespace {

u32 __cdecl hostHasCpuid() {
    u32 changed;
    __asm {
        pushfd
        pop eax
        mov ecx, eax
        xor eax, 0x200000
        push eax
        popfd
        pushfd
        pop eax
        xor eax, ecx
        mov changed, eax
    }
    return changed != 0;  // (EFLAGS.ID is left toggled, as the original leaves it)
}

void __cdecl hostCpuid(u32 leaf, u32 *regs) {
    int r[4];
    __cpuid(r, static_cast<int>(leaf));
    for (int i = 0; i < 4; ++i) regs[i] = static_cast<u32>(r[i]);
}

const CpuInfoEnv kHostEnv = {&hostHasCpuid, &hostCpuid};
const CpuInfoEnv *g_env = &kHostEnv;

const char kAnonymous[0xd] = "AnonymousCPU";  // 0x7010d8

}  // namespace

void CpuInfo_SetEnv(const CpuInfoEnv *e) { g_env = e ? e : &kHostEnv; }
const CpuInfoEnv *CpuInfo_GetEnv() { return g_env; }

extern "C" {

void __fastcall CpuInfo_Ctor(CpuInfo *c) {
    std::memcpy(c->vendor, kAnonymous, sizeof kAnonymous);
    c->hasTsc = 0;
    c->signature = 0;
    c->features = 0;
    if (!g_env->hasCpuid()) return;
    u32 r[4];
    g_env->cpuid(0, r);
    std::memcpy(c->vendor, &r[1], 4);
    std::memcpy(c->vendor + 4, &r[3], 4);
    std::memcpy(c->vendor + 8, &r[2], 4);
    g_env->cpuid(1, r);
    c->signature = r[0];
    c->features = r[3];
    g_env->cpuid(0x80000001, r);
    c->has3dNow = r[3] >> 31;
    const s32 family = c->signature == 0 ? 4 : (static_cast<s32>(c->signature) >> 8) & 0xf;
    if (family >= 5 && c->features & 0x10) c->hasTsc = 1;
}

}  // extern "C"
