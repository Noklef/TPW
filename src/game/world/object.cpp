#include "object.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "objectdefs.h"
#include "person.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Object_GetDef(const u8 *object) {
    return ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(object, 0xe));
}

void __fastcall Object_AddCustomer(u8 *object) {
    ++at<u32>(object, 0x1a0);
    ++at<u32>(object, 0x1a8);
}

void __fastcall Object_AddWalkAway(u8 *object) {
    ++at<u32>(object, 0x1a4);
    ++at<u32>(object, 0x230);
}

void __fastcall Object_AddHappinessChange(u8 *object, void *, s32 change) {
    const s32 average = at<s32>(object, 0x340);
    at<s32>(object, 0x340) = average == 0 ? change : (s32)((u32)average + (u32)change) / 2;
}

void __fastcall Object_SetQualityOfGoods(u8 *object, void *, u32 value) { at<u32>(object, 0x18c) = value & 0xff; }

void __fastcall Object_SetChanceOfWinning(u8 *object, void *, u32 value) { at<u32>(object, 0x190) = value & 0xff; }

u16 *__fastcall Object_DefWord284(const u8 *object, void *, u16 *out) {
    *out = at<u16>(Object_GetDef(object), 0x284);
    return out;
}

u32 __fastcall Object_DefIsKind3(const u8 *object) { return at<u32>(Object_GetDef(object), 0x9c) == 3; }

u32 __fastcall Kinds_Compatible(const u8 *object, void *, u32 kind) {
    if (kind == 0)
        return 1;
    const u32 mine = at<u32>(object, 8);
    if (kind == 4 && mine == 4)
        return 0;
    if (kind == 0x15 && mine == 0x15)
        return 1;
    if (kind == 1 && mine == 3)
        return 1;
    if (kind == mine || mine == 0)
        return 1;
    if (kind == 4 && mine == 1)
        return 1;
    if (kind == 3 && mine == 1)
        return global<u32>(0x00820ab8) == 0 ? 1 : 0;
    return 0;
}

void __fastcall Slots_AddIfMatch(u8 *object, void *, u32 id, u32 amount, u32 kind, u32 extra) {
    const s32 index = at<s32>(object, 0x3cc);
    if (index < 0 || index >= 20)
        return;
    u8 *slot = object + 0xc + 48 * (u32)index;
    if (!at<u8>(object, 0x3d9))
        return;
    if (id != at<u32>(slot, 0))
        return;
    if ((u16)kind != at<u16>(slot, 0xc))
        return;
    const u16 limit = at<u16>(object, 0x3e0);
    if (limit && (u16)extra != limit)
        return;
    at<u32>(object, 0x3dc) += amount;
}

u32 __fastcall Slots_GetValue(const u8 *object, void *, u32 slot) {
    const s32 entry = at<s32>(object, 0x1414 + 16 * slot);
    if (entry < 0)
        return 0;
    if (slot == 4) {
        const u32 index = at<u32>(object, 0x1454);
        const u8 *record = object + 20 * index;
        const u8 *def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(record, 0xc));
        return at<u32>(def, 0x40 * at<u8>(record, 0x15) + 0x1b8);
    }
    const u8 *record = object + 20 * (u32)entry + 0xc;
    u8 *controls = global<u8 *>(0x0080239c) + 0x1d943c;
    return at<u32>(ObjectControls_FindUncached(controls, nullptr, at<u16>(record, 0), 0), 4);
}

u32 __fastcall Obj_HasActivity(const u8 *object) {
    if (at<u32>(object, 0) || at<u32>(object, 0x6c))
        return 1;
    for (u32 offset = 0xe0; offset <= 0x100; offset += 4) {
        if (at<u32>(at<u8 *>(object, offset), 0xc))
            return 1;
    }
    if (at<u32>(object, 0x324) || at<u32>(object, 0x38c))
        return 1;
    return 0;
}

u16 *__fastcall Obj_GetTrackedHandle(u8 *object, void *, u16 *out) {
    const u16 handle = at<u16>(object, 0x5e);
    if (handle == 0) {
        *out = 0;
        return out;
    }
    if (global<u32>(global<u32>(0x0080239c) + 0x1da70c) > at<u32>(object, 0x60) + 0x64) {
        const u8 *thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        if (!Person_HasWord(thing, nullptr, at<u16>(object, 0))) {
            at<u16>(object, 0x5e) = 0;
            at<u32>(object, 0x60) = 0;
            *out = 0;
            return out;
        }
    }
    *out = at<u16>(object, 0x5e);
    return out;
}

u32 __fastcall Is19c_Eq1_4e0370(const u8 *o) {
    return at<u32>(o, 0x19c) == 0x1 ? 1 : 0;
}

u32 __fastcall Is19c_Eq4_4e0380(const u8 *o) {
    return at<u32>(o, 0x19c) == 0x4 ? 1 : 0;
}

u32 __fastcall Is19c_Eq2_4e03e0(const u8 *o) {
    return at<u32>(o, 0x19c) == 0x2 ? 1 : 0;
}

u32 __fastcall Is68_Zero_4e0440(const u8 *o) {
    return at<u32>(o, 0x68) == 0 ? 1 : 0;
}
}
