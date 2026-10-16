#include "ridelist.h"

#include <cstdint>

#include "objectdefs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

bool system() { return global<u32>(0x008791a0) != 0; }

u8 *find(s32 id) {
    for (u8 *node = global<u8 *>(0x008791b0); node; node = at<u8 *>(node, 0))
        if (at<s32>(node, 8) == id)
            return node;
    return nullptr;
}

}  // namespace

extern "C" {

u8 *__cdecl Rides_FindById(s32 id) {
    if (!system())
        return nullptr;
    return find(id);
}

void __cdecl Rides_SetElement(u8 *record, s32 index, u32 value) {
    if (!system() || !record)
        return;
    if (index >= 0 && index < at<s32>(record, 0x8c))
        at<u32 *>(record, 0x1c)[index] = value;
}

u32 __cdecl Rides_GetElement(const u8 *record, s32 index) {
    if (!system() || !record)
        return 0;
    if (index >= 0 && index < at<s32>(record, 0x8c))
        return at<const u32 *>(record, 0x1c)[index];
    return 0;
}

void __cdecl Rides_SetWordC0(u8 *record, u32 value) {
    if (!system() || !record)
        return;
    at<u16>(record, 0xc0) = (u16)value;
}

s32 __cdecl Rides_GetWordC0(const u8 *record) {
    if (!system() || !record)
        return 0;
    return at<s16>(record, 0xc0);
}

u32 __cdecl Rides_Get9c(const u8 *record) {
    if (!system() || !record)
        return 0;
    return at<u32>(record, 0x9c);
}

u32 __cdecl Rides_GetLinkedElement(s32 id, s32 index) {
    if (!system())
        return 0;
    const u8 *first = find(id);
    if (!first)
        return 0;
    const u8 *second = find(at<s32>(first, 0x14));
    if (!second)
        return 0;
    if (index >= 0 && index < at<s32>(second, 0x8c))
        return at<const u32 *>(second, 0x1c)[index];
    return 0;
}

void __fastcall Obj_ClearRideElement4(const u8 *object) { Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 4, 0); }

u32 __fastcall Obj_RideElement7Positive(const u8 *object) { return (s32)Rides_GetElement(Rides_FindById(at<s32>(object, 0x24)), 7) > 0; }

u32 __fastcall Obj_RideElement0Differs(const u8 *object) {
    return Rides_GetElement(Rides_FindById(at<s32>(object, 0x24)), 0) != at<u16>(object, 0x3c);
}

void __fastcall Obj_SetSpeed(u8 *object, void *, u32 speed) {
    Rides_SetWordC0(Rides_FindById(at<s32>(object, 0x24)), speed);
    at<u32>(object, 0x58) = speed;
}

void __fastcall Obj_SetDuration(u8 *object, void *, u32 duration) {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    s32 value = (s32)(duration & 0xff);
    if (value < at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x12c))
        value = (u8)at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x12c);
    else if (value > at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x130))
        value = (u8)at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x130);
    Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 3, (u8)value);
    at<u8>(object, 0x5c) = (u8)value;
}

s32 __fastcall Game_GetRideElement1(const u8 *game) {
    u32 handle = at<u16>(game, 0x1da732);
    if (handle == 0)
        return -1;
    u8 *thing = *reinterpret_cast<u8 **>(0x007cfb90 + 20 * handle);
    return (s32)Rides_GetElement(Rides_FindById(at<s32>(thing, 0x24)), 1);
}

}  // extern "C"
