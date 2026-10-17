#include "small62.h"

#include <cstdint>

#include "../world/objectdefs.h"
#include "methods.h"
#include "misc.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The virtual base offset: the word at +4 of the object's vtable.
s32 BaseOffset(const u8 *object) { return at<s32>(at<const u8 *>(object, 0), 4); }

}  // namespace

extern "C" {

u8 *__fastcall Thing_IngredientRecord(u8 *thing) {
    const u8 *definition = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe));
    const u32 kind = at<u32>(definition, 0x158);
    if (kind - 1 > 3)
        return nullptr;
    return Records_DataIfTerminated(global<const u8 *>(0x0078ba8c), nullptr, kind - 1);
}

u8 *__fastcall Class701378_Ctor(u8 *object, void *, u32 mostDerived) {
    if (mostDerived != 0) {
        at<u32>(object, 0) = 0x00701378;
        Class701260_Ctor(object + 0x2c);
    }
    at<u32>(object, BaseOffset(object)) = 0x00701330;
    const s32 offset = BaseOffset(object);
    at<s32>(object, offset - 4) = offset - 0x2c;
    const u8 *vtable = at<const u8 *>(object, 0);
    at<u32>(object, 8) = 0;
    at<u32>(object, 0x10) = 2;
    at<u32>(object, 0x14) = 1;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 4) = 0;
    at<u32>(object, 0x20) = 0xffffffff;
    at<u32>(object, at<s32>(vtable, 4) + 0xc) = 0x3000;
    return object;
}

}  // extern "C"
