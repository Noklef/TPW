// The sign over a window's rectangle: reimplementation of 0x486410 and 0x4864f0 (names invented, unconfirmed).
#pragma once

#include "../../../re/types.h"

// Sign_PlaceOverRect's calls (0x4690a0, agent B's, by address by default, the ports Sign_Create and RideObject_Swing; the bench's recorders: their own benches have them).
struct SignRectEnv {
    void(__cdecl *destroy)(u32 sign);                                                     // 0x4690a0
    u32(__cdecl *create)(float x, float y, float height, float width, float depth, u32 type);   // 0x4689f0 (Sign_Create in agent B's world/sign)
    void(__cdecl *swing)(u8 *object);                                                     // RideObject_Swing
};
void SignRect_SetEnv(const SignRectEnv *env);

extern "C" {

// 0x486410 (cdecl (rect, type, row); name invented, unconfirmed): the rectangle's signed words +4, +6, +8, +0xa as x0, y0, x1, y1 on a frame about (0x400, 0x300), x by the float [0x6ff154], y upwards by the float [0x6ff150]; the height Sprite_FlipY of row. A sign at [0x7b9f4c]: 0x4690a0 of it.
// [0x7b9f4c] = 0x4689f0 (x0, y1, height, x1 - x0, y0 - y1, type).
void __cdecl Sign_PlaceOverRect(const u8 *rect, u32 type, s32 row);

// 0x4864f0 (cdecl): RideObject_Swing of the sign [0x7b9f4c], if any.
void __cdecl Sign_Swing();

// 0x486510 (cdecl): the sign [0x7b9f4c], if any, destroyed (0x4690a0) and forgotten.
void __cdecl Sign_Destroy();
}
