// A map cell's sprite effect started: see celleffect.h.
#include "celleffect.h"

#include <cstdint>

#include "../core/options.h"
#include "../math/x87.h"
#include "../render/particles.h"
#include "../sprite/script.h"
#include "mapcell.h"
#include "random.h"
#include "../ui/dialog74fb20.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const CellEffectEnv kGameEnv = {Setting_ReadMode, GameAddress<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0), Map_SpreadAdd, SpriteScript_Start, World_SetSeed, World_Rand, SpriteScript_SetPosition, Particle_Spawn};
const CellEffectEnv *g_env = &kGameEnv;

u32 scriptOf(u32 kind, u32 word) {
    if (kind >= 1 && kind <= 5) return SpriteCode(0x1598) + 0x28 * (kind - 1);
    if (kind == 7 || kind == 8) {
        g_env->spread(global<u8 *>(0x008023a0), nullptr, kind == 7 ? 2 : 5, word);
        return kind == 7 ? SpriteCode(0x1688) : SpriteCode(0x16b0);
    }
    return SpriteCode(0x1660);
}

// (fild of the rand % 9, fsub -1, fild of the cell's coordinate, fmul 10, faddp, fstp float.)
float position(u32 coordinate) {
    const u32 step = (u32)g_env->rand(global<u8 *>(0x0080239c)) % 9;
    return (float)(X87Reg((double)step + 1.0) + X87Reg(X87_MulInt(coordinate, 10.0f)));
}

}  // namespace

void CellEffect_SetEnv(const CellEffectEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __fastcall MapCell_StartEffect(u8 *cell, void *, u32 kind) {
    u8 &guard = gCellEffect_fb34b9;
    if (!(guard & 1)) {
        guard |= 1;
        g_env->readMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (gDialog74fb20_fb3b7c == 1 || at<u32>(cell, 0x2c)) return;
    const u32 script = scriptOf(kind, at<u16>(cell, 0));
    at<s32>(cell, 0x28) = g_env->start(script, 0xb, 0, 0, 0, 0, 0);
    g_env->setSeed(global<u8 *>(0x0080239c), nullptr, at<u16>(cell, 0));
    const u32 x = (at<u16>(cell, 0) - 1u) & 0x7f;
    const float placeX = position(x);
    const u32 y = (at<u16>(cell, 0) - 1u) >> 7;
    const float placeY = position(y);
    g_env->setPosition(at<s32>(cell, 0x28), *reinterpret_cast<const u32 *>(&placeX), 0, *reinterpret_cast<const u32 *>(&placeY), 0);
    at<u16>(cell, 0x30) = 0;
    at<u32>(cell, 0x2c) = kind;
    if (kind == 8) {
        const s32 z = (s32)(long long)(X87Reg((double)placeY) * X87Reg(1024.0)).v;
        const s32 across = (s32)(long long)(X87Reg((double)placeX) * X87Reg(1024.0)).v;
        at<s32>(cell, 0x38) = g_env->spawn(0x32, across, 0, z);
    }
}
