#include "listrows.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/accessors.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../render/textparts.h"
#include "panelparts.h"
#include "window.h"
#include "windowfactory.h"
#include "windowreg.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

using Height = u32(__fastcall *)(u8 *, void *);

// the font's height (its slot 1)
u32 fontHeight(u32 which) {
    u8 *const font = Language_Font(which);
    return reinterpret_cast<Height>((*reinterpret_cast<void ***>(font))[1])(font, nullptr);
}

u32 rowHeight(u32 which = 7) {
    const s32 height = (s16)fontHeight(which);
    return (u32)(height * 0x600 / (s32)global<s16>(0x00faa5c0) + 6);
}

u8 *build(u8 *window, u32 height) { return Window_Ctor(window, nullptr, nullptr, 0, 0, 0, 0, 0x64, height); }

// the height cached in the dword `cache`, worked out (rowHeight) while it is 0 with the font index `*font` (a global's, or 7)
const u32 kFont7 = 7;

u16 cachedHeight(u32 cache, const u32 *font) {
    if (global<u32>(cache) == 0) global<u32>(cache) = rowHeight(*font);
    return global<u16>(cache);
}

const u32 *fontAt(u32 address) { return reinterpret_cast<const u32 *>((uintptr_t)address); }

// a new row window (0x138 bytes) built with no parent, id 0, the rectangle (0, 0, right, height); 0 when the allocation fails
u8 *newRow(u32 flags, u32 right, u32 height) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x138));
    return memory ? Window_Ctor(memory, nullptr, nullptr, flags, 0, 0, 0, right, height) : nullptr;
}

u8 *child(u8 *row, u32 kind, u32 flags, u32 id, u32 left = 0, u32 top = 0, u32 right = 0, u32 bottom = 0) {
    return Window_CreateChild(row, nullptr, kind, flags, id, left, top, right, bottom);
}

using PartCtor = u8 *(__fastcall *)(u8 *, void *, u32);

// a new text part of the kind `ctor` builds (0x1a4 bytes; none when the allocation fails) as the window's impl
void setPart(u8 *window, PartCtor ctor) {
    u8 *const part = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(window, nullptr, part ? ctor(part, nullptr, 0) : nullptr);
}

void initRow(u8 *row) {
    setPart(row, TextPart_Ctor);
    Window_Set4c(row, nullptr, Language_Font(7));
    Window_Notify40000(row, nullptr, 0xff, 0xff, 0xff, 0xff);
}

u8 *firstChild(u8 *window) { return reinterpret_cast<u8 *>((uintptr_t)Fn_65d547(window, nullptr, 0)); }

u8 *nextSibling(u8 *window) { return reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(window)); }

u8 *lookOf(const char *name) { return reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(name)); }

// a TextPart_Ctor part, font 7 and the text 0 (Window_NotifyImpl100)
void textRow(u8 *window) {
    setPart(window, TextPart_Ctor);
    Window_Set4c(window, nullptr, Language_Font(7));
    Window_NotifyImpl100(window, nullptr, 0);
}

// a TextPart6ff500_Ctor part as the object +0x98, font 7 and the bytes 1, 1 as 0xc00
void objectRow(u8 *window) {
    u8 *const part = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetObject98(window, nullptr, part ? TextPart6ff500_Ctor(part, nullptr, 0) : nullptr);
    Window_Set4c(window, nullptr, Language_Font(7));
    Window_NotifyC00(window, nullptr, 1, 1);
}

// a TextPart6ff350_Ctor part, font 7, slot 7 called with 0 and the bytes 1, 1 as 0xc00
void partRow(u8 *window) {
    setPart(window, TextPart6ff350_Ctor);
    Window_Set4c(window, nullptr, Language_Font(7));
    vcall<void>(window, 0x1c, 0u);
    Window_NotifyC00(window, nullptr, 1, 1);
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
    u8 *const window = newRow(0, 0x64, cachedHeight(0x007cc35c, fontAt(0x00756f38)));
    if (window) child(window, 9, 0x11, 1, 0x64, 0, 0xc8, 0x32);
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
    u8 *const box = firstChild(row);
    Window_SetObject98(box, nullptr, lookOf("i_boxtick"));
    vcall<void>(box, 0x1c, 0u);
}

u8 *__cdecl List7ca140_NewRow() {
    if (global<u32>(0x007ca140) == 0) fontHeight(global<u32>(0x007508b4));   // (sic: the font's height asked for and dropped)
    global<u32>(0x007ca140) = 0x38;
    u8 *const window = newRow(0, 0, global<u16>(0x007ca140));
    if (window) {
        child(window, 1, 1, 3, 0, 0x2c, 0, 0x2c);
        child(window, 1, 1, 0x3e7);
        child(window, 1, 0x11, 4);
        child(window, 6, 1, 4);
        child(window, 9, 0x11, 5);
    }
    return window;
}

u8 *__cdecl List7ca314_NewRow() {
    u8 *const window = newRow(0, 0x64, cachedHeight(0x007ca314, fontAt(0x007523dc)));
    if (window) child(window, 1, 1, 1, 0x64, 0, 0xc8, 0x32);
    return window;
}

u8 *__cdecl List7cb24c_NewRow() {
    u8 *const window = newRow(0, 0x64, cachedHeight(0x007cb24c, &kFont7));
    child(window, 1, 1, 1, 0, 0, 0x64, global<u16>(0x007cb24c));   // (sic: not checked)
    return window;
}

u8 *__cdecl List7cb264_NewRow() {
    u8 *const window = newRow(0, 0x64, cachedHeight(0x007cb264, fontAt(0x00752934)));
    for (u32 id = 2; id <= 6; id++) child(window, 1, 1, id);   // (sic: not checked)
    return window;
}

u8 *__cdecl List7cc250_NewRow() {
    u8 *const window = newRow(1, 0x64, cachedHeight(0x007cc250, fontAt(0x0075583c)));
    if (window) child(window, 1, 1, 1, 0x64, 0, 0xc8, global<u16>(0x007cc250));
    return window;
}

u8 *__cdecl List7cc654_NewRow() {
    const u32 height = rowHeight();
    u8 *const window = newRow(0, 0x64, height);
    if (window) {
        child(window, 1, 1, 1, 0x64, 0, 0xc8, 0x32);
        child(window, 6, 1, 2, 0x64, 0, 0xc8, 0x32);
        child(window, 6, 1, 2, 0x64, 0, 0xc8, 0x32);   // (sic: twice)
    }
    return window;
}

u8 *__cdecl List7ca158_NewRow() {
    u8 *const window = newRow(0, 0, cachedHeight(0x007ca158, fontAt(0x00750d54)));
    Window_SetHandler(window, nullptr, 0x004959c0);   // (sic: not checked)
    if (!window) return window;
    switch (global<u32>(0x007ca154)) {
    case 0:
        child(window, 1, 1, 4);
        child(window, 9, 0x11, 5);
        child(window, 9, 0x11, 6);
        child(window, 9, 0x11, 3);
        break;
    case 1:
        child(window, 1, 1, 3);
        child(window, 1, 1, 4);
        child(window, 1, 1, 5);
        child(window, 9, 0x11, 6);
        break;
    case 2:
        child(window, 1, 1, 2);
        child(window, 9, 0x11, 3);
        child(window, 1, 1, 4);
        child(window, 1, 1, 5);
        child(window, 9, 0x11, 6);
        break;
    case 3:
        child(window, 1, 1, 4);
        break;
    default:
        break;
    }
    return window;
}

u8 *__cdecl List7ca170_NewRow() {
    u8 *const window = newRow(0, 0, cachedHeight(0x007ca170, fontAt(0x00751184)));
    if (window) {
        child(window, 1, 1, 0x3e7);
        child(window, 1, 1, 0x3e7);   // (sic: two children 0x3e7)
        child(window, 9, 0x11, 4);
        child(window, 9, 0x11, 5);
    }
    return window;
}

u8 *__cdecl List7cc200_NewRow() {
    u8 *const window = newRow(0, 0x64, cachedHeight(0x007cc200, fontAt(0x00755134)));
    if (window) {
        Window_SetHandler(window, nullptr, 0x004acb70);
        Window_SetHandler(child(window, 1, 1, 1, 0x64, 0, 0xc8, 0x32), nullptr, 0x004acb70);   // (sic: not checked)
        child(window, 6, 1, 2, 0x64, 0, 0xc8, 0x32);
    }
    return window;
}

u8 *__cdecl List7ca314_NewRowJump() { return List7ca314_NewRow(); }

void __cdecl List7ca314_InitRow(u8 *row) {
    setPart(row, TextPart_Ctor);
    Window_Set4c(row, nullptr, Language_Font(global<u32>(0x007523dc)));
    u8 *const text = firstChild(row);
    setPart(text, TextPart6ff3b0_Ctor);
    Window_Set4c(text, nullptr, Language_Font(global<u32>(0x007523dc)));
    Window_NotifyC00(text, nullptr, 2, 1);
    vcall<void>(text, 0x1c, 0u);
}

void __cdecl List7cb24c_InitRow(u8 *row) {
    initRow(row);
    u8 *const text = firstChild(row);
    initRow(text);
    Window_NotifyC00(text, nullptr, 2, 1);
}

void __cdecl List7cc250_InitRow(u8 *row) {
    setPart(row, TextPart_Ctor);
    Window_Set4c(row, nullptr, Language_Font(global<u32>(0x0075583c)));
    Window_Notify40000(row, nullptr, 0xff, 0xff, 0xff, 0xff);
    u8 *const text = firstChild(row);
    setPart(text, TextPart6ff410_Ctor);
    Window_Set4c(text, nullptr, Language_Font(global<u32>(0x0075583c)));
    Window_NotifyC00(text, nullptr, 2, 1);
    vcall<void>(text, 0x1c, 0u);
}

void __cdecl List7cc654_InitRow(u8 *row) {
    setPart(row, TextPart_Ctor);
    Window_Notify40000(row, nullptr, 0xff, 0xff, 0xff, 0xff);
    Window_Set4c(row, nullptr, Language_Font(7));
    Window_NotifyImpl100(row, nullptr, 0);
    u8 *const text = firstChild(row);
    setPart(text, TextPart_Ctor);
    Window_NotifyC00(text, nullptr, 1, 1);
    Window_Notify3000(text, nullptr, 1, 1);
    Window_Set4c(text, nullptr, Language_Font(7));
    Window_Notify40000(text, nullptr, 0xff, 0xff, 0xff, 0xff);
    Window_NotifyImpl100(text, nullptr, 0);
    u8 *const box1 = nextSibling(text);
    Window_SetObject98(box1, nullptr, lookOf("i_boxtick"));
    u8 *const box2 = nextSibling(box1);
    Window_SetObject98(box2, nullptr, lookOf("i_boxtick"));
}

u8 *__cdecl List7cc94c_NewRow() {
    u8 *const window = newRow(0, 0, cachedHeight(0x007cc94c, &kFont7));
    for (u32 id = 2; id <= 6; id++) child(window, 1, 1, id);   // (sic: not checked)
    return window;
}

u8 *__cdecl List7cca90_NewRow() {
    u8 *const window = newRow(0, 0, cachedHeight(0x007cca90, &kFont7));
    for (u32 id = 2; id <= 5; id++) child(window, 1, 1, id);   // (sic: not checked)
    return window;
}

void __cdecl List7cc94c_InitRow(u8 *row) {
    textRow(row);
    u8 *window = firstChild(row);
    textRow(window);
    Window_NotifyC00(window, nullptr, 1, 1);
    window = nextSibling(window);
    textRow(window);
    Window_NotifyC00(window, nullptr, 1, 1);
    window = nextSibling(window);
    objectRow(window);
    for (int k = 0; k < 2; k++) {
        window = nextSibling(window);
        partRow(window);
    }
}

void __cdecl List7cca90_InitRow(u8 *row) {
    textRow(row);
    u8 *window = firstChild(row);
    textRow(window);
    window = nextSibling(window);
    objectRow(window);
    for (int k = 0; k < 2; k++) {
        window = nextSibling(window);
        partRow(window);
    }
}

void __cdecl List7cc200_InitRow(u8 *row) {
    setPart(row, TextPart_Ctor);
    Window_Set4c(row, nullptr, Language_Font(global<u32>(0x00755134)));
    Window_Notify40000(row, nullptr, 0xff, 0xff, 0xff, 0xff);
    u8 *const text = firstChild(row);
    setPart(text, TextPart6ff3e0_Ctor);
    Window_Set4c(text, nullptr, Language_Font(global<u32>(0x00755134)));
    Window_NotifyC00(text, nullptr, 2, 1);
    vcall<void>(text, 0x1c, 0u);
    u8 *const box = nextSibling(text);
    Window_SetObject98(box, nullptr, lookOf("i_boxtick"));
    vcall<void>(box, 0x1c, 0u);
}

void __cdecl List7cb264_InitRow(u8 *row) {
    setPart(row, TextPart_Ctor);
    Window_Set4c(row, nullptr, Language_Font(global<u32>(0x00752934)));
    Window_SetHandler(row, nullptr, 0x004a0710);
    static const PartCtor kParts[5] = {TextPart6ff3b0_Ctor, TextPart6ff350_Ctor, TextPart6ff440_Ctor, TextPart6ff3b0_Ctor, TextPart6ff3b0_Ctor};
    u8 *window = row;
    for (int k = 0; k < 5; k++) {
        window = k ? nextSibling(window) : firstChild(window);
        Window_SetHandler(window, nullptr, 0x004a0710);
        setPart(window, kParts[k]);
        const bool label = kParts[k] == TextPart6ff3b0_Ctor;
        if (label) Window_NotifyC00(window, nullptr, 2, 1);
        Window_Set4c(window, nullptr, Language_Font(global<u32>(0x00752934)));
        vcall<void>(window, 0x1c, 0u);
        if (!label) Window_NotifyC00(window, nullptr, 1, 1);
    }
}

void __cdecl List7ca158_InitRow(u8 *row) {
    setPart(row, TextPart_Ctor);
    Window_Set4c(row, nullptr, Language_Font(global<u32>(0x00750d54)));
    Window_NotifyImpl100(row, nullptr, 0);
    u8 *window = firstChild(row);
    const u32 state = global<u32>(0x007ca154);
    if (state > 3) return;
    setPart(window, TextPart6ff350_Ctor);
    Window_Set4c(window, nullptr, Language_Font(global<u32>(0x00750d54)));
    vcall<void>(window, 0x1c, 0u);
    Window_NotifyC00(window, nullptr, 1, 1);
    if (state == 3) return;
    // the next siblings by the state: 'b' a ridestatbar bar, 'l' a 0x6ff3b0 part (the bytes 2, 1 as 0xc00, the font); slot 7 with 0
    static const char *const kSteps[3] = {"bbb", "llb", "bllb"};
    for (const char *step = kSteps[state]; *step; step++) {
        window = nextSibling(window);
        if (*step == 'b') {
            panelparts::barOf(window, panelparts::kRideStatBar);
        } else {
            setPart(window, TextPart6ff3b0_Ctor);
            Window_NotifyC00(window, nullptr, 2, 1);
            Window_Set4c(window, nullptr, Language_Font(global<u32>(0x00750d54)));
        }
        vcall<void>(window, 0x1c, 0u);
    }
}

void __cdecl List7ca140_InitRow(u8 *row) {   // (sic: the original builds a GuiRect on its stack and destroys it, unused)
    setPart(row, TextPart6ff470_Ctor);
    Window_Set4c(row, nullptr, Language_Font(global<u32>(0x007508b4)));
    Window_NotifyC00(row, nullptr, 1, 1);
    u8 *window = firstChild(row);
    setPart(window, TextPart6ff3b0_Ctor);
    Window_Set4c(window, nullptr, Language_Font(global<u32>(0x007508b4)));
    vcall<void>(window, 0x1c, 0u);
    Window_NotifyC00(window, nullptr, 2, 1);
    window = nextSibling(window);
    setPart(window, TextPart6ff4a0_Ctor);
    Window_Set4c(window, nullptr, Language_Font(global<u32>(0x007508b4)));
    Window_NotifyC00(window, nullptr, 1, 1);
    window = nextSibling(window);
    setPart(window, TextPart6ff350_Ctor);
    Window_Set4c(window, nullptr, Language_Font(global<u32>(0x007508b4)));
    vcall<void>(window, 0x1c, 0u);
    window = nextSibling(window);
    Window_SaveInsetRect(window, 4, 0);
    Window_SetImpl(window, nullptr, lookOf("ti_hungry"));   // (sic: the registered window itself as the impl)
    panelparts::barOf(nextSibling(window), panelparts::kHappyGrad, 0, 8);
}

void __cdecl List7ca170_InitRow(u8 *row) {   // (sic: the original builds a GuiRect on its stack and destroys it, unused)
    setPart(row, TextPart_Ctor);
    Window_Set4c(row, nullptr, Language_Font(global<u32>(0x00751184)));
    Window_NotifyImpl100(row, nullptr, 0);
    u8 *window = firstChild(row);
    setPart(window, TextPart_Ctor);
    Window_Set4c(window, nullptr, Language_Font(global<u32>(0x00751184)));
    Window_NotifyImpl100(window, nullptr, 0);
    window = nextSibling(window);
    setPart(window, TextPart6ff3b0_Ctor);
    Window_NotifyC00(window, nullptr, 2, 1);
    Window_Set4c(window, nullptr, Language_Font(global<u32>(0x00751184)));
    vcall<void>(window, 0x1c, 0u);
    window = nextSibling(window);
    vcall<void>(panelparts::barOf(window, panelparts::kRideStatBar, 0, 8), 0x1c, 0x1f4u);
    panelparts::barOf(nextSibling(window), panelparts::kHappyGrad, 0, 8);
}
}  // extern "C"
