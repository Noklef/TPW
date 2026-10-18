#include "misc.h"

#include <cstdint>
#include <initializer_list>

#include "../render/renderer.h"
#include "../render/state.h"
#include "../sprite/script.h"
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


s32 __fastcall Vector36_Size(const u8 *vector) {
    const u32 begin = at<u32>(vector, 4);
    if (!begin)
        return 0;
    return (s32)(at<u32>(vector, 8) - begin) / 36;
}

s32 __cdecl PairTable_Commit() {
    const s32 count = global<s32>(0x00820a8c);
    for (s32 i = 0; i < count; i++) {
        global<u32>(0x00818d98 + 8 * (u32)i) = global<u32>(0x0081b740 + 8 * (u32)i);
        global<u32>(0x00818d9c + 8 * (u32)i) = global<u32>(0x0081b744 + 8 * (u32)i);
    }
    global<s32>(0x00820a90) = count;
    global<u32>(0x00820a94) = 0;
    return count > 0 ? count : 0;
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

void __cdecl Global7c24d8_CopyScript() {
    u8 *object = global<u8 *>(0x007c24d8);
    if (object && at<u32>(object, 0xc))
        SpriteScript_CopyFrom((s32)global<u32>(0x007c24a0), (s32)at<u32>(object, 0xc));
}

u32 __fastcall Records24_Field14(const u8 *records, void *, u32 i) { return at<u32>(records, 24 * i + 0x14); }

u8 *__fastcall Records24_IfUsed(u8 *records, void *, u32 i) {
    u8 *record = records + 24 * i;
    return at<u32>(record, 4) != 0 ? record : nullptr;
}

u32 __fastcall Records24_CountUsed(const u8 *records) {
    u32 count = 0;
    for (u32 i = 0; i < 4; i++)
        count += at<u32>(records, 4 + 24 * i) != 0 ? 1 : 0;
    return count;
}

u32 __stdcall Span_Limited(u32, u32 from, u32 to, u32 limit) {
    const u32 span = to - from;
    return limit < span ? limit : span;
}

u32 __stdcall Obj_Positive40(const u8 *object) { return at<s32>(object, 0x40) > 0 ? 1 : 0; }

void __fastcall Obj_SetOrKeepC(u8 *object, void *, u32 value) {
    if (value) {
        at<u32>(object, 0xc) = value;
        at<u32>(object, 0x14) = 0;
    }
}

u8 *__fastcall Class703c18_Ctor(u8 *object) {
    Class703770_Ctor(object);
    at<u32>(object, 0) = 0x00703c18;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    return object;
}

u8 *__fastcall WString_CtorReturning(u8 *string) {
    WString_Ctor(reinterpret_cast<WString *>(string));
    return string;
}

u32 __cdecl DirEntry_IsRealDir(const u8 *entry) {
    if (at<u8>(entry, 4) & 0x10) {
        if (*at<const char *>(entry, 0x20) != '.')
            return 1;
    }
    return 0;
}

u8 *__fastcall Class70295c_Ctor(u8 *object) {
    Init_5d36c0(object);
    at<u32>(object, 0) = 0x0070295c;
    for (u32 offset : {0x8u, 0xcu, 0x10u, 0x14u})
        at<u32>(object, offset) = 0;
    return object;
}

void __fastcall Obj_SwapPair(u8 *object, void *, u8 *other) {
    const u32 first = at<u32>(object, 4);
    at<u32>(object, 4) = at<u32>(other, 4);
    at<u32>(other, 4) = first;
    const u32 second = at<u32>(other, 8);
    const u32 own = at<u32>(object, 8);
    at<u32>(object, 8) = second;
    at<u32>(other, 8) = own;
}

void __fastcall Obj_ChooseVector(u8 *object) {
    const u32 base = at<u32>(object, 0x58) ? 0x28 : 0x1c;
    const u32 x = at<u32>(object, base), y = at<u32>(object, base + 4), z = at<u32>(object, base + 8);
    at<u32>(object, 0x10) = x;
    at<u32>(object, 0x14) = y;
    at<u32>(object, 0x18) = z;
}

u32 __stdcall Obj_Pending(const u8 *object) {
    return at<u32>(object, 0x60) == 0 && at<s32>(object, 0x40) < at<s32>(object, 0x64) ? 1 : 0;
}

u32 __fastcall Obj_SetShown(u8 *object, void *, u32 value) {
    if (at<u32>(object, 0x2c) == value)
        return value;
    at<u32>(object, 0x2c) = value;
    u8 *target = reinterpret_cast<u8 *>(Fn_56b620(at<u32>(object, 0x28)));
    at<u32>(target, 0x10) = value ? at<u32>(target, 0x10) & 0xfffffffe : at<u32>(target, 0x10) | 1;
    return (u32)(uintptr_t)target;
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

u8 *__fastcall Class7035d8_Ctor(u8 *object) {
    at<u32>(object, 4) = 0x007035d8;
    at<u8>(object, 0) = 0;
    at<u16>(object, 8) = 0;
    at<u16>(object, 0xa) = 0;
    at<u16>(object, 0xc) = 0;
    return object;
}

u8 *__cdecl Class7035d8_InitGlobal() { return Class7035d8_Ctor(reinterpret_cast<u8 *>(0x00fa78e8)); }

u32 __cdecl Obj_RemoveRectAndClear14(u8 *object, u32 from, u32 to) {
    Obj_RemoveFromRect(at<u8 *>(object, 0x24), from, to);
    at<u32>(object, 0) &= ~0x4000u;
    return 1;
}
}
