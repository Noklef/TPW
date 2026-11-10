#include "m3d2.h"

#include <cstdint>
#include "../core/imports.h"

extern "C" {

u32 __cdecl M3D2_DataErrorBox(const char *text) {
    using MessageBoxA = s32(__stdcall *)(void *, const char *, const char *, u32);
    const MessageBoxA box = *reinterpret_cast<const MessageBoxA *>(static_cast<std::uintptr_t>(IMPORT(User32_MessageBoxA)));
    return box(nullptr, text, gText_M3D2_DataErrorBox_74da14, 0x31) != 2 ? 1 : 0;
}
}
