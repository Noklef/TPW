#include "small25.h"

#include <cstdint>

#include "small15.h"
#include "small22.h"
#include "stdtree.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __cdecl Table879ad0_Clear() {
    for (u32 address = 0x00879ad0, i = 0; i < 0x40; ++i, address += 8) {
        global<u32>(address) = 0;
        global<u32>(address + 4) = 0;
    }
    global<u32>(0x00879cd0) = 0;
}

u8 **__fastcall StdTreeC_PostInc(u8 **it, void *, u8 **out, s32) {
    u8 *old = *it;
    StdTree_IncC(it);
    *out = old;
    return out;
}

u8 **__fastcall StdTreeE_PostInc(u8 **it, void *, u8 **out, s32) {
    u8 *old = *it;
    StdTree_IncE(it);
    *out = old;
    return out;
}

u8 *__stdcall Obj_InitMinus1A(u8 *object, u32, u32, u32) {
    Obj_Init4f1520(object, nullptr, 0xffffffffu);
    return object;
}

u8 *__stdcall Obj_InitMinus1B(u8 *object, u32, u32, u32, u32, u32, u32, u32) {
    Obj_Init4f1520(object, nullptr, 0xffffffffu);
    return object;
}

u32 __fastcall Obj_SetAt8AndAdd(u8 *object, void *, u8 *item) {
    at<u8 *>(object, 8) = item;
    return ObserverList0_Add(object, nullptr, item);
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

String *__stdcall Strings_CopyRange(const String *first, const String *last, String *dest) {
    while (first != last) {
        if (dest)
            String_CtorCopy(dest, nullptr, reinterpret_cast<const StringView *>(first));
        first = reinterpret_cast<const String *>((uintptr_t)first + 0x14);
        dest = reinterpret_cast<String *>((uintptr_t)dest + 0x14);
    }
    return dest;
}

void __stdcall Strings_FillCopy(String *dest, u32 count, const String *from) {
    for (; count != 0; --count) {
        if (dest)
            String_CtorCopy(dest, nullptr, reinterpret_cast<const StringView *>(from));
        dest = reinterpret_cast<String *>((uintptr_t)dest + 0x14);
    }
}

void __cdecl String_PlacementCopy(String *dest, const String *from) {
    if (dest)
        String_CtorCopy(dest, nullptr, reinterpret_cast<const StringView *>(from));
}

}  // extern "C"
