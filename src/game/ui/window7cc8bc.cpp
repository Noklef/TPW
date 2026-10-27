#include "window7cc8bc.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../frontend/helpbg.h"
#include "../render/textparts.h"
#include "guirect.h"
#include "listrows.h"
#include "radiogroup.h"
#include "window.h"
#include "window7059f0.h"
#include "window705a20.h"
#include "windowfactory.h"
#include "windowreg.h"
#include "windowscript.h"

namespace {

u8 *window(uintptr_t global) { return *reinterpret_cast<u8 **>(global); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *hilight() { return reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))); }

// [slot] built hidden on the GUI root from script with the handler 0x4bed60, unlinked, its child id the handler 0x4bede0.
void buildHidden(u32 script, u32 slot, u32 id) {
    global<u8 *>(slot) = WindowScript_Build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>((uintptr_t)script), 0x004bed60);
    Window_SetShown(global<u8 *>(slot), nullptr, 0);
    Window_Unlink(global<u8 *>(slot));
    Window_SetHandler(Window_FindChild(global<u8 *>(slot), nullptr, id), nullptr, 0x004bede0);
}

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

void __cdecl Window7cc8bc_Create() {
    global<u32>(0x007cc8d0) = 0;
    u8 *const page = WindowScript_Build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00758ae8), 0x004bd4a0);
    global<u8 *>(0x007cc8bc) = page;
    if (page) {
        u8 *const names = Window_FindChild(page, nullptr, 0xc3123);
        Window_SetHandler(names, nullptr, 0x004be290);
        u8 *list = Window_FindChild(names, nullptr, 0xc3124);
        Window7059f0_BuildRows(list, nullptr, &List758f04_NewRowA, &List758f04_InitRowJump);   // (0x4bf740, 0x4bf800: only called)
        Window7059f0_SetColumnBottom(list, nullptr, 0, 0);
        Window7059f0_SetHighlight(list, nullptr, hilight());
        Window_SetHandler(list, nullptr, 0x004be820);
        RadioGroup_Select(Window_FindChild(list, nullptr, 0xc3125), nullptr, 0xc3126);
        list = Window_FindChild(names, nullptr, 0xc312b);
        Window7059f0_BuildRows(list, nullptr, &List758f04_NewRowB, &List758f04_InitRowJumpB);   // (0x4bf8b0, 0x4bf8d0)
        Window7059f0_SetColumnBottom(list, nullptr, 0, 0);
        u8 *const edit = Window_FindChild(list, nullptr, 0xc312c);
        Window_SetHandler(edit, nullptr, 0x004be3a0);
        u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        Window_SetImpl(edit, nullptr, memory ? TextPart6ff4d0_Ctor(memory, nullptr, 0) : nullptr);
        Window_Set4c(edit, nullptr, Language_Font(global<u32>(0x00758f08)));
        Window705a20_SetBuffer(edit, nullptr, 0x007cc6a8, 0xff);
        Window705a20_SetCaret(edit, nullptr, hilight());
        vcall<void>(edit, 0x24, 0x0078a374u);
        global<u8>(0x007cc8b4) = 1;
        u8 rect[12];
        GuiRect_Ctor(rect);
        Window_GetRect(global<u8 *>(0x007cc8bc), nullptr, rect, 1);
        global<u32>(0x007a1c54) = global<u32>(0x007a192c);
        global<u32>(0x007a1c58) = global<u32>(0x007a194c);
        global<u32>(0x0074d108) = global<u32>(0x007a162c);
        global<u32>(0x0074d104) = global<u32>(0x007a0c9c);
        GuiRect_Dtor(rect);
    }
    buildHidden(0x00758cd8, 0x007cc8c4, 0x1789);
    buildHidden(0x00758e18, 0x007cc8c8, 0x3003);
    global<u32>(0x007cc8cc) = 0;
    global<u8 *>(0x007cc8c0) = Window_CreateChild(global<u8 *>(0x00faa5d8), nullptr, 1, 0, 0xfff04d2a, 0, 0, 0x7ff, 0x5ff);
    Window_SetHandler(global<u8 *>(0x007cc8c0), nullptr, 0x004bd4a0);
    HelpBg_Place(0xffffffff, 10);
    global<u32>(0x007cc8e4) = 0;
    global<u8>(0x007cc8d4) = 0;
}
