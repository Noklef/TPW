#include "guidialog.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "guirect.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T get(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }
}

extern "C" {

void __fastcall GuiDialog_SetA(u8 *dialog, void *, u32 value) {
    if ((s16)value < at<s16>(dialog, 0xe)) at<u16>(dialog, 0xa) = (u16)value;
}

void __fastcall GuiDialog_SetC(u8 *dialog, void *, u32 value) {
    if ((s16)value < at<s16>(dialog, 0xe)) at<u16>(dialog, 0xc) = (u16)value;
}

u8 *__fastcall GuiDialog_Ctor(u8 *dialog, void *, u32 value) {
    GuiRect_Ctor(dialog + 0x14);
    GuiRect_Ctor(dialog + 0x20);
    at<u32>(dialog, 0) = 0x7058f8;
    at<u16>(dialog, 0x10) = 0;
    at<u16>(dialog, 0x12) = 0;
    at<u32>(dialog, 4) = value;
    at<u16>(dialog, 0xa) = 0;
    at<u16>(dialog, 0xc) = 0;
    at<u16>(dialog, 8) = 0;
    for (u32 offset = 0x36; offset >= 0x2c; offset -= 2) at<u16>(dialog, offset) = 0;
    Fn_659994(dialog, nullptr, 0);
    return dialog;
}

void __fastcall GuiDialog_Dtor(u8 *dialog) {
    at<u32>(dialog, 0) = 0x7058f8;
    GuiRect_Dtor(dialog + 0x20);
    GuiRect_Dtor(dialog + 0x14);
}

u8 *__fastcall GuiDialog_ScalarDtor(u8 *dialog, void *, u32 flags) {
    GuiDialog_Dtor(dialog);
    if (flags & 1) GameHeap_Free(dialog);
    return dialog;
}

void __fastcall GuiDialog_Apply(u8 *dialog, void *, u32 flags, const u8 *source) {
    if (!source || !flags) return;
    if (flags & 1) GuiDialog_SetC(dialog, nullptr, get<u16>(source, 4));
    if (flags & 4) at<u16>(dialog, 0x30) = get<u16>(source, 8);
    if (flags & 0x80000) GuiDialog_SetA(dialog, nullptr, get<u16>(source, 0xa));
    if (flags & 0x40) {
        at<u16>(dialog, 0x34) = get<u16>(source, 0xc);
        at<u16>(dialog, 0x36) = get<u16>(source, 0xe);
    }
}
}
