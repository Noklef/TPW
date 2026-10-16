#include "misc.h"

#include <cstdint>
#include <initializer_list>

#include "../render/state.h"
#include "../sprite/script.h"
#include "accessors.h"
#include "tiny_c.h"
#include "tiny_d.h"
#include "tiny_e.h"

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

u32 __cdecl Countdown7cc150_Step() {
    s32 &counter = global<s32>(0x007cc150);
    if (counter > 0)
        counter--;
    return counter == 0 && global<u32>(0x007cc190) != 0 ? 1 : 0;
}

u32 __fastcall Obj_KillSlot24(u8 *object) {
    if (!at<u32>(object, 0x24))
        return 0;
    SpriteScript_Kill((s32)at<u32>(object, 0x24));
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x28) = 0xffffffff;
    at<u32>(object, 0x2c) = 0xffffffff;
    return 0xffffffff;
}

u32 __cdecl Global7cc698_Choose() {
    if (global<u32>(0x007cc8b8)) {
        global<u32>(0x007cc698) = global<u32>(0x007cc8b8);
        return global<u32>(0x007cc698);
    }
    if (global<u8>(0x007cc8b4)) {
        global<u32>(0x007cc698) = global<u32>(0x007cc8bc);
        return global<u32>(0x007cc698);
    }
    global<u32>(0x007cc698) = global<u32>(0x007cc8c0);
    return 0;
}

s32 __fastcall Obj_SmallestEntry(const u8 *object) {
    s32 best = -1;
    u32 smallest = 0;
    bool found = false;
    for (u32 i = 0; i < 8; i++) {
        const u8 *entry = object + 0x20 + 0x20 * i;
        const u32 size = at<u32>(entry, 8) ? (at<u32>(entry, 0) - at<u32>(entry, 0xc)) * at<u32>(entry, 4) : 0;
        if (size > 0 && (size < smallest || !found)) {
            best = (s32)i;
            smallest = size;
            found = true;
        }
    }
    return best;
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

u8 *__fastcall Class701260_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x00701260;
    at<u32>(object, 4) = 0;
    at<u32>(object, 0x14) = 0xffffffff;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0x24) = 0;
    if (at<u32>(global<u8 *>(0x00879a24), 0x428) == 0x10de && global<u32>(0x008bd524) == 1)
        global<u32>(0x00879ab4) = 1;
    return object;
}

u8 *__fastcall Obj_CtorVirtual5646c0(u8 *object, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(object, 0) = 0x00701188;
        at<u32>(object, 0x18) = 0x00701178;
    }
    at<u32>(object, at<u32>(at<u8 *>(object, 0), 4)) = 0x00701168;
    const u32 base = at<u32>(at<u8 *>(object, 0), 4);
    at<u32>(object, base - 4) = base - 0x18;
    for (u32 offset : {0xcu, 0x10u, 0x8u, 0x4u})
        at<u32>(object, offset) = 0;
    return object;
}

u8 *__fastcall Obj_CtorVirtual564ba0(u8 *object, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(object, 0) = 0x007011d8;
        at<u32>(object, 0x2c) = 0x00701178;
    }
    at<u32>(object, at<u32>(at<u8 *>(object, 0), 4)) = 0x007011c8;
    const u32 base = at<u32>(at<u8 *>(object, 0), 4);
    at<u32>(object, base - 4) = base - 0x2c;
    for (u32 i = 0; i < 7; i++)
        at<u32>(object, 4 + 4 * i) = 0;
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x20) = 0;
    return object;
}

u32 __fastcall PtrArray_SetVectorOf(const u8 *list, void *, u8 *key, u32 a, u32 b, u32 c) {
    const u32 *items = at<const u32 *>(list, 8);
    s32 index = (s32)at<u32>(list, 0x1c) - 1;
    if (index >= 0) {
        while (items[index] != (u32)(uintptr_t)key) {
            index--;
            if (index < 0)
                break;
        }
    }
    if (index < 0)
        return 0;
    at<u32>(key, 0x14) = a;
    at<u32>(key, 0x18) = b;
    at<u32>(key, 0x1c) = c;
    return 1;
}

u32 __cdecl PairTable_Add(u32 a, u32 b) {
    const s32 count = global<s32>(0x00820a8c);
    if (count > 0x400)
        return 0;
    global<u32>(0x0081b740 + 8 * count) = a;
    global<u32>(0x0081b744 + 8 * count) = b;
    global<s32>(0x00820a8c) = count + 1;
    return 1;
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

void __fastcall Obj_KillSlot(u8 *object) {
    SpriteScript_Kill((s32)at<u32>(object, 0));
    at<u32>(object, 0) = 0;
}

u32 __fastcall Obj_StampAt30(u8 *object, void *, u32 word) {
    at<u16>(object, 0x30) = (u16)word;
    const u32 stamp = at<u32>(global<u8 *>(0x0080239c), 0x1da70c);
    at<u32>(object, 0x34) = stamp;
    return stamp;
}

u32 __fastcall Obj_StampAt5e(u8 *object, void *, u32 word) {
    at<u16>(object, 0x5e) = (u16)word;
    const u32 stamp = at<u32>(global<u8 *>(0x0080239c), 0x1da70c);
    at<u32>(object, 0x60) = stamp;
    return stamp;
}

u32 __cdecl Dword_ClearBit0(u32 *p) {
    *p &= 0xfffffffe;
    return 1;
}

u32 __fastcall Obj_Clear18_0c(u8 *object) {
    at<u32>(object, 0x18) = 0;
    at<u16>(object, 0xc) = 0;
    return 0;
}

u32 __fastcall Obj_Diff180_184(const u8 *object) { return at<u32>(object, 0x180) - at<u32>(object, 0x184); }

void __fastcall Vtbl7099ec_SetVirtual(u8 *sub) {
    const u32 base = at<u32>(at<u8 *>(sub, (u32)-4), 4);
    at<u32>(sub, base - 4) = 0x007099ec;
}

u32 __fastcall Wstreambuf_InitPointers(u8 *object) {
    static const u32 kSlots[6][2] = {{0xc, 0x4}, {0x10, 0x8}, {0x1c, 0x14}, {0x20, 0x18}, {0x2c, 0x24}, {0x30, 0x28}};
    for (const auto &slot : kSlots)
        at<u32>(object, slot[0]) = (u32)(uintptr_t)object + slot[1];
    for (u32 offset : {0x8u, 0x18u, 0x28u, 0x4u, 0x14u, 0x24u})
        at<u32>(object, offset) = 0;
    return (u32)(uintptr_t)object + 0x24;
}

u32 *__fastcall Wstreambuf_Setg(u8 *object, void *, u32 first, u32 next, u32 last) {
    *at<u32 *>(object, 0xc) = first;
    *at<u32 *>(object, 0x1c) = next;
    u32 *count = at<u32 *>(object, 0x2c);
    *count = (u32)((s32)(last - next) >> 1);
    return count;
}

u32 *__fastcall Wstreambuf_Setp(u8 *object, void *, u32 first, u32 last) {
    *at<u32 *>(object, 0x10) = first;
    *at<u32 *>(object, 0x20) = first;
    u32 *count = at<u32 *>(object, 0x30);
    *count = (u32)((s32)(last - first) >> 1);
    return count;
}

u32 __stdcall SoftRenderer_PixelAt(s32 x, s32 y, u32 *out) {
    const u32 *buffer = at<const u32 *>(global<u8 *>(0x00879dac), 8);
    *out = buffer[(u32)y * global<u32>(0x008bcbd0) + (u32)x];
    return 1;
}

u32 __cdecl Table879d08_Fill() {
    for (u32 i = 0; i < 16; i++)
        global<u32>(0x00879d08 + 4 * i) = 16 - i;
    return 0;
}

u32 __fastcall Obj_Set30_34(u8 *object, void *, u32 a, u32 b) {
    at<u32>(object, 0x30) = a;
    at<u32>(object, 0x34) = b;
    return a;
}
}
