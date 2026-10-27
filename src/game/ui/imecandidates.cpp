#include "imecandidates.h"

#include <cstdint>

#include "../core/ime.h"
#include "guidepth.h"
#include "guirect.h"
#include "listrows.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall ImeHost_OpenCandidates(u8 *host, void *, u8 *parent, u32 second) {
    at<u8 *>(host, 0) = WindowScript_Build(parent, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x0074f9e0), 0x00480d10);
    u8 anchor[0xc], own[0xc];
    GuiRect_Ctor(anchor);
    GuiRect_Ctor(own);
    Window_GetRect(at<u8 *>(host, 4), nullptr, anchor, 1);
    Window_GetRect(at<u8 *>(host, 0), nullptr, own, 1);
    const s32 width = at<s16>(own, 8) - at<s16>(own, 4);
    const s32 height = at<s16>(own, 0xa) - at<s16>(own, 6);
    s32 x = at<s16>(anchor, 4), y = at<s16>(anchor, 0xa);
    if (height + y > 0x5f8) {
        y = at<s16>(anchor, 6) - height;
        if (y < 0) {
            y = at<s16>(anchor, 6);
            if (height + y > 0x5f8) y = 0;
            x = at<s16>(anchor, 8);
        }
    }
    if (width + x > 0x7f8) x = 0x7f8 - width;
    Window_SetRectTo(at<u8 *>(host, 0), nullptr, (u32)x, (u32)y, (u32)(width + x), (u32)(height + y));
    Window_GetRect(at<u8 *>(host, 0), nullptr, own, 1);
    Window_SetDepth(at<u8 *>(host, 0), nullptr, (u32)Gui_TopDepth(*reinterpret_cast<u8 **>((uintptr_t)0x00faa5d8), own));
    u8 *const list = Window_FindChild(at<u8 *>(host, 0), nullptr, 0x7b79e3);
    Window7059f0_BuildRows(list, nullptr, &HilightList_NewRow, &HilightList_InitRow);   // (0x480d30, 0x480dd0: only called, not stored)
    Window7059f0_SetColumnBottom(list, nullptr, 0, 0);
    Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))));
    at<u8 *>(host, 8) = list;
    at<u32>(host, 0x10) = 0;
    at<u32>(host, 0x14) = 0;
    at<u32>(host, 0x18) = 0;
    Window7059f0_Clear(list);
    u8 *const ime = host + 0x1c;
    const s32 count = (s32)Ime_CacheCurrent(ime);
    u32 *const texts = reinterpret_cast<u32 *>(&parent);   // (the texts: the parent's slot, then the second argument's, sic)
    for (s32 i = 0; i < count; ++i) {
        u32 text = 0;
        if (at<u8>(ime, 0x184)) {
            const u8 *const first = at<u8 *>(ime, 0x174);
            const u32 n = first ? (u32)((s32)(at<u8 *>(ime, 0x178) - first) >> 4) : 0;
            if ((u32)i < n) {
                text = at<u32>(first + 0x10 * i, 4);
                if (!text) text = 0x006fd6c4;
            }
        }
        texts[0] = text;
        Window7059f0_AddItem(at<u8 *>(host, 8), nullptr, texts, (u32)i, -1, 0);
    }
    (void)second;
    Window7059f0_RefreshRows(at<u8 *>(host, 8));
    GuiRect_Dtor(own);
    GuiRect_Dtor(anchor);
}
}  // extern "C"
