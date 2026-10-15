#include "mousemode.h"

#include <cstdint>

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const MouseModeEnv kGameEnv = {reinterpret_cast<void(__cdecl *)(void *)>(0x0067acb0)};
const MouseModeEnv *g_env = &kGameEnv;

}  // namespace

void MouseModes_SetEnv(const MouseModeEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

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
