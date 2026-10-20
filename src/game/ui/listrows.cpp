#include "listrows.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/accessors.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../render/textparts.h"
#include "window.h"
#include "windowfactory.h"
#include "windowreg.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

using Height = u32(__fastcall *)(u8 *, void *);

u32 rowHeight(u32 which = 7) {
    u8 *const font = Language_Font(which);
    const s32 height = (s16)reinterpret_cast<Height>((*reinterpret_cast<void ***>(font))[1])(font, nullptr);
    return (u32)(height * 0x600 / (s32)global<s16>(0x00faa5c0) + 6);
}

u8 *build(u8 *window, u32 height) { return Window_Ctor(window, nullptr, nullptr, 0, 0, 0, 0, 0x64, height); }

void initRow(u8 *row) {
    u8 *const part = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(row, nullptr, part ? TextPart_Ctor(part, nullptr, 0) : nullptr);
    Window_Set4c(row, nullptr, Language_Font(7));
    Window_Notify40000(row, nullptr, 0xff, 0xff, 0xff, 0xff);
}
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

void __cdecl HilightList_InitRow(u8 *row) { initRow(row); }

void __cdecl List7cc188_InitRow(u8 *row) { initRow(row); }

u8 *__cdecl List7cc338_NewRow() {
    if (global<u32>(0x007cc35c) == 0) global<u32>(0x007cc35c) = rowHeight(global<u32>(0x00756f38));
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x138));
    u8 *const window = memory ? build(memory, global<u16>(0x007cc35c)) : nullptr;
    if (window) Window_CreateChild(window, nullptr, 9, 0x11, 1, 0x64, 0, 0xc8, 0x32);
    return window;
}

u8 *__cdecl List7cc530_NewRow() {
    const u32 height = rowHeight();
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x138));
    u8 *const window = memory ? build(memory, height) : nullptr;
    Window_CreateChild(window, nullptr, 6, 1, 2, 0x64, 0, 0xc8, 0x32);   // (sic: not checked)
    return window;
}

void __cdecl List7cc530_InitRow(u8 *row) {
    initRow(row);
    u8 *const box = reinterpret_cast<u8 *>((uintptr_t)Fn_65d547(row, nullptr, 0));
    Window_SetObject98(box, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName("i_boxtick")));
    vcall<void>(box, 0x1c, 0u);
}
}  // extern "C"
