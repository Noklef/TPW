#include "window7cc8bc.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../frontend/helpbg.h"
#include "../math/x87.h"
#include "../online/chat.h"
#include "../online/chatavatar.h"
#include "../render/textparts.h"
#include "cursor.h"
#include "guimanager.h"
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

// The chat [0x7cddec], made first when there is none (0xf0c bytes, OnlineChat_Ctor; none when the allocation fails: kept, sic).
u8 *chat() {
    u8 *&chat = global<u8 *>(0x007cddec);
    if (!chat) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0xf0c));
        chat = block ? OnlineChat_Ctor(block) : nullptr;
    }
    return chat;
}

}  // namespace

void __cdecl Window7cc8bc_EnterChat() {
    global<u32>(0x007cc8b8) = global<u8>(0x007cc8b4) ? global<u32>(0x007cc8bc) : global<u32>(0x007cc8c0);
    u8 rect[12];
    GuiRect_Ctor(rect);
    Window_GetRect(window(0x007cc8c0), nullptr, rect, 1);
    Window_SendMessage(window(0x007cc8bc), 6, 0, 0);
    Window_SendMessage(window(0x007cc8c0), 6, 0, 0);
    u32 last = 0;   // (the last __ftol's low dword: EAX's high word is pushed with x below)
    const auto pack = [&last](u32 from, u8 *out) {
        const X87Reg tenths = X87Reg(global<float>(from)) * 0.100000001f;  // 0x700184
        const s32 whole = (s32)(long long)tenths.v;  // __ftol
        out[1] = (u8)whole;
        last = (u32)(s32)(long long)((tenths - X87Reg((double)(whole & 0xff))) * 256.0f).v;  // 0x700188
        out[0] = (u8)last;
    };
    pack(0x007a1c70, &global<u8>(0x007cc690));
    pack(0x007a1c78, &global<u8>(0x007cc694));
    const bool had = global<u8 *>(0x007cddec) != nullptr;
    u8 *const avatar = chat();
    // x's high word EAX's (the __ftol's, or the new chat's); y's 0 (EDX's after a new chat is whatever the allocation left: 0 here)
    const u32 high = had ? last : (u32)(uintptr_t)avatar;
    ChatAvatar_Arrive(avatar, nullptr, (high & 0xffff0000u) | global<u16>(0x007cc690), global<u16>(0x007cc694));
    if (global<u8>(0x00790ab0) & 4) ChatAvatar_SayPosition(chat());
    global<u8>(0x007cc8d4) = 1;
    global<u32>(0x007cc8ac) = global<u32>(0x00790ab0);
    global<u32>(0x007cc8d8) = 5;
    GuiRect_Dtor(rect);
}

void __cdecl Window7cc8bc_LeaveChat() {
    if (global<u32>(0x00790ab0) & 0x400) return;
    const bool first = global<u32>(0x007cc8b8) == global<u32>(0x007cc8bc);
    Window_SendMessage(window(0x007cc8bc), 6, first, 0);
    Window_SendMessage(window(0x007cc8c0), 6, !first, 0);
    global<u32>(0x007cc8b8) = 0;
    if (window(0x007cc8bc)) Window_SetCursor(window(0x007cc8bc), nullptr, 0);
    if (window(0x007cc8c0)) Window_SetCursor(window(0x007cc8c0), nullptr, 0);
    u32 last = 0;   // (the last __ftol's: y's high word)
    const auto pack = [&last](u32 from, u8 *out) {   // (as Window7cc8bc_EnterChat's)
        const X87Reg tenths = X87Reg(global<float>(from)) * 0.100000001f;  // 0x700184
        const s32 whole = (s32)(long long)tenths.v;  // __ftol
        out[1] = (u8)whole;
        last = (u32)(s32)(long long)((tenths - X87Reg((double)(whole & 0xff))) * 256.0f).v;  // 0x700188
        out[0] = (u8)last;
    };
    pack(0x00790aa0, &global<u8>(0x007cc690));
    pack(0x00790aa8, &global<u8>(0x007cc694));
    ChatAvatar_Enter(chat(), nullptr, global<u16>(0x007cc690), (last & 0xffff0000u) | global<u16>(0x007cc694));
    global<u8>(0x007cc8d4) = 0;
    // (fraction / 256 + whole) * 10, exact before the store)
    global<float>(0x00790ad0) = (float)(((double)global<u8>(0x007cc690) * 0.00390625 + global<u8>(0x007cc691)) * 10.0);
    global<float>(0x00790ad8) = (float)(((double)global<u8>(0x007cc694) * 0.00390625 + global<u8>(0x007cc695)) * 10.0);
}

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

void __cdecl Window7cc8cc_PlaceAtMouse(u32, u8 *window) {
    u8 *const placed = global<u8 *>(0x007cc8cc);
    u8 *const list = Window_FindChild(Window_FindChild(global<u8 *>(0x007cc8bc), nullptr, 0xc3123), nullptr, 0xc3124);
    Window_AddChild(list, nullptr, placed, 1, 1);
    u8 rect[12];
    GuiRect_Ctor(rect);
    Window_GetRect(window, nullptr, rect, 1);
    u16 x, y;
    Gui_GetMousePos(&x, &y);
    Window_Offset(window, nullptr, (u16)(x - *reinterpret_cast<u16 *>(rect + 4)), (u16)(y - *reinterpret_cast<u16 *>(rect + 6)));
    Window_SetDepth(global<u8 *>(0x007cc8cc), nullptr, (u16)(*reinterpret_cast<u16 *>(global<u8 *>(0x007cc8cc) + 0xe0) + 4));
    Window_SetShown(global<u8 *>(0x007cc8cc), nullptr, 1);
    Window_SetFocus(global<u8 *>(0x007cc8cc), nullptr, 0);
    GuiRect_Dtor(rect);
}

void __cdecl Window7cc8bc_ChatAdd(u16 *text, u32 data) {
    if (!global<u8 *>(0x007cc8bc)) return;
    u8 *const list = Window_FindChild(Window_FindChild(window(0x007cc8bc), nullptr, 0xc3123), nullptr, 0xc312b);
    u8 *const font = Language_Font(global<u32>(0x00758f04));
    u16 first = 0, second = 0;   // (sic: the original's are the stack's leftovers when ColumnSpan fails)
    Window7059f0_ColumnSpan(list, nullptr, 0, &first, &second);
    const s32 width = (s32)((u32)((s32)(s16)second - (s16)first) * (u32)(s32)global<s16>(0x00faa5c4)) / 2048;
    for (s32 start = 0;;) {
        const s32 length = (s32)Crt().wcslen(text);
        s32 last = -1;
        if (start < length) {
            s32 taken = (s16)vcall<u16>(font, 0xc, (u32)text[start]);
            if (taken < width) {
                for (s32 j = start; j < length;) {
                    const u16 c = text[j + 1];
                    last = j++;
                    if (c) taken += (s16)vcall<u16>(font, 0xc, (u32)c);
                    if (taken >= width) break;
                }
            }
        }
        if (last == -1) break;
        // the line text[start..last] added as the list's one column, the text cut after it for the while
        const u16 kept = text[last + 1];
        text[last + 1] = 0;
        const u32 texts[1] = {(u32)(uintptr_t)(text + start)};
        Window7059f0_AddItem(list, nullptr, texts, data, -1, 1);
        text[last + 1] = kept;
        start = last + 1;
    }
    for (s32 count = *reinterpret_cast<const s32 *>(list + 0x150); count >= global<s32>(0x00758f0c); --count)
        Window7059f0_RemoveItem(list, nullptr, 0);
    Window7059f0_ScrollTo(list, nullptr, *reinterpret_cast<const s32 *>(list + 0x150) - 1);
}

void __stdcall Window7cc8bc_ChatAddStd(u16 *text, u32 data) { Window7cc8bc_ChatAdd(text, data); }
