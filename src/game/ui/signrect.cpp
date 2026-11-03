// The sign over a window's rectangle: see signrect.h.
#include "signrect.h"

#include <cstdint>

#include "../coaster/orient.h"
#include "../sprite/screenmatrix.h"

namespace {

const SignRectEnv kGameEnv = {reinterpret_cast<void(__cdecl *)(u32)>(0x004690a0), reinterpret_cast<u32(__cdecl *)(float, float, float, float, float, u32)>(0x004689f0), &RideObject_Swing};
const SignRectEnv *g_env = &kGameEnv;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 word(const u8 *rect, u32 offset) { return *reinterpret_cast<const s16 *>(rect + offset); }

}  // namespace

void SignRect_SetEnv(const SignRectEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Sign_PlaceOverRect(const u8 *rect, u32 type, s32 row) {
    const float scaleX = global<float>(0x006ff154), scaleY = global<float>(0x006ff150);
    const float x0 = static_cast<float>(word(rect, 4) - 0x400) * scaleX;
    const float x1 = static_cast<float>(word(rect, 8) - 0x400) * scaleX;
    const float y0 = static_cast<float>(0x300 - word(rect, 6)) * scaleY;
    const float y1 = static_cast<float>(0x300 - word(rect, 0xa)) * scaleY;
    const float height = static_cast<float>(Sprite_FlipY(row));
    u32 &sign = global<u32>(0x007b9f4c);
    if (sign) g_env->destroy(sign);
    sign = g_env->create(x0, y1, height, x1 - x0, y0 - y1, type);
}

void __cdecl Sign_Swing() {
    u8 *const sign = global<u8 *>(0x007b9f4c);
    if (sign) g_env->swing(sign);
}
