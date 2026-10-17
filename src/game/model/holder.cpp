#include "holder.h"

#include <cstdint>

#include "../math/matrix.h"
#include "../render/renderer.h"
#include "../core/vcall.h"
#include "../core/misc.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Holder_IsAwake(const u8 *holder) { return at<u32>(holder, 0) & 1; }

void __cdecl Holder_FlagModel(const u8 *const *ref) { at<u32>(at<const u8 *>(*ref, 4), 0x30) |= 0x200; }

void __cdecl Holder_SetPosition(const u8 *holder, u32 x, u32 y, u32 z) {
    u8 *matrix = at<u8 *>(at<const u8 *>(holder, 4), 0x78) + 0x10;
    at<u32>(matrix, 0x30) = x;
    at<u32>(matrix, 0x34) = y;
    at<u32>(matrix, 0x38) = z;
}

void __cdecl Holder_SetScale(const u8 *holder, const float *scale) {
    float *const m = reinterpret_cast<float *>(at<u8 *>(at<const u8 *>(holder, 4), 0x78) + 0x10);
    Matrix_ScaleAxes(m, scale[0], scale[1], scale[2]);
}

void __cdecl Holder_InitUnusedOne() { at<float>(reinterpret_cast<void *>(0x007a1cac), 0) = 1.0f; }

void __cdecl Holder_InitUnusedEpsilon() { at<float>(reinterpret_cast<void *>(0x007a1cdc), 0) = 1.0f / 8388608.0f; }

u32 __cdecl Ride_SlotKey(u32 ride, u32 i) {
    const u8 *holder = at<const u8 *>(reinterpret_cast<const u8 *const *>(0x007a4610)[ride], 8);
    // Integer arithmetic: the original tests table + i * 0x14 for zero, which pointer
    // arithmetic on a null table would leave undefined.
    const uintptr_t slot = (uintptr_t)at<const u8 *>(at<const u8 *>(holder, 0x28), 4) + i * 0x14;
    return slot ? at<u32>(reinterpret_cast<const void *>(slot), 8) : 0;
}
void __cdecl Holder_Sleep(u8 *holder) {
    u8 *const part = at<u8 *>(holder, 0x1c);
    const u32 flags = at<u32>(holder, 0);
    u8 *key = nullptr;
    if (part)
        key = part + at<s32>(at<u8 *>(part, 0), 4);
    u32 tag;
    if (flags & 0x10)
        tag = 0x10;
    else if (flags & 8)
        tag = 0x20;
    else
        tag = 0x70;
    Renderer_RemoveFromSlot(global<u8 *>(0x0087a6d0), nullptr, (u32)(uintptr_t)key, tag);
    at<u32>(holder, 0) = at<u32>(holder, 0) & 0xffffffe6;
}

u32 __cdecl Animator_Sleep(u8 *animator) {
    Holder_Sleep(at<u8 *>(animator, 0x24));
    at<u32>(animator, 0) = at<u32>(animator, 0) & 0xffffbfff;
    return 1;
}

void __cdecl Holder_Destroy(u8 *holder) {
    if (at<u32>(holder, 0) & 1)
        Holder_Sleep(holder);
    u8 *owner = at<u8 *>(holder, 8);
    if (!owner)
        owner = holder;
    at<s32>(owner, 0x34) = at<s32>(owner, 0x34) - 1;
    if (u8 *part = at<u8 *>(holder, 0x1c)) {
        u8 *adjusted = part + at<s32>(at<u8 *>(part, 0), 4);
        vcall<u32>(adjusted, 0, 1u);
    }
    at<u32>(holder, 0x1c) = 0;
}

void __fastcall HolderUser_Release(u8 *user) {
    if (at<u32>(user, 4) == 0)
        return;
    u8 *const target = global<u8 *>(0x0079fcb4);
    if (at<u32>(target, 4) & 0x4000)
        Obj_RemoveFromRect(at<u8 *>(target, 8), at<u32>(target, 0xc0), at<u32>(target, 0xc4));
    else
        Holder_Sleep(at<u8 *>(target, 8));
    at<u32>(user, 4) = 0;
}

void __cdecl Holder_AddToGrid(u8 *holder, u32 from, u32 to) {
    u8 *part = nullptr;
    if (u8 *const sub = at<u8 *>(holder, 0x1c))
        part = sub + at<s32>(at<u8 *>(sub, 0), 4);
    Grid_AddObject(global<u8 *>(0x0087a6d0), nullptr, (s32)(from >> 16), (s32)(from & 0xffff), (s32)(to >> 16), (s32)(to & 0xffff), part, 0x70);
    at<u32>(holder, 0) = at<u32>(holder, 0) | 1;
}
}
