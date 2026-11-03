#include "dialog7cc1f8.h"

#include <cstdint>

#include "../core/globalsave.h"
#include "../core/language.h"
#include "../core/rings.h"
#include "../core/textarg.h"
#include "../files/wrapsources.h"
#include "../frontend/helpbg.h"
#include "../world/objectdefs.h"
#include "../world/objectunlock.h"
#include "../world/world.h"
#include "guimanager.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window.h"
#include "window7059f0.h"

using namespace panelparts;

namespace {
template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

const Dialog7cc1f8Env kGameEnv = {&Window7059f0_Clear, &Window7059f0_AddItem, &Window7059f0_RefreshRows, &ObjectDefs_GetFullName, &Language_WideText};
const Dialog7cc1f8Env *g_env = &kGameEnv;
}  // namespace

void Dialog7cc1f8_SetEnv(const Dialog7cc1f8Env *env) { g_env = env ? env : &kGameEnv; }

namespace {
const Dialog7cc1f8TabEnv kTabGameEnv = {&Dialog7cc1f8_ShowStats, &Dialog7cc1f8_FillList, &Language_WideText, &HoverLabel_Attach, &Window_CallDefault};
const Dialog7cc1f8TabEnv *g_tab = &kTabGameEnv;
}  // namespace

void Dialog7cc1f8_SetTabEnv(const Dialog7cc1f8TabEnv *env) { g_tab = env ? env : &kTabGameEnv; }

u32 __cdecl Dialog7cc1f8_TabHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x101 && b == 1 && a - 0x1fa <= 3) {
        static const u32 kKinds[4] = {2, 0, 3, 1};
        const u32 kind = kKinds[a - 0x1fa];
        g_tab->showStats(kind);
        g_tab->fillList(kind, 1);
        const u16 *const text = g_tab->text(0x77 + kind);
        g_tab->attach(*reinterpret_cast<u8 **>((uintptr_t)0x007cc1f8), 0x1fe, (u32)(uintptr_t)text);
    }
    return (u32)g_tab->callDefault(window, nullptr, message, a, b);
}

extern "C" {

void __cdecl Dialog7cc1f8_FillList(u32 kind, u32 clear) {
    u8 *const game = *reinterpret_cast<u8 **>((uintptr_t)0x007cf83c);
    u8 *const store = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(game));
    u8 *const dialog = *reinterpret_cast<u8 **>((uintptr_t)0x007cc1f8);
    if (!dialog) return;
    u8 *const list = Window_FindChild(dialog, nullptr, 0x1f8);
    if (!list) return;
    if (clear) g_env->clear(list);
    u8 *const defs = reinterpret_cast<u8 *>((uintptr_t)0x007890a0);
    u8 *def = ObjectDefs_First(defs);
    *reinterpret_cast<u32 *>((uintptr_t)0x007cc1fc) = kind;
    struct Row {
        u32 text, value, state;
    } row;
    for (; def; def = ObjectDefs_Next(defs)) {
        if (at<u32>(def, 0x508) || at<u32>(def, 0x4ac) != kind || at<u32>(def, 0x284)) continue;
        const u16 id = at<u16>(def, 0);
        u8 *const control = ObjectControls_Find(*reinterpret_cast<u8 **>((uintptr_t)0x007cf83c) + 0x1d943c, nullptr, id, 0);
        if (!at<u8>(control, 0x10)) continue;
        u16 name[0x42];
        const s32 price = at<s32>(def, 0xc4);
        if (price > 0 && !(u8)GlobalSave_HasKey(id)) {
            row.text = (u32)(uintptr_t)g_env->text(0x89);
            row.value = (u32)-price;
        } else {
            g_env->fullName(defs, nullptr, id, name, 0x42);
            row.text = (u32)(uintptr_t)name;
            row.value = at<u32>(control, 4);
        }
        if ((s32)ObjectControls_GetWord18(id) > 0) {
            row.state = 1;
        } else {
            row.state = 0;
            for (s32 i = 0; i < 3 && !row.state; ++i) {
                u16 latest;
                if (*Ring16_Get(store, nullptr, &latest, (s32)kind, i) == id) row.state = 2;
            }
        }
        g_env->addItem(list, nullptr, &row.text, id, -1, 0);
    }
    if (kind == 3) {
        row.text = (u32)(uintptr_t)g_env->text(0x86);
        row.value = *reinterpret_cast<u32 *>((uintptr_t)0x0078560c);
        row.state = 0;
        g_env->addItem(list, nullptr, &row.text, 0xffffffff, -1, 0);
        row.text = (u32)(uintptr_t)g_env->text(0x87);
        row.value = 0;
        row.state = 0;
        g_env->addItem(list, nullptr, &row.text, 0xfffffffe, -1, 0);
    }
    g_env->refresh(list);
}

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
