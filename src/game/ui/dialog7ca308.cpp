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
#include "../world/world.h"
#include "buttontext.h"
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

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7ca308Env kGameEnv = {&Window7059f0_Clear, &Window7059f0_AddItem, &Window7059f0_RefreshRows, &StaffPoolEntry_FormatName, &Staff_WageForRecord, &Window7059f0_FindItemData, &Window7059f0_RemoveItem,
                                  &Dialog7ca308_FillList, &Charset_FormatWide, &RadioGroup_Select, &HoverLabel_Attach};
const Dialog7ca308Env *g_env = &kGameEnv;

// The kind's radio button selected (kinds past 4: none).
void selectKind(u8 *group) {
    static const u32 kButton[5] = {0x2490, 0x2493, 0x2492, 0x2494, 0x2491};
    const u32 kind = global<u32>(0x007ca30c);
    if (kind <= 4) g_env->selectRadio(group, nullptr, kButton[kind]);
}

// The label 0x2480's text set by the kind (kinds past 4: none).
void labelKind() {
    const u32 kind = global<u32>(0x007ca30c);
    if (kind <= 4) g_env->attachLabel(global<u8 *>(0x007ca308), 0x2480, (u32)(uintptr_t)Language_WideText(0x8b + kind));
}

// A new part (none when the allocation fails) as the child's implementation.
u8 *withPart(u8 *window, panelparts::PartCtor ctor, u32 id) {
    u8 *const child = Window_FindChild(window, nullptr, id);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(child, nullptr, memory ? ctor(memory, nullptr, 0) : nullptr);
    return child;
}

void font6(u8 *child) { Window_Set4c(child, nullptr, Language_Font(6)); }

void red(u8 *child) { Window_Notify40000(child, nullptr, 0xff, 0, 0, 0xff); }

// A heading in red: its colour comes first, unlike panelparts::text's.
void redText(u8 *window, u32 id, u32 text) {
    u8 *const child = withPart(window, TextPart_Ctor, id);
    red(child);
    font6(child);
    Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_WideText(text));
}

// A number's part, without its value.
u8 *numberPart(u8 *window, u32 id) {
    u8 *const child = withPart(window, TextPart6ff3b0_Ctor, id);
    Window_NotifyC00(child, nullptr, 2, 1);
    font6(child);
    return child;
}

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

void __cdecl Dialog7ca308_FillList(u32 kind, u32 clear) {
    u8 *const window = global<u8 *>(0x007ca308);
    if (!window) return;
    u8 *const list = Window_FindChild(window, nullptr, 0x248e);
    if (!list) return;
    global<u32>(0x007ca30c) = kind;
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
    u8 *const window = global<u8 *>(0x007ca308);
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
    } else if (entry && global<u32>(0x007ca30c) == at<u32>(entry, 0)) {
        add();
        g_env->refresh(list);
    }
}

void __cdecl Dialog7ca308_ShowStaff(u32 index) {
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    u8 *const child = Window_FindChild(global<u8 *>(0x007ca308), nullptr, 0x2484);
    Window_GetSavedRect(child, nullptr, rect);
    const s32 left = at<s16>(rect, 4), top = at<s16>(rect, 6), right = at<s16>(rect, 8), bottom = at<s16>(rect, 0xa);
    const float x = (float)X87_MulIntDouble(((right - left) >> 1) + left - 0x3f8, global<float>(0x006ff984));
    const float y = (float)X87_MulIntDouble(((bottom - top) >> 2) - bottom + 0x300, global<float>(0x006ff988));
    const float z = (float)Sprite_FlipY(at<s16>(child, 0xe0) + 1);
    const u8 *const entry = StaffPool_ValidEntry(global<u8 *>(0x007cf83c), nullptr, index);
    if (entry) {
        const u32 type = at<u32>(entry, 0), sprite = entry[9];
        if (type != global<u32>(0x007523e8) || sprite != global<u32>(0x007523ec)) {
            global<u32>(0x007523e8) = type;
            global<u32>(0x007523ec) = sprite;
            if (global<s32>(0x007ca310)) SpriteScript_Kill(global<s32>(0x007ca310));
            static const u32 kFolder[5] = {5, 6, 4, 7, 8};   // (by the type)
            const u32 folder = type <= 4 ? kFolder[type] : index;   // sic: any other type, the index
            u32 bits[3];
            std::memcpy(&bits[0], &x, 4);
            std::memcpy(&bits[1], &y, 4);
            std::memcpy(&bits[2], &z, 4);
            global<s32>(0x007ca310) = SpriteScript_Start(0x0074f758, folder, sprite, 1, bits[0], bits[1], bits[2]);
            SpriteScript_SetRegF(global<s32>(0x007ca310), 8, 0x41000000);
        }
    }
    GuiRect_Dtor(rect);
}

void __cdecl Dialog7ca308_Open(u32 kind) {
    Window7c24c8_Close();
    Global7cb28c_Select(1, 2);
    if (global<u8 *>(0x007ca308) && kind != 0xffffffff && kind != global<u32>(0x007ca30c)) {
        global<u32>(0x007ca30c) = kind;
        selectKind(Window_FindChild(Window_FindChild(global<u8 *>(0x007ca308), nullptr, 0x248e), nullptr, 0x248f));
        labelKind();
        g_env->fillList(global<u32>(0x007ca30c), 1);
        return;
    }
    u8 *const dialog = WindowScript_Build(global<u8 *>(0x007cb2ac), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00751fa8), 0x0049b650);
    if (!dialog) return;
    if (kind == 0xffffffff) kind = global<u32>(0x007ca30c);
    global<u32>(0x007ca30c) = kind;
    global<u8 *>(0x007ca308) = dialog;
    Window7c24c8_Set(dialog);
    // the money line: a number argument 13, its size, in the text 0x1cb when negative (0x1ca else)
    u8 *child = Window_FindChild(global<u8 *>(0x007ca308), nullptr, 0x2496);
    const s32 money = (s32)Source_Field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x007cf83c))));
    u32 number[5] = {0x006ff170, 0x5641524d, 0xd, money < 0 ? 0u - (u32)money : (u32)money, 0};
    g_env->format(global<u8 *>(0x0078ba94), reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff, money < 0 ? 0x1cbu : 0x1cau, number);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetObject98(child, nullptr, memory ? TextPart6ff530_Ctor(memory, nullptr, 0) : nullptr);
    Window_NotifyC00(child, nullptr, 2, 1);
    Window_Set4c(child, nullptr, Language_Font(5));
    Window_Notify40000(child, nullptr, 0xff, 0xff, 0xff, 0xff);
    vcall<void>(child, 0x24, 0x007ba058u);
    // the list
    if (u8 *const list = Window_FindChild(global<u8 *>(0x007ca308), nullptr, 0x248e)) {
        Window7059f0_BuildRows(list, nullptr, &List7ca314_NewRowJump, &List7ca314_InitRow);
        Window7059f0_SetColumnBottom(list, nullptr, 0, 0);
        Window7059f0_SetColumnBottom(list, nullptr, 1, 1);
        Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))));
        Window_SetHandler(list, nullptr, 0x0049bc80);
        Window7059f0_SortBy(list, nullptr, global<s32>(0x007523e4), 0);
        child = Window_FindChild(list, nullptr, 0x10);
        ButtonText_Init(child, (u32)(uintptr_t)Language_WideText(0x90), global<u32>(0x007523e0), 0xff, 0xef, 0, 0x9f);
        child = Window_FindChild(list, nullptr, 0x11);
        ButtonText_Init(child, (u32)(uintptr_t)Language_WideText(0x91), global<u32>(0x007523e0), 0xff, 0xef, 0, 0xa0);
        labelKind();
        g_env->fillList(global<u32>(0x007ca30c), 0);
        if (u8 *const group = Window_FindChild(list, nullptr, 0x248f)) selectKind(group);
    }
    // the bar
    if (u8 *const bar = Window_FindChild(global<u8 *>(0x007ca308), nullptr, 0x2481)) {
        panelparts::text(bar, 0x2483, 0x92);
        vcall<void>(panelparts::barOf(Window_FindChild(bar, nullptr, 0x2482), panelparts::kRideStatBar), 0x1c, 0u);
    }
    // the figures
    if (u8 *const figures = Window_FindChild(global<u8 *>(0x007ca308), nullptr, 0x2485)) {
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
