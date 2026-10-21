#include "window705bf8.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

s32 width(u8 *window, const u16 *text) {   // the font's slot 2 (text, 0, length - 1), a word
    const u32 length = Crt().wcslen(text);
    return (s16)vcall<u32>(at<u8 *>(window, 0x4c), 8, text, 0u, length - 1);
}

s32 lineHeight(u8 *window) { return (s16)vcall<u32>(at<u8 *>(window, 0x4c), 4); }

u16 across(u8 *window, const u16 *text) {
    if (at<u32>(window, 0x13c) == 1) return (u16)(at<s16>(window, 0x34) - width(window, text));
    if (at<u32>(window, 0x13c) == 2) {
        const s32 room = (s32)at<s16>(window, 0x34) - at<s16>(window, 0x30);
        return (u16)(at<s16>(window, 0x30) + ((room - width(window, text)) >> 1));
    }
    return at<u16>(window, 0x30);
}

u16 down(u8 *window, s32 lines) {
    if (at<u32>(window, 0x140) == 1) return (u16)(at<s16>(window, 0x36) - lineHeight(window) * lines);
    if (at<u32>(window, 0x140) == 2) {
        const s32 room = (s32)at<s16>(window, 0x36) - at<s16>(window, 0x32);
        return (u16)(at<s16>(window, 0x32) + ((room - lineHeight(window) * lines) >> 1));
    }
    return at<u16>(window, 0x32);
}

}  // namespace

extern "C" {

u8 *__fastcall Window705bf8_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(window, 0) = 0x705bf8;
    at<u32>(window, 0x148) = 0;
    at<u32>(window, 0x138) = 0;
    at<u32>(window, 0x13c) = 0;
    at<u32>(window, 0x140) = 0;
    at<u32>(window, 0x144) = 0;
    at<u16>(window, 0x14c) = 0;
    at<u16>(window, 0x14e) = 0;
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    Window_SetHandler(window, nullptr, 0x66d6f5);
    return window;
}

void __fastcall Window705bf8_Place(u8 *window) {
    if (!at<u32>(window, 0x4c) || !at<u32>(window, 0x138)) return;
    at<u16>(window, 0x14c) = across(window, at<const u16 *>(window, 0x138));
    at<u16>(window, 0x14e) = down(window, 1);
}

u16 __fastcall Window705bf8_TextX(u8 *window, void *, const u16 *text) {
    if (!at<u32>(window, 0x4c) || !text) return 0xffff;
    return across(window, text);
}

u16 __fastcall Window705bf8_TextY(u8 *window, void *, const u16 *text, u16 *height) {
    if (!at<u32>(window, 0x4c) || !text) return 0xffff;
    const s32 length = (s32)Crt().wcslen(text);
    s32 lines = 1;
    for (s32 i = 0; i < length; ++i) {
        if (text[i] == 10) ++lines;
    }
    if (height) *height = (u16)vcall<u32>(at<u8 *>(window, 0x4c), 4);
    return down(window, lines);
}

u16 __fastcall Window705bf8_SetText(u8 *window, void *, const u16 *text) {
    const s32 length = (s32)Crt().wcslen(text);
    if (length > at<s32>(window, 0x148)) {
        if (at<u32>(window, 0x138)) GameHeap_Free(at<void *>(window, 0x138));
        at<void *>(window, 0x138) = GameHeap_Alloc((u32)(length * 2 + 2));
        at<s32>(window, 0x148) = length;
    }
    if (!at<u32>(window, 0x138)) return 0;
    Crt().wcscpy(at<u16 *>(window, 0x138), text);
    Window705bf8_Place(window);
    return 1;
}

void __fastcall Window705bf8_GetText(u8 *window, void *, u16 *out, u32 n) {
    if (out) Crt().wcsncpy(out, at<const u16 *>(window, 0x138), n);
}

void __fastcall Window705bf8_SetAlignX(u8 *window, void *, u32 value) {
    if (value != at<u32>(window, 0x13c)) at<u32>(window, 0x13c) = value;
}

void __fastcall Window705bf8_SetAlignY(u8 *window, void *, u32 value) {
    if (value != at<u32>(window, 0x140)) at<u32>(window, 0x140) = value;
}

u32 __fastcall Window705bf8_Dtor(u8 *window) {
    at<u32>(window, 0) = 0x705bf8;
    if (at<u32>(window, 0x138)) GameHeap_Free(at<void *>(window, 0x138));
    return Window_Dtor(window);
}

u8 *__fastcall Window705bf8_DeletingDtor(u8 *window, void *, u32 flags) {
    Window705bf8_Dtor(window);
    if (flags & 1) GameHeap_Free(window);
    return window;
}

u32 __fastcall Window705bf8_Compare(u8 *window, void *, u8 *other) {
    const u32 kind = vcall<u32>(other, 0xc);
    if (kind != vcall<u32>(window, 0xc)) return 0;
    return (u32)Crt().wcscmp(at<const u16 *>(window, 0x138), at<const u16 *>(other, 0x138));
}

u32 __cdecl Window705bf8_Proc(u8 *window, u32 message, u32 a, u32 b) { return Window_DefaultProc(window, message, a, b); }
}
