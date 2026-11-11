// The DCT video's macroblock: see dctmacroblock.h.
#include "dctmacroblock.h"

#include <cstdint>

#include "dctvlc.h"
#include "movieidct.h"
#include "refblock.h"
#include "tqivideo.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const DctMacroblockEnv kGameEnv = {&DctVlc_DecodeBlock, &DctIdct_Block, &TqiVideo_BlockBytes32};
const DctMacroblockEnv *g_env = &kGameEnv;

void decoded(u32 buffer, s32 pitch) {
    s32 *const out = reinterpret_cast<s32 *>((uintptr_t)buffer);
    if (g_env->decode() == 1) DctVlc_FillDc(out, pitch);
    else g_env->idct(out, pitch);
}

}  // namespace

void DctMacroblock_SetEnv(const DctMacroblockEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall DctVideo_DecodeMacroblock(u8 *reference, u8 *out, s32 pitch) {
    static const u32 kLuma[4] = {(u32)(uintptr_t)&gDctmacroblock_faa860, ((u32)(uintptr_t)&gDctmacroblock_faa860 + 0x20), ((u32)(uintptr_t)&gDctmacroblock_faa860 + 0x200), ((u32)(uintptr_t)&gDctmacroblock_faa860 + 0x220)};
    const s32 half = pitch >> 1;
    u32 mode = 0;
    s32 swapped = 0;
    if (global<u32>((u32)(uintptr_t)&gDctvlc_faac60)) {
        const u32 bits = gDctvlc_fbd650;
        if (!(bits & 0xc0000000)) {
            DctVlc_Skip(2);
        } else {
            if (bits & 0x80000000) {
                mode = 0x3ff;
                DctVlc_Skip(1);
            } else {
                mode = bits >> 24;
                DctVlc_Skip(8);
            }
            const s32 x = DctVlc_ReadCode();
            const s32 y = DctVlc_ReadCode();
            reference += (y * half + (x >> 1)) * 4;
            swapped = x & 1;
        }
    }
    for (u32 i = 0; i < 4; ++i) {
        if (mode & (1u << i)) {
            const s32 bias = DctVlc_ReadCode() - 0x40;
            RefBlock_Copy16(reference + ((i & 2) ? half << 5 : 0) + ((i & 1) ? 0x10 : 0), swapped, half, reinterpret_cast<u8 *>((uintptr_t)kLuma[i]), static_cast<u8>(bias));
        } else {
            decoded(kLuma[i], 0x10);
        }
    }
    for (u32 i = 0; i < 2; ++i) {
        const u32 buffer = i ? (u32)(uintptr_t)&gDctmacroblock_faa760 : (u32)(uintptr_t)&gDctmacroblock_faa660;
        if (mode & (0x10u << i)) {
            const s32 bias = DctVlc_ReadCode() - 0x40;
            RefBlock_Copy8((s32)(uintptr_t)reference, i ? 0 : 1, half, reinterpret_cast<u8 *>((uintptr_t)buffer), static_cast<u8>(bias));
        } else {
            decoded(buffer, 8);
        }
    }
    g_env->bytes(gDctmacroblock_faa860, gDctmacroblock_faa660, reinterpret_cast<u32 *>(out), half * 4);
}
