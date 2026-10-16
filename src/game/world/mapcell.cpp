#include "mapcell.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall MapHandle_Entry28(const u16 *handle) {
    return global<u8 *>(0x008023a0) + 0x10ffd8 + (u32)*handle * 0x28;
}

u8 *__fastcall MapHandle_Cell44(const u16 *handle) { return global<u8 *>(0x008023a0) + (u32)*handle * 0x44 - 0x44; }

void __fastcall MemRanges_Set(u8 *ranges, void *, u32 index, u32 a, u32 b, const char *) {
    at<u32>(ranges, index * 8) = a;
    at<u32>(ranges, index * 8 + 4) = b;
}

void __cdecl MemRanges_SetCells44(u32 base, u32 stride) {
    global<u32>(0x007cdb40) = base;
    global<u32>(0x007cdb50) = stride;
    global<u32>(0x007cdb44) = (stride << 14) + base;
}

void __cdecl MemRanges_SetCells28(u32 base, u32 stride) {
    global<u32>(0x007cdb48) = base;
    global<u32>(0x007cdb54) = stride;
    global<u32>(0x007cdb4c) = (stride << 14) + base;
}
}
