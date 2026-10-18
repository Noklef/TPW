#include "indexed.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Out_SetFromGame1da70c(u32 *out) {
    const u32 game = global<u32>(0x0080239c);
    *out = at<u32>(reinterpret_cast<const u8 *>(game), 0x1da70c);
    return game;
}

u32 __fastcall Game1da70c_MinusAt0(const u32 *object) {
    return at<u32>(reinterpret_cast<const u8 *>(global<u32>(0x0080239c)), 0x1da70c) - *object;
}

u32 __fastcall Game1da70c_MinusAt0Quarters(const u32 *object) {
    return (at<u32>(reinterpret_cast<const u8 *>(global<u32>(0x0080239c)), 0x1da70c) >> 2) - (*object >> 2);
}

u32 __fastcall Obj_GetIndexed4e0(const u8 *object, void *, u32 index) { return at<u32>(object, 0x4e0 + 4 * index); }

u32 __fastcall Obj_Word6cEquals(const u8 *object, void *, u32 value) { return (u16)value == at<u16>(object, 0x6c); }

u32 __fastcall Obj_ClearIndexed28(u8 *object, void *, u32 index) {
    at<u32>(object, 0x28 + 12 * index) = 0;
    return 3 * index;
}

u32 __fastcall Obj_GetIndexed28(const u8 *object, void *, u32 index) { return at<u32>(object, 0x28 + 12 * index); }

s32 __fastcall Vector20_Count(const u8 *object) {
    const u8 *begin = at<const u8 *>(object, 4);
    return begin ? (s32)(at<const u8 *>(object, 8) - begin) / 20 : 0;
}

u8 *__fastcall Array36_At(const u8 *object, void *, u32 index) { return at<u8 *>(object, 4) + 36 * index; }
}  // extern "C"
