#include "dialog7cc1f8.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/addrcalls.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/language.h"
#include "../core/rings.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../files/wrapsources.h"
#include "../frontend/helpbg.h"
#include "../render/renderer.h"
#include "../render/textparts.h"
#include "../world/advisor.h"
#include "../world/mousemode.h"
#include "../world/objectdefs.h"
#include "../world/objectunlock.h"
#include "../world/world.h"
#include "buttontext.h"
#include "dialogs.h"
#include "focus.h"
#include "guimanager.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "listrows.h"
#include "panelparts.h"
#include "radiogroup.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"
#include "panel7b9940.h"
#include "dialog7cc188.h"

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

namespace {
const Dialog7cc1f8OpenEnv kOpenGameEnv = {&Dialog7cc1f8_FillList, &Charset_FormatWide, &RadioGroup_Select, &HoverLabel_Attach, &Window7059f0_FindItemData, &Window7059f0_ScrollTo, &Window7059f0_BuildRows};
const Dialog7cc1f8OpenEnv *g_open = &kOpenGameEnv;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void selectKind(u8 *list, u32 kind) {
    u8 *const group = Window_FindChild(list, nullptr, 0x1f9);
    static const u32 kButtons[4] = {0x1fb, 0x1fd, 0x1fa, 0x1fc};
    if (group && kind <= 3) g_open->selectRadio(group, nullptr, kButtons[kind]);
}

void scrollToSelected(u8 *list, u32 selected) {
    if (!(u16)selected) return;
    const s32 position = g_open->findItem(list, nullptr, selected & 0xffff);
    if (position >= 0) g_open->scrollTo(list, nullptr, position);
}

u8 *raised(u32 id) { return Window_FindChild(Window_FindChild(global<u8 *>(0x007cc1f8), nullptr, 0x1ea), nullptr, id); }
void raise(u8 *child) { Window_SetDepth(child, nullptr, (u16)(at<u16>(child, 0xe0) + 0xc)); }
}  // namespace

void Dialog7cc1f8_SetOpenEnv(const Dialog7cc1f8OpenEnv *env) { g_open = env ? env : &kOpenGameEnv; }

void __cdecl Dialog7cc1f8_Open(u32 kind, u32 selected) {
    Global7cb28c_Select(1, 1);
    if (global<u8 *>(0x007cc1f8)) {
        if (kind == 0xffffffff) kind = global<u32>(0x007cc1fc);
        g_open->fillList(kind, 1);
        u8 *const list = Window_FindChild(global<u8 *>(0x007cc1f8), nullptr, 0x1f8);
        if (!list) return;
        selectKind(list, kind);
        scrollToSelected(list, selected);
        return;
    }
    Dialog7cb2b8_ResetChild21(1);
    Window7c24c8_Close();
    gDialog7cc1f8_Open_7cc1f0 = 0;
    reinterpret_cast<u16 &>(gDialog7cc1f8_Open_7cc1e4) = 0;
    gDialog7cc1f8_Open_7cc1e8 = 0;
    global<u16>(0x007cc1e0) = 0;
    global<u8 *>(0x007cc1f8) = WindowScript_Build(gPanel7b9940_7cb2ac, nullptr, gDialog7cc1f8_Open_754cf8, 0x004ac270);
    if (global<u8 *>(0x007cc1f8)) {
        Window7c24c8_Set(global<u8 *>(0x007cc1f8));
        // the money line: a number argument 13, its size, in the text 0x1cb when negative (0x1ca else)
        u8 *child = Window_FindChild(global<u8 *>(0x007cc1f8), nullptr, 0x200);
        const s32 money = (s32)Source_Field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(gWorldPtr)));
        u32 number[5] = {(u32)(uintptr_t)TextArgInt_vtable, 0x5641524d, 0xd, money < 0 ? 0u - (u32)money : (u32)money, 0};
        g_open->format(global<u8 *>(0x0078ba94), reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff, money < 0 ? 0x1cbu : 0x1cau, number);
        u8 *memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        Window_SetObject98(child, nullptr, memory ? TextPart6ff530_Ctor(memory, nullptr, 0) : nullptr);
        Window_NotifyC00(child, nullptr, 2, 1);
        Window_Set4c(child, nullptr, Language_Font(5));
        Window_Notify40000(child, nullptr, 0xff, 0xff, 0xff, 0xff);
        vcall<void>(child, 0x24, 0x007ba058u);
        child = raised(0x1eb);
        memory = static_cast<u8 *>(GameHeap_Alloc(0xd8));
        Window_SetImpl(child, nullptr, memory ? RendererPart_Ctor(memory, nullptr, 0) : nullptr);
        raise(child);
        child = raised(0x1ec);
        memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        Window_SetImpl(child, nullptr, memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr);
        Window_NotifyC00(child, nullptr, 1, 1);
        Window_Notify3000(child, nullptr, 1, 1);
        Window_Set4c(child, nullptr, Language_Font(6));
        Window_Notify40000(child, nullptr, 0xff, 0xff, 0xff, 0xff);
        Window_NotifyImpl100(child, nullptr, 0);
        raise(child);
        if (u8 *const list = Window_FindChild(global<u8 *>(0x007cc1f8), nullptr, 0x1f8)) {
            Window_SetHandler(list, nullptr, 0x004aca60);
            g_open->buildRows(list, nullptr, reinterpret_cast<u8 *(__cdecl *)()>((uintptr_t)0x004ad240), reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)0x004ad340));   // (List7cc200_NewRow, List7cc200_InitRow, by the original's addresses: the hooks)
            Window7059f0_SetColumnBottom(list, nullptr, 0, 0);
            Window7059f0_SetColumnBottom(list, nullptr, 1, 1);
            Window7059f0_SetColumnBottom(list, nullptr, 2, 1);
            Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>((u32)(uintptr_t)&gText_Dialog7cc188_74fa84))));
            Window7059f0_SortBy(list, nullptr, gDialog7cc1f8_Open_755140, 0);
            static const u32 kTexts[3] = {0x7b, 0x7c, 0x8a};
            for (u32 i = 0; i < 3; ++i) {
                u8 *const button = Window_FindChild(list, nullptr, 0x10 + i);
                ButtonText_Init(button, (u32)(uintptr_t)Language_WideText(kTexts[i]), gDialog7cc1f8_Open_755138, 0xff, 0xef, 0, 0x90 + i);
            }
            if (kind == 0xffffffff) kind = global<u32>(0x007cc1fc);
            selectKind(list, kind);
            g_open->fillList(kind, 0);
            if (kind <= 3) g_open->attach(global<u8 *>(0x007cc1f8), 0x1fe, (u32)(uintptr_t)Language_WideText(0x77 + kind));
            scrollToSelected(list, selected);
        }
    }
    Advisor_PostId(0xbc);
}

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

namespace {
AddrCalls g_handler;

u32 CloseDialog() { return g_handler.Cd(0x00658d9f, global<u32>(0x007cc1f8), 4u, 0u, 0u); }   // Window_SendMessage (4, 0, 0)
u32 Child(u8 *window, u32 id) { return g_handler.Th(0x0065d7d0, (u32)(uintptr_t)window, id); }   // Window_FindChild
s32 Balance() { return (s32)g_handler.Th(0x006ad810, g_handler.Th(0x005195d0, (u32)(uintptr_t)gWorldPtr)); }   // Source_Field0c of the money record

// A 0x14-byte tile mode (none when the allocation fails) made current, the holder made first; then Advisor_PostId(advice).
void SetTileMode(u32 kind, u32 arg, u32 advice) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x14));
    u8 *const mode = !block ? nullptr : kind ? MouseModeTile_CtorKind(block, nullptr, kind, (u16)arg) : MouseModeTile_Ctor(block, nullptr, arg);
    MouseModes_EnsureHolder();
    g_handler.St(0x0046c350, (u32)(uintptr_t)mode);   // MouseMode_Set
    if (advice) g_handler.Cd(0x00486b00, advice);
}
}  // namespace

void Dialog7cc1f8_SetHandlerCalls(const AddrCall *table, u32 count) { g_handler.Set(table, count); }

s32 __cdecl Dialog7cc1f8_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 5:
        CloseDialog();
        return 1;
    case 0x10:
        if (a == 0x80080) {
            g_handler.Th(0x00663324, Child(window, 0x1f8));   // Window7059f0_RefreshRows
            u8 *const money = (u8 *)(uintptr_t)Child(window, 0x200);
            const s32 balance = Balance();
            u32 number[5] = {(u32)(uintptr_t)TextArgInt_vtable, 0x5641524d, 0xd, balance < 0 ? 0u - (u32)balance : (u32)balance, 0};
            g_handler.Cd(0x006acd60, global<u32>(0x0078ba94), 0x007ba058u, 0x3fffu, balance < 0 ? 0x1cbu : 0x1cau, (u32)(uintptr_t)number);   // Charset_FormatWide
            vcall<void>(money, 0x24, 0x007ba058u);
        }
        break;
    case 0x14:
        global<u8 *>(0x007cc1f8) = nullptr;
        g_handler.Cd(0x00486510);           // Sign_Destroy
        g_handler.Cd(0x00486b40, 0xbcu);    // Game_CancelAdvisor
        break;
    case 0x15:
        g_handler.Cd(0x004ab6b0);                                // Dialog7cc1f8_DressStats
        g_handler.Cd(0x004abfb0, global<u32>(0x007cc1fc));       // Dialog7cc1f8_ShowStats
        if ((u16)g_handler.Th(0x0065ef90, (u32)(uintptr_t)window, 0x80080u, 0x3e8u)) g_handler.Th(0x0065f024, (u32)(uintptr_t)window, 0x80080u);   // Window_SetTimer, _StartTimer
        break;
    case 0x1e:
        if (reinterpret_cast<u16 &>(gDialog7cc1f8_Open_7cc1e4) && (s32)(g_handler.Cd(0x0065968e) - gDialog7cc1f8_Open_7cc1f0) > 0x1f4) {
            g_handler.Cd(0x004ab1b0, (u32)(uintptr_t)&gDialog7cc1f8_Open_7cc1e4);   // Dialog7cc1f8_ShowPreview
            reinterpret_cast<u16 &>(gDialog7cc1f8_Open_7cc1e4) = 0;
        }
        g_handler.Cd(0x004864f0);   // Sign_Swing
        if (gDialog7cc1f8_Open_7cc1e8) g_handler.Cd(0x004abc80, (u32)(uintptr_t)&gDialog7cc1f8_Open_7cc1e8);   // Dialog7cc1f8_ShowItemTip
        break;
    case 0x100:
        if (a == 0xfffffffe) {
            CloseDialog();
            return 0;
        }
        if (a == 0x1ff) {
            g_handler.Cd(0x0049bdd0, 0xffffffffu);   // Dialog7ca308_Open
            return 0;
        }
        break;
    case 0x400: {
        const u32 list = Child(window, a);
        if (!list || b == 0xffffffff) break;
        const s32 balance = Balance();
        s32 item;
        g_handler.Th(0x00664c71, list, b, (u32)(uintptr_t)&item);   // Window7059f0_NthItemData
        if (item > 0) {
            const u8 *const controls = (const u8 *)(uintptr_t)g_handler.Th(0x004d3d10, (u32)(uintptr_t)gWorldPtr + 0x1d943c, (u32)item, 0u);   // ObjectControls_Find
            const u32 id = (u16)item;   // (the original pushes the dword whose low word it set: stack garbage above)
            if (at<u32>((u8 *)(uintptr_t)g_handler.Th(0x00412e90, 0x007890a0u, id), 0xc4) && !(u8)g_handler.St(0x004d4b70, id)) {   // ObjectDefs_Find, GlobalSave_HasKey
                if (!(u8)g_handler.St(0x004d4b90, id)) {   // ObjectDef_IsUnlocked
                    g_handler.Cd(0x00486ae0);   // SoundUi_Play1d
                    break;
                }
            } else if (balance < *reinterpret_cast<const s32 *>(controls + 4)) {
                g_handler.Cd(0x00486ae0);
                break;
            }
            SetTileMode(4, id, 0);
            CloseDialog();
            const bool first = !at<u32>((u8 *)(uintptr_t)g_handler.Th(0x00412e90, 0x007890a0u, id), 0x4c) && !g_handler.Cd(0x004824d0, 0x80u, 0u);   // Things_FindNextOfCategory
            g_handler.Cd(0x00486b00, first ? 0xc9u : 0xceu);   // Advisor_PostId
            return 0;
        }
        if (item == -1) SetTileMode(0, 0x39, 0x15a);
        else if (item == -2) SetTileMode(0, 0x3a, 0x159);
        CloseDialog();
        return 0;
    }
    case 0x401: {
        const u32 list = Child(window, a);
        if (!list || b == 0xffffffff) break;
        u32 item;
        g_handler.Th(0x00664c71, list, b, (u32)(uintptr_t)&item);
        if ((u16)item != reinterpret_cast<u16 &>(gDialog7cc1f8_Open_7cc1e4)) {
            reinterpret_cast<u16 &>(gDialog7cc1f8_Open_7cc1e4) = (u16)item;
            gDialog7cc1f8_Open_7cc1f0 = g_handler.Cd(0x0065968e);   // GuiClock_Now
        }
        gDialog7cc1f8_Open_7cc1e8 = (u16)item;
        u32 shown = (u16)item;   // (the original's word in the argument a's slot)
        g_handler.Cd(0x004abb00, (u32)(uintptr_t)&shown);   // Dialog7cc1f8_ShowFigures
        break;
    }
    case 0x406:
        gDialog7cc1f8_Open_755140 = (s32)b;
        break;
    default:
        break;
    }
    return (s32)g_handler.Cd(0x00488ba0, (u32)(uintptr_t)window, message, a, b);   // Dialog_KeyHandler
}

extern "C" {

void __cdecl Dialog7cc1f8_FillList(u32 kind, u32 clear) {
    u8 *const game = *&gWorldPtr;
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
        u8 *const control = ObjectControls_Find(*&gWorldPtr + 0x1d943c, nullptr, id, 0);
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
        row.value = global<u32>(0x0078560c);
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

s32 __cdecl Dialog7cc1f8_RowHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x403) {
        if ((s16)b < 0) {
            Window_Notify40000(window, nullptr, 0xe7, 0xef, 0x4b, 0xff);
        } else {
            u8 *const game = *&gWorldPtr;
            u8 *const control = ObjectControls_Find(game + 0x1d943c, nullptr, b, 0);
            const s32 balance = (s32)Source_Field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(game)));
            if (at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, (u16)b), 0xc4) && !(u8)GlobalSave_HasKey(b))
                Window_Notify40000(window, nullptr, 0xff, ObjectDef_IsUnlocked(b) ? 0xe0 : 0, 0, 0xff);
            else if (at<s32>(control, 4) > balance)
                Window_Notify40000(window, nullptr, 0xff, 0, 0, 0xff);
            else
                Window_Notify40000(window, nullptr, 0xff, 0xff, 0xff, 0xff);
        }
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}

void __cdecl Dialog7cc1f8_ShowItemTip(const s16 *item) {
    u8 *const list = Window_FindChild(*reinterpret_cast<u8 **>((uintptr_t)0x007cc1f8), nullptr, 0x1f8);
    u8 rect[0x10];
    GuiRect_Ctor(rect);
    Window_GetSavedRect(list, nullptr, rect);
    u16 x, y;
    Gui_GetMousePos(&x, &y);
    s32 &state = gDialog7cc1f8_ShowItemTip_75513c;
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
        u8 *const game = *&gWorldPtr;
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
                TextFormat().format(*reinterpret_cast<void * *>(&gDialog7cc1f8_78bad0), name, 0x3f,
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
