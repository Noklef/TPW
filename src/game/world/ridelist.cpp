#include "ridelist.h"

#include <cstdint>

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

}  // extern "C"
