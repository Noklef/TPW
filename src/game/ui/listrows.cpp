#include "listrows.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

using Height = u32(__fastcall *)(u8 *, void *);

u32 rowHeight() {
    u8 *const font = Language_Font(7);
    const s32 height = (s16)reinterpret_cast<Height>((*reinterpret_cast<void ***>(font))[1])(font, nullptr);
    return (u32)(height * 0x600 / (s32)global<s16>(0x00faa5c0) + 6);
}

u8 *build(u8 *window, u32 height) { return Window_Ctor(window, nullptr, nullptr, 0, 0, 0, 0, 0x64, height); }
}  // namespace

extern "C" {

u8 *__cdecl HilightList_NewRow() {
    const u32 height = rowHeight();
    u8 *const window = static_cast<u8 *>(GameHeap_Alloc(0x138));
    return window ? build(window, height) : nullptr;
}

u8 *__cdecl List7cc188_NewRow() {
    u8 *const window = static_cast<u8 *>(GameHeap_Alloc(0x138));
    return window ? build(window, rowHeight()) : nullptr;
}
}  // extern "C"
