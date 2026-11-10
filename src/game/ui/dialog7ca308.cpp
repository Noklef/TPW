#include "dialog7ca308.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../files/wrapsources.h"
#include "../math/x87.h"
#include "../render/textparts.h"
#include "../sprite/screenmatrix.h"
#include "../sprite/script.h"
#include "../world/advisor.h"
#include "../world/staff.h"
#include "../world/staffcandidate.h"
#include "../world/world.h"
#include "buttontext.h"
#include "dialog7ca098.h"
#include "dialog7cc1f8.h"
#include "dialogbase.h"
#include "focus.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "listrows.h"
#include "panelparts.h"
#include "radiogroup.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"
#include "windowtimers.h"
#include "panel7b9940.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7ca308Env kGameEnv = {&Window7059f0_Clear, &Window7059f0_AddItem, &Window7059f0_RefreshRows, &StaffPoolEntry_FormatName, &Staff_WageForRecord, &Window7059f0_FindItemData, &Window7059f0_RemoveItem,
                                  &Dialog7ca308_FillList, &Charset_FormatWide, &RadioGroup_Select, &HoverLabel_Attach, &SpriteScript_Kill, &Language_WideText};
const Dialog7ca308Env *g_env = &kGameEnv;

// The kind's radio button selected (kinds past 4: none).
void selectKind(u8 *group) {
    static const u32 kButton[5] = {0x2490, 0x2493, 0x2492, 0x2494, 0x2491};
    const u32 kind = gDialog7ca308_7ca30c;
    if (kind <= 4) g_env->selectRadio(group, nullptr, kButton[kind]);
}

// The label 0x2480's text set by the kind (kinds past 4: none).
void labelKind() {
    const u32 kind = gDialog7ca308_7ca30c;
    if (kind <= 4) g_env->attachLabel(gDialog7ca308_7ca308, 0x2480, (u32)(uintptr_t)Language_WideText(0x8b + kind));
}

using panelparts::numberPart;
using panelparts::red;
using panelparts::redText;

// The newest value of a ring (the values, +0x240 the position, +0x244 the count, +0x248 wrapped) into the dword at `address`: an empty
// ring, or one not wrapped with the position negative, leaves it.
void newest(const u8 *ring, u32 address) {
    const s32 count = at<s32>(ring, 0x244);
    if (count <= 0) return;
    const s32 position = at<s32>(ring, 0x240);
    if (position >= 0) global<u32>(address) = at<u32>(ring, (u32)position * 4);
    else if (at<u8>(ring, 0x248)) global<u32>(address) = at<u32>(ring, (u32)(position + count) * 4);
}
}  // namespace

void Dialog7ca308_SetEnv(const Dialog7ca308Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7ca308_ListHandler(u8 *window, u32 message, u32 id, u32 value) {
    static const u8 kKinds[5] = {0, 4, 2, 1, 3};
    if (message == 0x101 && value == 1 && id - 0x2490 <= 4) {
        const u32 kind = kKinds[id - 0x2490];
        g_env->fillList(kind, 1);
        const u32 text = (u32)(uintptr_t)g_env->wideText(0x8b + kind);
        g_env->attachLabel(gDialog7ca308_7ca308, 0x2480, text);
        if (const s32 script = gDialog7ca308_7ca310) g_env->killScript(script);
        reinterpret_cast<u32 &>(gDialog7ca308_7ca310) = 0;
        gDialog7ca308_7523e8 = 0xffffffff;
        gDialog7ca308_7523ec = 0xffffffff;
    }
    return Window_CallDefault(window, nullptr, message, id, value);
}

void __cdecl Dialog7ca308_FillList(u32 kind, u32 clear) {
    u8 *const window = gDialog7ca308_7ca308;
    if (!window) return;
    u8 *const list = Window_FindChild(window, nullptr, 0x248e);
    if (!list) return;
    gDialog7ca308_7ca30c = kind;
    if (clear) g_env->clear(list);
    struct Row {   // (the original's frame: the texts just before the name)
        u32 texts[2];
        u16 name[0x28];
    } row;
    for (u32 i = 0; i < 0x20; ++i) {
        const u8 *const entry = StaffPool_ValidEntry(global<u8 *>(0x007cf83c), nullptr, i);
        if (!entry || at<u32>(entry, 0) != kind) continue;
        g_env->formatName(entry, nullptr, row.name, 0x27);
        row.texts[0] = (u32)(uintptr_t)row.name;
        row.texts[1] = (u32)g_env->wage(entry);
        g_env->addItem(list, nullptr, row.texts, i, -1, 0);
    }
    g_env->refresh(list);
}

void __cdecl Dialog7ca308_StaffChanged(u32 index) {
    u8 *const window = gDialog7ca308_7ca308;
    if (!window) return;
    u8 *const list = Window_FindChild(window, nullptr, 0x248e);
    const s32 position = g_env->findItem(list, nullptr, index);
    const u8 *const entry = StaffPool_ValidEntry(global<u8 *>(0x007cf83c), nullptr, index);
    struct Row {   // (the original's frame: the texts just before the name)
        u32 texts[2];
        u16 name[0x2c];
    } row;
    const auto add = [&]() {
        const u8 *const again = StaffPool_ValidEntry(global<u8 *>(0x007cf83c), nullptr, index);
        g_env->formatName(again, nullptr, row.name, 0x28);
        row.texts[0] = (u32)(uintptr_t)row.name;
        row.texts[1] = (u32)g_env->wage(again);
        g_env->addItem(list, nullptr, row.texts, index, -1, 0);
    };
    if (position > -1) {
        g_env->removeItem(list, nullptr, position);
        if (entry) add();
    } else if (entry && gDialog7ca308_7ca30c == at<u32>(entry, 0)) {
        add();
        g_env->refresh(list);
    }
}

void __cdecl Dialog7ca308_ShowStaff(u32 index) {
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    u8 *const child = Window_FindChild(gDialog7ca308_7ca308, nullptr, 0x2484);
    Window_GetSavedRect(child, nullptr, rect);
    const s32 left = at<s16>(rect, 4), top = at<s16>(rect, 6), right = at<s16>(rect, 8), bottom = at<s16>(rect, 0xa);
    const float x = (float)X87_MulIntDouble(((right - left) >> 1) + left - 0x3f8, global<float>(0x006ff984));
    const float y = (float)X87_MulIntDouble(((bottom - top) >> 2) - bottom + 0x300, global<float>(0x006ff988));
    const float z = (float)Sprite_FlipY(at<s16>(child, 0xe0) + 1);
    const u8 *const entry = StaffPool_ValidEntry(global<u8 *>(0x007cf83c), nullptr, index);
    if (entry) {
        const u32 type = at<u32>(entry, 0), sprite = entry[9];
        if (type != gDialog7ca308_7523e8 || sprite != gDialog7ca308_7523ec) {
            gDialog7ca308_7523e8 = type;
            gDialog7ca308_7523ec = sprite;
            if (gDialog7ca308_7ca310) SpriteScript_Kill(gDialog7ca308_7ca310);
            static const u32 kFolder[5] = {5, 6, 4, 7, 8};   // (by the type)
            const u32 folder = type <= 4 ? kFolder[type] : index;   // sic: any other type, the index
            u32 bits[3];
            std::memcpy(&bits[0], &x, 4);
            std::memcpy(&bits[1], &y, 4);
            std::memcpy(&bits[2], &z, 4);
            gDialog7ca308_7ca310 = SpriteScript_Start(SpriteCode(0x1ca0), folder, sprite, 1, bits[0], bits[1], bits[2]);
            SpriteScript_SetRegF(gDialog7ca308_7ca310, 8, 0x41000000);
        }
    }
    GuiRect_Dtor(rect);
}

void __cdecl Dialog7ca308_Open(u32 kind) {
    Window7c24c8_Close();
    Global7cb28c_Select(1, 2);
    if (gDialog7ca308_7ca308 && kind != 0xffffffff && kind != gDialog7ca308_7ca30c) {
        gDialog7ca308_7ca30c = kind;
        selectKind(Window_FindChild(Window_FindChild(gDialog7ca308_7ca308, nullptr, 0x248e), nullptr, 0x248f));
        labelKind();
        g_env->fillList(gDialog7ca308_7ca30c, 1);
        return;
    }
    u8 *const dialog = WindowScript_Build(gPanel7b9940_7cb2ac, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00751fa8), 0x0049b650);
    if (!dialog) return;
    if (kind == 0xffffffff) kind = gDialog7ca308_7ca30c;
    gDialog7ca308_7ca30c = kind;
    gDialog7ca308_7ca308 = dialog;
    Window7c24c8_Set(dialog);
    // the money line: a number argument 13, its size, in the text 0x1cb when negative (0x1ca else)
    u8 *child = Window_FindChild(gDialog7ca308_7ca308, nullptr, 0x2496);
    const s32 money = (s32)Source_Field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x007cf83c))));
    u32 number[5] = {0x006ff170, 0x5641524d, 0xd, money < 0 ? 0u - (u32)money : (u32)money, 0};
    g_env->format(global<u8 *>((u32)(uintptr_t)&gLanguage_78b530 + 0x564), reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff, money < 0 ? 0x1cbu : 0x1cau, number);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetObject98(child, nullptr, memory ? TextPart6ff530_Ctor(memory, nullptr, 0) : nullptr);
    Window_NotifyC00(child, nullptr, 2, 1);
    Window_Set4c(child, nullptr, Language_Font(5));
    Window_Notify40000(child, nullptr, 0xff, 0xff, 0xff, 0xff);
    vcall<void>(child, 0x24, 0x007ba058u);
    // the list
    if (u8 *const list = Window_FindChild(gDialog7ca308_7ca308, nullptr, 0x248e)) {
        Window7059f0_BuildRows(list, nullptr, &List7ca314_NewRowJump, &List7ca314_InitRow);
        Window7059f0_SetColumnBottom(list, nullptr, 0, 0);
        Window7059f0_SetColumnBottom(list, nullptr, 1, 1);
        Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))));
        Window_SetHandler(list, nullptr, 0x0049bc80);
        Window7059f0_SortBy(list, nullptr, gDialog7ca308_7523e4, 0);
        child = Window_FindChild(list, nullptr, 0x10);
        ButtonText_Init(child, (u32)(uintptr_t)Language_WideText(0x90), gDialog7ca308_Open_7523e0, 0xff, 0xef, 0, 0x9f);
        child = Window_FindChild(list, nullptr, 0x11);
        ButtonText_Init(child, (u32)(uintptr_t)Language_WideText(0x91), gDialog7ca308_Open_7523e0, 0xff, 0xef, 0, 0xa0);
        labelKind();
        g_env->fillList(gDialog7ca308_7ca30c, 0);
        if (u8 *const group = Window_FindChild(list, nullptr, 0x248f)) selectKind(group);
    }
    // the bar
    if (u8 *const bar = Window_FindChild(gDialog7ca308_7ca308, nullptr, 0x2481)) {
        panelparts::text(bar, 0x2483, 0x92);
        vcall<void>(panelparts::barOf(Window_FindChild(bar, nullptr, 0x2482), panelparts::kRideStatBar), 0x1c, 0u);
    }
    // the figures
    if (u8 *const figures = Window_FindChild(gDialog7ca308_7ca308, nullptr, 0x2485)) {
        const u8 *const rings = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
        panelparts::text(figures, 0x2488, 0x165);
        redText(figures, 0x248d, 0x166);
        redText(figures, 0x248a, 0x167);
        panelparts::text(figures, 0x2486, 0x169);
        global<u32>(0x007ca2f0) = 0;
        global<u32>(0x007ca2fc) = 0;
        global<u32>(0x007ca2f8) = 0;
        newest(rings + 0x1fc94, 0x007ca2f0);
        vcall<void>(numberPart(figures, 0x2489), 0x1c, global<u32>(0x007ca2f0));
        newest(rings + 0x1f7f4, 0x007ca2fc);
        child = numberPart(figures, 0x248c);
        vcall<void>(child, 0x1c, global<u32>(0x007ca2fc));
        red(child);
        newest(rings + 0x1f5a4, 0x007ca2f8);
        global<u32>(0x007ca300) = global<u32>(0x007ca2f8) - global<u32>(0x007ca2fc);
        child = numberPart(figures, 0x248b);
        vcall<void>(child, 0x1c, global<u32>(0x007ca300));
        red(child);
        vcall<void>(numberPart(figures, 0x2487), 0x1c, global<u32>(0x007ca2f0) - global<u32>(0x007ca2f8));
    }
    Advisor_PostId(0xbd);
}
}  // extern "C"

namespace {
const Dialog7ca308HandlerEnv kGameHandlerEnv = {Window_SendMessage, Window_FindChild, GameTable7cfb90_Get1da726_5195d0, reinterpret_cast<u32(__fastcall *)(u32)>(Source_Field0c), Charset_FormatWide, SpriteScript_Kill, Game_CancelAdvisor, Window_SetTimer, Window_StartTimer, Dialog7cc1f8_Open,
                                                Window7059f0_NthItemData, StaffPool_ValidEntry, Staff_WageForRecord, Dialog7ca308_ShowStaff, reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32)>(&StaffPool_TakeCandidate), Advisor_PostId, Dialog_KeyHandler};
const Dialog7ca308HandlerEnv *g_handler = &kGameHandlerEnv;

u32 &word(u32 at) { return *reinterpret_cast<u32 *>(static_cast<uintptr_t>(at)); }
u8 *addr(u32 at) { return reinterpret_cast<u8 *>(static_cast<uintptr_t>(word(at))); }

void showMoney(u8 *window) {
    u8 *const label = g_handler->child(window, nullptr, 0x2496);
    const s32 money = static_cast<s32>(g_handler->money(g_handler->table26(addr(0x007cf83c))));
    u32 arg[5] = {0x006ff170, 0x5641524d, 0xd, static_cast<u32>(money < 0 ? -money : money), 0};   // (TextArgInt_Ctor (0xd, the value, unsigned), inline)
    g_handler->format(addr((u32)(uintptr_t)&gLanguage_78b530 + 0x564), reinterpret_cast<u16 *>(static_cast<uintptr_t>(0x007ba058)), 0x3fff, money < 0 ? 0x1cb : 0x1ca, arg);
    vcall<void>(label, 0x24, 0x007ba058u);
}

void onRowHighlighted(u8 *window, u32 message, u32 a, u32 b) {
    u8 *const list = g_handler->child(window, nullptr, a);
    if (!list || b == 0xffffffffu) return;
    u32 data = message;   // (the original's out is its message argument's slot)
    g_handler->nth(list, nullptr, static_cast<s32>(b), &data);
    u8 *entry = g_handler->entry(addr(0x007cf83c), nullptr, data);
    u8 *const bar = g_handler->child(addr(reinterpret_cast<uintptr_t>(&gDialog7ca308_7ca308)), nullptr, 0x2481);
    if (bar && entry) {
        const u32 level = entry[8];
        vcall<void>(g_handler->child(bar, nullptr, 0x2482), 0x1c, static_cast<s32>(level << 10) / 5);
    }
    entry = g_handler->entry(addr(0x007cf83c), nullptr, data);
    u8 *const wages = g_handler->child(addr(reinterpret_cast<uintptr_t>(&gDialog7ca308_7ca308)), nullptr, 0x2485);
    if (wages && entry) {
        const u32 wage = static_cast<u32>(g_handler->wage(entry));
        vcall<void>(g_handler->child(wages, nullptr, 0x248c), 0x1c, word(0x007ca2fc) + wage);
        vcall<void>(g_handler->child(wages, nullptr, 0x2487), 0x1c, word(0x007ca2f0) - wage - word(0x007ca2fc) - word(0x007ca300));
    }
    g_handler->show(data);
}
}  // namespace

void Dialog7ca308_SetHandlerEnv(const Dialog7ca308HandlerEnv *env) { g_handler = env ? env : &kGameHandlerEnv; }

extern "C" s32 __cdecl Dialog7ca308_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 5:
        g_handler->send(addr(reinterpret_cast<uintptr_t>(&gDialog7ca308_7ca308)), 4, 0, 0);
        return 1;
    case 0x10:
        if (a == 0x80080) showMoney(window);
        break;
    case 0x14: {
        const u32 script = reinterpret_cast<u32 &>(gDialog7ca308_7ca310);
        reinterpret_cast<u32 &>(gDialog7ca308_7ca308) = 0;
        if (script) g_handler->kill(static_cast<s32>(script));
        reinterpret_cast<u32 &>(gDialog7ca308_7ca310) = 0;
        gDialog7ca308_7523e8 = 0xffffffffu;
        gDialog7ca308_7523ec = 0xffffffffu;
        g_handler->cancel(0xbd);
        break;
    }
    case 0x15:
        if (static_cast<u16>(g_handler->setTimer(window, nullptr, 0x80080, 0x3e8))) g_handler->startTimer(window, nullptr, 0x80080);
        break;
    case 0x100:
        if (a == 0xfffffffeu) {
            g_handler->send(addr(reinterpret_cast<uintptr_t>(&gDialog7ca308_7ca308)), 4, 0, 0);
            return 0;
        }
        if (a == 0x2495) {
            g_handler->openList(0xffffffffu, 0);
            return 0;
        }
        break;
    case 0x400: {
        u8 *const list = g_handler->child(window, nullptr, a);
        if (!list || b == 0xffffffffu) break;
        u32 data = b;   // (the original's out is its b argument's slot)
        g_handler->nth(list, nullptr, static_cast<s32>(b), &data);
        g_handler->hire(addr(0x007cf83c), nullptr, data);
        g_handler->send(addr(reinterpret_cast<uintptr_t>(&gDialog7ca308_7ca308)), 4, 0, 0);
        g_handler->post(0x135);
        return 0;
    }
    case 0x401:
        onRowHighlighted(window, message, a, b);
        break;
    case 0x406:
        reinterpret_cast<u32 &>(gDialog7ca308_7523e4) = b;
        break;
    }
    return g_handler->keys(window, message, a, b);
}
