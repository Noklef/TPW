#include "small63.h"

#include <cstdint>

#include "../world/things.h"
#include "small15.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Thing_Construct(u8 *thing, void *, u32 cell, u32 kind) {
    at<u16>(thing, 0) = 0;
    at<u8>(thing, 2) = (u8)kind;
    at<u8>(thing, 3) = 0;
    at<u16>(thing, 8) = 0;
    at<u16>(thing, 0xa) = 0;
    u16 handle;
    at<u16>(thing, 0) = *World_AddThing(global<void *>(0x0080239c), nullptr, &handle, thing);
    at<u8>(thing, 4) = 0x80;
    at<u8>(thing, 6) = 0x80;
    const u32 id = cell & 0xffff;
    const u32 index = id - 1;
    at<u8>(thing, 5) = (u8)((u8)cell - 1) & 0x7f;
    at<u8>(thing, 7) = (u8)(index >> 7);
    Obj_ReplaceHandle24(global<u8 *>(0x008023a0) + id * 0x44 - 0x44, nullptr, at<u16>(thing, 0));
    // (the print "Creating thing at ..." is left out)
    return thing;
}

}  // extern "C"
