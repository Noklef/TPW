#include "object.h"

#include <cstdint>

#include "objectdefs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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
}
