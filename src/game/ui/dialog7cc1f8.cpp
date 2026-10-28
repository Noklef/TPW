#include "dialog7cc1f8.h"

#include <cstdint>

#include "../core/globalsave.h"
#include "../core/textarg.h"
#include "../files/wrapsources.h"
#include "../frontend/helpbg.h"
#include "../world/objectdefs.h"
#include "../world/objectunlock.h"
#include "../world/world.h"
#include "guimanager.h"
#include "guirect.h"
#include "panelparts.h"

using namespace panelparts;

extern "C" {

void __cdecl Dialog7cc1f8_DressStats() {
    u8 *const stats = Window_FindChild(*reinterpret_cast<u8 **>((uintptr_t)0x007cc1f8), nullptr, 0x1ed);
    if (!stats) return;
    text(stats, 0x1f4, 0x7d);
    text(stats, 0x1f3, 0x7e);
    text(stats, 0x1f1, 0x7f);
    text(stats, 0x1f6, 0x80);
    text(stats, 0x1f7, 0x81);
    text(stats, 0x1f5, 0x88);
    text(stats, 0x1f2, 0x88);   // (sic: the same text as 0x1f5's)
    bar(stats, kRideStatBar, 0x1ee, 0x1f4);
    bar(stats, kRideStatBar, 0x1ef, 0xc8);
    bar(stats, kRideStatBar, 0x1f0, 0x64);
}

namespace {

// The stats window's child id shown or hidden (nothing when it has none).
void Show(u8 *stats, u32 id, u32 shown) {
    if (u8 *const child = Window_FindChild(stats, nullptr, id)) Window_SetShown(child, nullptr, shown);
}

}  // namespace

void __cdecl Dialog7cc1f8_ShowStats(u32 mode) {
    u8 *const stats = Window_FindChild(*reinterpret_cast<u8 **>((uintptr_t)0x007cc1f8), nullptr, 0x1ed);
    if (!stats) return;
    static const u32 kIds[8] = {0x1f3, 0x1f1, 0x1f6, 0x1f7, 0x1ee, 0x1f2, 0x1ef, 0x1f0};
    switch (mode) {
    case 0:
    case 1:
    case 3:
        for (const u32 id : kIds) Show(stats, id, mode == 0);
        break;
    case 2:
        Show(stats, 0x1f3, 1);
        Show(stats, 0x1f1, 0);
        Show(stats, 0x1f6, 0);
        Show(stats, 0x1f6, 0);   // (sic: twice)
        Show(stats, 0x1f7, 0);
        Show(stats, 0x1ee, 1);
        Show(stats, 0x1f2, 0);
        Show(stats, 0x1ef, 0);
        Show(stats, 0x1f0, 0);
        break;
    default:
        break;
    }
}

void __cdecl Dialog7cc1f8_ShowItemTip(const s16 *item) {
    u8 *const list = Window_FindChild(*reinterpret_cast<u8 **>((uintptr_t)0x007cc1f8), nullptr, 0x1f8);
    u8 rect[0x10];
    GuiRect_Ctor(rect);
    Window_GetSavedRect(list, nullptr, rect);
    u16 x, y;
    Gui_GetMousePos(&x, &y);
    s32 &state = *reinterpret_cast<s32 *>((uintptr_t)0x0075513c);
    s16 &tipped = *reinterpret_cast<s16 *>((uintptr_t)0x007cc1e0);
    void *const texts = *reinterpret_cast<void **>((uintptr_t)0x0078ba98);
    u16 text[0x80];   // (left as it is for the items -3 and below, sic: the stack's)
    if (!(u16)GuiRect_Contains(rect, nullptr, x, y)) {
        state = -1;
    } else if (*item < 0) {
        if (tipped != *item) {
            state = 4;
            tipped = *item;
            if (*item == -1) TextFormat().format(texts, text, 0x80, 0x21a);
            else if (*item == -2) TextFormat().format(texts, text, 0x80, 0x21b);
            HelpBg_SetText(text);
        }
    } else {
        u8 *const game = *reinterpret_cast<u8 **>((uintptr_t)0x007cf83c);
        const u8 *const control = ObjectControls_Find(game + 0x1d943c, nullptr, (u16)*item, 0);
        const s32 balance = (s32)Source_Field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(game)));
        u8 *const defs = reinterpret_cast<u8 *>((uintptr_t)0x007890a0);
        s32 now;
        if (*reinterpret_cast<const u32 *>(ObjectDefs_Find(defs, nullptr, (u16)*item) + 0xc4) && !(u8)GlobalSave_HasKey((u16)*item))
            now = ObjectDef_IsUnlocked((u16)*item) ? 0 : 1;
        else
            now = (*reinterpret_cast<const s32 *>(control + 4) <= balance) + 2;
        if (now != state || tipped != *item) {
            state = now;
            tipped = *item;
            if (now == 0) {
                TextFormat().format(texts, text, 0x80, 0x97);
            } else if (now == 1) {
                TextFormat().format(texts, text, 0x80, 0x98);
            } else {   // (2: too dear, 3: affordable) the definition's name (+0x90) as the argument 0
                u16 name[0x40];
                TextFormat().format(*reinterpret_cast<void **>((uintptr_t)0x0078bad0), name, 0x3f,
                                    *reinterpret_cast<const u32 *>(ObjectDefs_Find(defs, nullptr, (u16)*item) + 0x90));
                u8 arg[0x14];
                TextArgString_Ctor(arg, nullptr, 0, (u32)(uintptr_t)name);
                TextFormat().format(texts, text, 0x80, now == 2 ? 0x96 : 0x94, arg);
            }
            HelpBg_SetText(text);
        }
    }
    GuiRect_Dtor(rect);
}
}
