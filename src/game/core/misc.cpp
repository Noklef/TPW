#include "misc.h"

#include <cstdint>
#include <initializer_list>

#include "../render/renderer.h"
#include "../render/state.h"
#include "accessors.h"
#include "gameheap.h"
#include "initializers.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Obj_CtorPart8(u8 *object, void *, u32 value) {
    at<u32>(object, 4) = 0;
    at<u32>(object, 0) = 0x006fd7b8;
    Fn_59b550(object + 8, nullptr, value);
    at<u32>(object, 0) = 0x006fd7b0;
    return object;
}

// The part a reference reaches: the object plus the offset at +8 of its vtable.
static u8 *RefPart(u8 *const *ref) {
    u8 *object = *ref;
    return object + at<u32>(at<u8 *>(object, 0), 8);
}

void __cdecl MeshRef_ClearFlag10000000(u8 *const *ref) {
    u8 *part = RefPart(ref);
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<u32>(part, 0x50) &= 0xefffffff;
}

void __cdecl MeshRef_SetFlag10000000(u8 *const *ref) {
    u8 *part = RefPart(ref);
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<u32>(part, 0x50) |= 0x10000000;
}

u8 *__fastcall Obj_CtorVirtual(u8 *object, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(object, 0) = 0x006fe570;
        at<u32>(object, 0x60) = 0x006fdce8;
    }
    at<u32>(object, at<u32>(at<u8 *>(object, 0), 4)) = 0x006fdcd8;
    const u32 base = at<u32>(at<u8 *>(object, 0), 4);
    at<u32>(object, base - 4) = base - 0x60;
    at<u32>(object, 0x50) = 0;
    return object;
}

u32 __fastcall Obj_SetVectorAt2c(u8 *object, void *, u32 a, u32 b, u32 c) {
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    at<u32>(object, 0x2c) = a;
    at<u32>(object, 0x30) = b;
    at<u32>(object, 0x34) = c;
    return a;
}

u32 __fastcall Flags7a1a14_Update(const u8 *object) {
    const u32 bit = (global<u32>(0x007a1a14) >> 3) & 1;
    Obj_AdvanceUnless30(reinterpret_cast<u8 *>(0x00785970));
    global<u32>(0x007477e4) = bit;
    if (!at<u32>(object, 0x18))
        return 0;
    global<u32>(0x007a1a14) |= 2;
    return global<u32>(0x007a1a14);
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

u8 *__fastcall Class703c18_Ctor(u8 *object) {
    Class703770_Ctor(object);
    at<u32>(object, 0) = 0x00703c18;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    return object;
}

u8 *__fastcall Class703ca0_Ctor(u8 *object) {
    Class703770_Ctor(object);
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0x00703748;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x10) = 0x00703744;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 0) = 0x00703ca0;
    at<u32>(object, 0xc) = 0x00703c98;
    return object;
}

u8 *__cdecl Records32_MaxWeight(u8 *records, s32 count) {
    u8 *best = nullptr;
    s32 weight = 0;
    for (s32 i = 0; i < count; i++) {
        u8 *record = records + 0x20 * (u32)i;
        if (at<s32>(record, 0x1c) > weight && at<s32>(record, 0x18) > 0) {
            best = record;
            weight = at<s32>(record, 0x1c);
        }
    }
    return best;
}

u32 __fastcall Obj_AdvanceUnless30(u8 *object) {
    u32 result = Obj_Get30(object);
    if (!result) {
        result = at<u32>(object, 0x40);
        at<u32>(object, 0x3c) += result;
    }
    return result;
}

u8 *__fastcall Obj_CtorOne(u8 *object) {
    Init_402f30(object);
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 0x24) = 0x3ff00000;
    return object;
}

u8 *__fastcall Obj_CtorValue(u8 *object, void *, u32 value) {
    Fn_657d30(object);
    at<u32>(object, 0) = 0x006ff058;
    at<u32>(object, 0x14) = value;
    return object;
}

u32 __cdecl Dword_ClearBit0(u32 *p) {
    *p &= 0xfffffffe;
    return 1;
}

u8 *__fastcall Class703770_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x00703770;
    at<u32>(object, 4) = 1;
    return object;
}

u8 *__fastcall Class703c80_Ctor(u8 *object) {
    at<u32>(object, 4) = 5;
    at<u32>(object, 8) = 5;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x10) = 1;
    at<u32>(object, 0x14) = 0xffffffff;
    at<u32>(object, 0x18) = 0xffffffff;
    at<u32>(object, 0x1c) = 0xffffffff;
    at<u32>(object, 0) = 0x00703c80;
    return object;
}

u8 *__cdecl Codec631280_Init(u8 *object) {
    at<u32>(object, 0) = 0x6312d0;
    at<u32>(object, 4) = 0x631340;
    at<u32>(object, 8) = 0x631300;
    at<u32>(object, 0xc) = 0x631380;
    at<u32>(object, 0x10) = 0x631440;
    at<u32>(object, 0x68) = 0;
    at<u32>(object, 0x6c) = 0;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0x70) = 0x7039d0;
    at<u32>(object, 0x74) = 0x7b;
    at<u32>(object, 0x78) = 0;
    at<u32>(object, 0x7c) = 0;
    at<u32>(object, 0x80) = 0;
    return object;
}

u32 __cdecl Obj_RemoveRectAndClear14(u8 *object, u32 from, u32 to) {
    Obj_RemoveFromRect(at<u8 *>(object, 0x24), from, to);
    at<u32>(object, 0) &= ~0x4000u;
    return 1;
}
}
