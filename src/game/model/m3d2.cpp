#include "m3d2.h"

#include <cstdint>

extern "C" {

u32 __cdecl M3D2_DataErrorBox(const char *text) {
    using MessageBoxA = s32(__stdcall *)(void *, const char *, const char *, u32);
    const MessageBoxA box = *reinterpret_cast<const MessageBoxA *>(static_cast<std::uintptr_t>(0x006fd37c));
    return box(nullptr, text, reinterpret_cast<const char *>(static_cast<std::uintptr_t>(0x0074da14)), 0x31) != 2 ? 1 : 0;
}
}
