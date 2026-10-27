#include "dialog7cc5f4.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/keymap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../render/textparts.h"
#include "dialogbase.h"
#include "hoverlabel.h"
#include "window.h"
#include "window705a20.h"
#include "windowreg.h"

namespace {

u8 *&dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cc5f4); }

using Ctor = u8 *(__fastcall *)(u8 *, void *, u32);

// A new part (ctor; none when the allocation fails).
u8 *part(Ctor ctor) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    return memory ? ctor(memory, nullptr, 0) : nullptr;
}

// An entry field on the buffer (size bytes) with a new part (ctor), its text, and its label child with the language text caption.
void field(u8 *entry, u32 buffer, u32 size, Ctor ctor, u32 text, u32 label, u32 caption) {
    Window705a20_SetBuffer(entry, nullptr, buffer, size);
    Window_SetImpl(entry, nullptr, part(ctor));
    Window_Set4c(entry, nullptr, Language_Font(7));
    Window_Notify40000(entry, nullptr, 0xff, 0xff, 0xff, 0xff);
    Window_NotifyC00(entry, nullptr, 0, 0);
    Window705a20_SetCaret(entry, nullptr,
                          reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>((uintptr_t)0x0074fa84))));
    vcall<void>(entry, 0x24, text);
    u8 *const impl = part(TextPart_Ctor);
    Window_SetImpl(Window_FindChild(entry, nullptr, label), nullptr, impl);
    u8 *const font = Language_Font(7);
    Window_Set4c(Window_FindChild(entry, nullptr, label), nullptr, font);
    u16 *const wide = Language_WideText(caption);
    Window_NotifyImpl100(Window_FindChild(entry, nullptr, label), nullptr, (u32)(uintptr_t)wide);
}

// The child 0x4d: a TextPart6ff530, font 5, the language text caption, the yellow.
void yellow(u8 *window, u32 caption) {
    Window_SetImpl(window, nullptr, part(TextPart6ff530_Ctor));
    Window_Set4c(window, nullptr, Language_Font(5));
    Window_NotifyC00(window, nullptr, 1, 1);
    vcall<void>(window, 0x24, (u32)(uintptr_t)Language_WideText(caption));
    HoverLabel_SetYellow(window);
}

}  // namespace

void __cdecl Dialog7cc5f4_Create(u32 owner) {
    u8 *const built = Dialog_Create(reinterpret_cast<u8 *>((uintptr_t)0x007585a8), 0x004bb030, 0);
    *reinterpret_cast<u32 *>((uintptr_t)0x007cc5fc) = owner;
    dialog() = built;
    u8 *const name = Window_FindChild(built, nullptr, 0x49);
    field(name, 0x007cc590, 0x10, TextPart6ff4d0_Ctor, 0x0078a374, 0x4a, 0xed);
    field(Window_FindChild(dialog(), nullptr, 0x4b), 0x007cc548, 0x10, TextPart6ff380_Ctor, 0x0078a374, 0x4c, 0xee);
    Window_SetFocus(name, nullptr, 0);
    yellow(Window_FindChild(dialog(), nullptr, 0x4d), 0xec);
}

u32 __cdecl Dialog7cc5f4_Finish(u32 value) {
    Window_SendMessage(dialog(), 4, 0, 0);
    const auto owner = *reinterpret_cast<u32(__cdecl **)(u32)>((uintptr_t)0x007cc5fc);
    if (value == 0 || owner == nullptr) return 0;
    return owner(value);
}

void __cdecl Dialog7cc5f4_CreateFrom(u32 value, u8 *data) {
    u8 *const built = Dialog_Create(reinterpret_cast<u8 *>((uintptr_t)0x007583e8), 0x004b9d00, 0);
    u8 *const name = Window_FindChild(built, nullptr, 0x49);
    field(name, 0x007cc590, 0x20, TextPart6ff4d0_Ctor, (u32)(uintptr_t)(data + 0x234), 0x4a, 0xea);
    field(Window_FindChild(built, nullptr, 0x4b), 0x007cc548, 0x20, TextPart6ff380_Ctor, (u32)(uintptr_t)(data + 0x436), 0x4c, 0xeb);
    Window_SetFocus(name, nullptr, 0);
    yellow(Window_FindChild(built, nullptr, 0x4d), 0xe9);
    Window_SetDwordA0(built, nullptr, 0, value);
    KeyMaps_ResetAllAndSetFlag();
}
