#include "dialogs.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "window.h"

namespace {

u8 *window(u32 address) { return *reinterpret_cast<u8 **>((uintptr_t)address); }
}

extern "C" {

s32 __cdecl Dialog7cc47c_Message4() { return Window_SendMessage(window(0x007cc47c), 4, 0, 0); }

s32 __cdecl Dialog7cc47c_Message6On() { return Window_SendMessage(window(0x007cc47c), 6, 1, 0); }

s32 __cdecl Dialog7cc47c_Message6Off() { return Window_SendMessage(window(0x007cc47c), 6, 0, 0); }

s32 __cdecl Dialog7cc4b4_Message4() { return Window_SendMessage(window(0x007cc4b4), 4, 0, 0); }

s32 __cdecl Dialog7cc4b4_Message6Off() { return Window_SendMessage(window(0x007cc4b4), 6, 0, 0); }

s32 __cdecl Dialog7cc4c4_Message4() { return Window_SendMessage(window(0x007cc4c4), 4, 0, 0); }

s32 __cdecl Dialog7cc4c4_Message6On() { return Window_SendMessage(window(0x007cc4c4), 6, 1, 0); }

s32 __cdecl Dialog7cdb0c_Message4() { return Window_SendMessage(window(0x007cdb0c), 4, 0, 0); }

s32 __cdecl Dialog7cdb0c_Message6On() { return Window_SendMessage(window(0x007cdb0c), 6, 1, 0); }

s32 __cdecl Dialog7cdb0c_Message6Off() { return Window_SendMessage(window(0x007cdb0c), 6, 0, 0); }

s32 __cdecl Dialog7cdb0c_Child16007Message6On() {
    return Window_SendMessage(Window_FindChild(window(0x007cdb0c), nullptr, 0x16007), 6, 1, 0);
}

s32 __cdecl Dialog7cdb0c_Child16007Message6Off() {
    return Window_SendMessage(Window_FindChild(window(0x007cdb0c), nullptr, 0x16007), 6, 0, 0);
}

s32 __cdecl Dialog7ca098_Message4IfUp() {
    s32 result = 0;
    if (u8 *const w = window(0x007ca098)) result = Window_SendMessage(w, 4, 0, 0);
    *reinterpret_cast<u32 *>((uintptr_t)0x007ca090) = 0;
    return result;
}

s32 __cdecl Dialog7cca94_Message4IfUp() {
    u8 *const w = window(0x007cca94);
    return w ? Window_SendMessage(w, 4, 0, 0) : 0;
}

s32 __fastcall WindowAt10_Message4(u8 *object) { return Window_SendMessage(*reinterpret_cast<u8 **>(object + 0x10), 4, 0, 0); }

s32 __fastcall WindowAt10_Message4IfAny(u8 *object) {
    u8 *const w = *reinterpret_cast<u8 **>(object + 0x10);
    return w ? Window_SendMessage(w, 4, 0, 0) : 0;
}

u8 *__fastcall WindowAt10_ScalarDtor(u8 *object, void *, u32 flags) {
    WindowAt10_Message4IfAny(object);
    if (flags & 1) GameHeap_Free(object);
    return object;
}

void __cdecl Windows7c26b0_Message5() {
    for (u32 entry = 0x007c26b0; entry < 0x007c3b00; entry += 0x208)
        if (*reinterpret_cast<u16 *>((uintptr_t)entry + 4)) Window_SendMessage(*reinterpret_cast<u8 **>((uintptr_t)entry), 5, 0, 0);
}
}
