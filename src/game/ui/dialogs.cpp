#include "dialogs.h"

#include <cstdint>

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
}
