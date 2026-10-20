#include "soundset.h"

#include <cstdint>

#include "class6fd9d8.h"
#include "string.h"
#include "vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void set(u32 address, u32 value) {
    global<u32>(0x00803ac8) = 1;
    global<u32>(address) = value;
}

}  // namespace

extern "C" {

void __cdecl Sound_Set803ab8(u32 value) { set(0x00803ab8, value); }
void __cdecl Sound_Set803ac0(u32 value) { set(0x00803ac0, value); }
void __cdecl Sound_Set803abc(u32 value) { set(0x00803abc, value); }
void __cdecl Sound_Set803ac4(u32 value) { set(0x00803ac4, value); }
void __cdecl Sound_Set803adc(u32 value) { set(0x00803adc, value); }
void __cdecl Sound_Set803ad8(u32 value) { set(0x00803ad8, value); }
void __cdecl Sound_Set803ae0(u32 value) { set(0x00803ae0, value); }
void __cdecl Sound_Set803acc(u32 value) { set(0x00803acc, value); }

void __cdecl Sound_Set803ad0(u32 value) {
    global<u32>(0x00803ac8) = 1;
    global<u16>(0x00803ad0) = (u16)value;
}

void __cdecl Sound_Set803ad2(u32 value) { global<u16>(0x00803ad2) = (u16)value; }

u32 __cdecl Sound_IsOn() { return global<u32>(0x00802bc8); }

u8 *__fastcall SoundCmd_Ctor(u8 *cmd, void *, u32 value) {
    *reinterpret_cast<u32 *>(cmd + 4) = value;
    *reinterpret_cast<u32 *>(cmd) = 0x00700b58;
    return cmd;
}

u32 __cdecl Sound_Query(u32 a, u32 b) {
    if (!global<u32>(0x00802bc8) || !global<u32>(0x00802bd4)) return 0;
    void *const manager = global<void *>(0x00802bcc);
    u32 first = a, out = b;
    vcall<void>(manager, 4, &first, a, b, &out);
    return out;
}

u32 __cdecl Sound_QueryFound(const char *name, u8 *folders) {
    String found, key;
    String_Ctor(&found);
    String_CtorStr(&key, nullptr, name);
    const bool hit = Class6fd9d8_FindFirstMatch(folders, nullptr, reinterpret_cast<const StringView *>(&key), (u32)(uintptr_t)&found) == 1;
    String_Dtor(&key);
    u32 result = 0;
    if (hit && global<u32>(0x00802bc8) && global<u32>(0x00802bd4)) {
        u8 *first = folders;
        u32 out = (u32)(uintptr_t)name;
        vcall<void>(global<void *>(0x00802bcc), 4, &first, found.data, name, &out);
        result = out;
    }
    String_Dtor(&found);
    return result;
}

void __cdecl Sound_Shutdown() {
    if (global<u32>(0x00802bc8) && global<u32>(0x00802bd4)) {
        u32 result;
        vcall<void>(global<void *>(0x00802bcc), 0, &result);
    }
    global<u32>(0x00802bd4) = 0;
}
}
