#include "holder.h"

#include <cstdint>

#include "../math/matrix.h"
#include "../render/renderer.h"
#include "../core/vcall.h"
#include "../core/globalmem.h"
#include "objmodel.h"
#include "partstate.h"
#include "release.h"

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

u32 __cdecl Holder_Dispose(u8 *holder) {
    if (at<u32>(holder, 0) & 1)
        Holder_Sleep(holder);
    Holder_Destroy(holder);
    GlobalMem_Free(at<void *>(holder, 0x24));
    return 1;
}

u32 __cdecl HolderOwner_Dispose(u8 *owner) {
    u8 *const holder = at<u8 *>(owner, 0x24);
    if (at<u32>(holder, 0) & 1)
        Holder_Sleep(holder);
    Holder_Destroy(holder);
    GlobalMem_Free(at<void *>(holder, 0x24));
    u8 *const counter = at<u8 *>(owner, 0x3c);
    at<s32>(counter, 0x1c) = at<s32>(counter, 0x1c) - 1;
    GlobalMem_Free(at<void *>(owner, 0x34));
    return 1;
}

u32 __cdecl Animator_AddToGrid(u8 *animator, u32 from, u32 to) {
    Holder_AddToGrid(at<u8 *>(animator, 0x24), from, to);
    at<u32>(animator, 0) = at<u32>(animator, 0) | 0x4000;
    return 1;
}

void __cdecl Owner_DisposeChildren(u8 *owner) {
    while (u8 *const child = at<u8 *>(owner, 0x38)) {
        at<u8 *>(owner, 0x38) = at<u8 *>(child, 0x38);
        HolderOwner_Dispose(child);
    }
}

void __cdecl Holder_Wake(u8 *holder) {
    u8 *part = nullptr;
    if (u8 *const sub = at<u8 *>(holder, 0x1c))
        part = sub + at<s32>(at<u8 *>(sub, 0), 4);
    Renderer_Add(global<u8 *>(0x0087a6d0), nullptr, part, 0x70);
    at<u32>(holder, 0) = at<u32>(holder, 0) | 1;
}

void __fastcall HolderUser_Acquire(u8 *user) {
    if (at<u32>(user, 4) != 0)
        return;
    u8 *const target = global<u8 *>(0x0079fcb4);
    if (at<u32>(target, 4) & 0x4000)
        Holder_AddToGrid(at<u8 *>(target, 8), at<u32>(target, 0xc0), at<u32>(target, 0xc4));
    else
        Holder_Wake(at<u8 *>(target, 8));
    at<u32>(user, 4) = 1;
}

void __fastcall RideUser_Acquire(u8 *user) {
    if (at<u32>(user, 8) != 0)
        return;
    u8 *const target = reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(user, 4)];
    if (at<u32>(target, 4) & 0x4000)
        Holder_AddToGrid(at<u8 *>(target, 8), at<u32>(target, 0xc0), at<u32>(target, 0xc4));
    else
        Holder_Wake(at<u8 *>(target, 8));
    at<u32>(user, 8) = 1;
}

u32 __cdecl Animator_Dispose(u8 *animator) {
    if (at<u32>(animator, 0) & 0x4000)
        Holder_Sleep(at<u8 *>(animator, 0x24));
    Holder_Destroy(at<u8 *>(animator, 0x24));
    at<s32>(at<u8 *>(animator, 0x3c), 0x1c) = at<s32>(at<u8 *>(animator, 0x3c), 0x1c) - 1;
    if (at<s32>(at<u8 *>(animator, 0x3c), 0x1c) == 0) {
        for (u32 i = 0; i < at<u16>(at<u8 *>(at<u8 *>(animator, 0x24), 4), 0x36); ++i) {
            GlobalMem_Free(at<void *>(at<u8 *>(at<u8 *>(at<u8 *>(animator, 0x24), 4), 0x54), i * 0x10 + 0xc));
            at<u32>(at<u8 *>(at<u8 *>(at<u8 *>(animator, 0x24), 4), 0x54), i * 0x10 + 0xc) = 0;
        }
    }
    GlobalMem_Free(at<void *>(animator, 0x20));
    at<u32>(animator, 0x20) = 0;
    at<u32>(animator, 0) = at<u32>(animator, 0) & 0xffffdfff;
    return 1;
}

u32 __cdecl Instance_Dispose(u8 *instance) {
    if (u8 *const animator = at<u8 *>(instance, 0xb4)) {
        if (at<u32>(animator, 0) & 0x4000)
            Obj_RemoveRectAndClear14(animator, at<u32>(instance, 0xc0), at<u32>(instance, 0xc4));
        HolderOwner_Dispose(at<u8 *>(instance, 0xb4));
        at<u32>(instance, 0xb4) = 0;
    }
    for (u32 i = 0; i < at<u32>(instance, 0xa8); ++i) {
        if (u8 *const part = at<u8 *const *>(instance, 0xac)[i])
            GlobalMem_Free(at<void *>(part, 0x44));
    }
    u8 *const holder = at<u8 *>(instance, 8);
    if (at<u32>(holder, 0) & 1) {
        if (at<u32>(instance, 4) & 0x4000)
            Obj_RemoveFromRect(holder, at<u32>(instance, 0xc0), at<u32>(instance, 0xc4));
        else
            Holder_Sleep(holder);
    }
    Holder_Dispose(at<u8 *>(instance, 8));
    GlobalMem_Free(at<void *>(instance, 0xe0));
    return 1;
}

void __cdecl Animator_InitUnusedOne() { *reinterpret_cast<float *>(0x0078ed8c) = 1.0f; }

void __cdecl Animator_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x0078f47c) = 1.0f / 8388608.0f; }

void __cdecl Instance_InitUnusedOne() { *reinterpret_cast<float *>(0x007a0174) = 1.0f; }

void __cdecl Instance_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x007a086c) = 1.0f / 8388608.0f; }

void __cdecl Holder_Free(u8 *holder) {
    if (at<u32>(holder, 0) & 1)
        Holder_Sleep(holder);
    if (at<u8 *>(holder, 0x1c))
        Holder_Destroy(holder);
    if (at<u8 *>(holder, 0x20) && at<s32>(holder, 0x34) == 0) {
        ObjModel_Unload(at<u8 *>(holder, 0x20));
        if (u8 *object = at<u8 *>(holder, 0x20)) {
            u8 *adjusted = object + at<s32>(at<u8 *>(object, 0), 4);
            vcall<u32>(adjusted, 0, 1u);
        }
    }
    const u8 *model = at<u8 *>(holder, 4);
    u8 *part = at<u8 *>(model, 0x70);
    for (u32 n = at<u16>(model, 0x44); n; --n, part += 0xa0)
        if (part != at<u8 *>(holder, 0x2c) && (at<u32>(part, 0) & 0x1000))
            GlobalMem_Free(at<void *>(part, 0x98));
    GlobalMem_Free(at<void *>(holder, 0x14));
    at<u32>(holder, 0x14) = 0;
    PartStates_Free(holder);
    GlobalMem_Free(at<void *>(at<u8 *>(holder, 4), 0xb0));
    at<u32>(holder, 4) = 0;
    GlobalMem_Free(holder);
}

void __cdecl Instance_Free(u8 *instance, u32 kept) {
    for (u32 i = 0; i < at<u32>(instance, 0xa8); ++i) GlobalMem_FreeAtB0(at<u8 **>(instance, 0xdc)[i]);
    GlobalMem_Free(at<void *>(instance, 0xac));
    at<u32>(instance, 0xac) = 0;
    GlobalMem_Free(at<void *>(instance, 0xdc));
    at<u32>(instance, 0xdc) = 0;
    GlobalMem_Free(at<void *>(instance, 0xd8));
    at<u32>(instance, 0xd8) = 0;
    if (at<u8 *>(instance, 0xb4)) {
        Animator_Dispose(at<u8 *>(instance, 0xb4));
        GlobalMem_Free(at<void *>(at<u8 *>(instance, 0xb4), 8));
        at<u32>(at<u8 *>(instance, 0xb4), 8) = 0;
        GlobalMem_Free(at<void *>(instance, 0xb4));
        at<u32>(instance, 0xb4) = 0;
    }
    if (at<u8 *>(instance, 0xb0)) {
        GlobalMem_Free(at<void *>(at<u8 *>(instance, 0xb0), 0xc));
        at<u32>(at<u8 *>(instance, 0xb0), 0xc) = 0;
        GlobalMem_Free(at<void *>(instance, 0xb0));
        at<u32>(instance, 0xb0) = 0;
    }
    if (kept)
        at<u32>(instance, 0xcc) = 0;
    else
        Instance_FreeBlockCC(instance);
    Holder_Free(at<u8 *>(instance, 8));
    GlobalMem_Free(instance);
}

void __cdecl InstanceTypes_FreeAll() {
    for (u32 n = global<u32>(0x007a439c); n; --n) {
        u8 *const instance = global<u8 *>(0x007ae6a8 + 4 * n);
        if (at<u8 *>(instance, 0xd0)) {
            Instance_Free(at<u8 *>(instance, 0xd0), 1);
            at<u32>(instance, 0xd0) = 0;
        }
        Instance_FreeHolderBlock(instance);
        Instance_FreeRecordBlock(instance);
        Instance_Free(instance, 0);
    }
    global<u32>(0x007a439c) = 0;
}

u32 __cdecl Obj_RemoveFromRect(u8 *object, u32 from, u32 to) {
    u8 *part = at<u8 *>(object, 0x1c);
    if (part)
        part += at<u32>(at<u8 *>(part, 0), 4);
    Renderer_RemoveInRect(global<u8 *>(0x0087a6d0), nullptr, (s32)(from >> 16), (s32)(from & 0xffff), (s32)(to >> 16),
                          (s32)(to & 0xffff), (u32)(uintptr_t)part);
    at<u32>(object, 0) &= 0xfffffffe;
    return at<u32>(object, 0);
}

u32 __cdecl Obj_RemoveRectAndClear14(u8 *object, u32 from, u32 to) {
    Obj_RemoveFromRect(at<u8 *>(object, 0x24), from, to);
    at<u32>(object, 0) &= ~0x4000u;
    return 1;
}

u32 __cdecl Dword_ClearBit0(u32 *p) {
    *p &= 0xfffffffe;
    return 1;
}
}
