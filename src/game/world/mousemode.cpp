#include "mousemode.h"

#include <cstdint>

#include "../math/x87.h"
#include "mapcell.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const MouseModeEnv kGameEnv = {reinterpret_cast<void(__cdecl *)(void *)>(0x0067acb0)};
const MouseModeEnv *g_env = &kGameEnv;

}  // namespace

void MouseModes_SetEnv(const MouseModeEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl MouseModes_InitUnusedOne() { *reinterpret_cast<float *>(0x007b05a0) = 1.0f; }

void __cdecl MouseModes_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x007b05a4) = 1.0f / 8388608.0f; }

void __cdecl MouseModes_InitTile() {
    *reinterpret_cast<u16 *>(0x007b05cc) = 1;
    *reinterpret_cast<u32 *>(0x007b05d0) = 0;
}

void __cdecl MouseState_SetTileFromPosition(const float *position) {
    u8 *const map = *reinterpret_cast<u8 **>(0x007a0854);
    if (!map) return;
    u8 *const grid = at<u8 *>(map, 0x6c);
    if (!grid) return;
    s32 x = (s32)(long long)((X87Reg(position[0]) - at<float>(grid, 8)) / at<float>(grid, 0x10)).v;  // __ftol
    s32 z = (s32)(long long)((X87Reg(position[2]) - at<float>(grid, 0xc)) / at<float>(grid, 0x14)).v;
    if (x < 0) x = 0;
    if (z < 0) z = 0;
    if ((u32)x >= at<u32>(grid, 0x18)) x = at<u32>(grid, 0x18) - 1;
    if ((u32)z >= at<u32>(grid, 0x1c)) z = at<u32>(grid, 0x1c) - 1;
    if (x < 0) x = 0;
    if (z < 0) z = 0;
    const s32 width = *reinterpret_cast<const s32 *>(0x007a0314), depth = *reinterpret_cast<const s32 *>(0x007a0374);
    if (x >= width) x = width - 1;
    if (z >= depth) z = depth - 1;
    *reinterpret_cast<u16 *>(0x007b05cc) = Map_InBounds(x, z) ? (u16)((z << 7) + x + 1) : 0;
}

void __fastcall MouseState_NewFrame(u8 *self) {
    const u32 count = at<u32>(self, 0xc);
    at<u32>(self, 0) = 0;
    at<u32>(self, 0xc) = count + 1;
}

void __fastcall MouseMode_Nop4(void *, void *, u32, u32, u32, u32) {}

void __fastcall MouseMode_Nop2(void *, void *, u32, u32) {}

void __fastcall MouseMode_Dtor(u8 *self) { at<u32>(self, 0) = 0x006fe9b0; }

u8 *__fastcall MouseMode_DeletingDtor(u8 *self, void *, u32 flags) {
    at<u32>(self, 0) = 0x006fe9b0;
    if (flags & 1) g_env->free(self);
    return self;
}

u8 *__fastcall MouseMode_DerivedDeletingDtor(u8 *self, void *, u32 flags) {
    MouseMode_Dtor(self);
    if (flags & 1) g_env->free(self);
    return self;
}

u8 *__fastcall MouseModeTile_Ctor(u8 *self, void *, u32 arg) {
    at<u32>(self, 0) = 0x006fe9e0;
    at<u32>(self, 4) = 1;
    at<u32>(self, 8) = arg;
    at<u32>(self, 0xc) = 0;
    return self;
}

u8 *__fastcall MouseModeTile_CtorKind(u8 *self, void *, u32 arg, u16 kind) {
    at<u32>(self, 4) = 1;
    at<u32>(self, 0xc) = 1;
    at<u32>(self, 0) = 0x006fe9e0;
    at<u32>(self, 8) = arg;
    at<u16>(self, 0x10) = kind;
    return self;
}

u8 *__fastcall MouseModeOnlineBase_Ctor(u8 *self) {
    at<u32>(self, 0) = 0x006fea10;
    at<u32>(self, 4) = 1;
    return self;
}

u8 *__fastcall MMPlaceStaff_Ctor(u8 *self, void *, u32 a, u8 b, u8 c, u32 d) {
    at<u32>(self, 0xc) = a;
    at<u8>(self, 0x10) = b;
    at<u8>(self, 0x11) = c;
    at<u32>(self, 4) = 0;
    at<u32>(self, 8) = 0;
    at<u32>(self, 0x14) = d;
    at<u8>(self, 0x18) = 0;
    at<u32>(self, 0) = 0x006fea40;
    return self;
}

u8 *__fastcall MMStaffDrop_Ctor(u8 *self, void *, u32 a, u8 b) {
    at<u32>(self, 4) = 0;
    at<u32>(self, 8) = 0;
    at<u32>(self, 0) = 0x006fea70;
    at<u32>(self, 0xc) = a;
    at<u8>(self, 0x10) = b;
    return self;
}

u8 *__fastcall MouseMode6feaa0_Ctor(u8 *self) {
    at<u32>(self, 4) = 0;
    at<u32>(self, 0) = 0x006feaa0;
    return self;
}

u8 *__fastcall MouseMode6fead0_Ctor(u8 *self) {
    at<u32>(self, 4) = 0;
    at<u32>(self, 0) = 0x006fead0;
    return self;
}
}
