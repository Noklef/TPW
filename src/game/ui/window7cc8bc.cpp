#include "window7cc8bc.h"

#include <cstdint>

#include "window.h"
#include "window7059f0.h"

namespace {

u8 *window(uintptr_t global) { return *reinterpret_cast<u8 **>(global); }

// Whether the window [0x7cc8bc] is set and the mode [0x7cc8d0] is mode.
bool ready(u32 mode) { return window(0x007cc8bc) && *reinterpret_cast<const u32 *>((uintptr_t)0x007cc8d0) == mode; }

// The window [0x7cc8bc]'s list (its child 0xc3123's child 0xc3124; not checked).
u8 *list() { return Window_FindChild(Window_FindChild(window(0x007cc8bc), nullptr, 0xc3123), nullptr, 0xc3124); }

void add(u32 mode, const u16 *text, u32 data) {
    if (!ready(mode)) return;
    const u32 texts[1] = {(u32)(uintptr_t)text};
    Window7059f0_AddItem(list(), nullptr, texts, data, -1, 1);
}

void remove(u32 mode, const u16 *text) {
    if (!ready(mode)) return;
    u8 *const items = list();
    const s32 n = Window7059f0_FindText(items, nullptr, text, 1);
    if (n > -1) Window7059f0_RemoveItem(items, nullptr, n);
}

void setData(u32 mode, const u16 *text, u32 data) {
    if (!ready(mode)) return;
    u8 *const items = list();
    const s32 n = Window7059f0_FindText(items, nullptr, text, 1);
    if (n > -1) {
        Window7059f0_SetNthItemData(items, nullptr, n, data);
        Window7059f0_RefreshRows(items);
    }
}

}  // namespace

void __cdecl Window7cc8bc_ListAdd(const u16 *text, u32 data) { add(0, text, data); }
void __cdecl Window7cc8bc_ListAdd1(const u16 *text, u32 data) { add(1, text, data); }
void __cdecl Window7cc8bc_ListRemove(const u16 *text) { remove(0, text); }
void __cdecl Window7cc8bc_ListRemove1(const u16 *text) { remove(1, text); }
void __cdecl Window7cc8bc_ListSetData(const u16 *text, u32 data) { setData(0, text, data); }
void __cdecl Window7cc8bc_ListSetData1(const u16 *text, u32 data) { setData(1, text, data); }

void __stdcall Window7cc8bc_ListAddStd(const u16 *text, u32 data) { Window7cc8bc_ListAdd(text, data); }
void __stdcall Window7cc8bc_ListRemoveStd(const u16 *text) { Window7cc8bc_ListRemove(text); }
void __stdcall Window7cc8bc_ListSetDataStd(const u16 *text, u32 data) { Window7cc8bc_ListSetData(text, data); }
void __stdcall Window7cc8bc_ListAdd1Std(const u16 *text, u32 data) { Window7cc8bc_ListAdd1(text, data); }
void __stdcall Window7cc8bc_ListRemove1Std(const u16 *text) { Window7cc8bc_ListRemove1(text); }
void __stdcall Window7cc8bc_ListSetData1Std(const u16 *text, u32 data) { Window7cc8bc_ListSetData1(text, data); }

s32 __cdecl Windows7cc8c4_Handler(u8 *window_, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        u32 *const first = reinterpret_cast<u32 *>((uintptr_t)0x007cc8c4);
        if (reinterpret_cast<u8 *>((uintptr_t)*first) == window_)
            *first = 0;
        else
            *reinterpret_cast<u32 *>((uintptr_t)0x007cc8c8) = 0;
    } else if (message == 0x10006) {
        Window_SetShown(window(0x007cc8cc), nullptr, 0);
        Window_Unlink(window(0x007cc8cc));
        Window_SetFocus(window(0x007cc8bc), nullptr, 0);
    }
    return Window_CallDefault(window_, nullptr, message, a, b);
}
