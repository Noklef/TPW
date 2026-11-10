#include "dialog7cb25c.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/language.h"
#include "../core/methods.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../files/wrapsources.h"
#include "../world/advisor.h"
#include "../world/money.h"
#include "../world/world.h"
#include "buttontext.h"
#include "dialog7ca098.h"
#include "dialog7ca210.h"
#include "dialog7ca2e8.h"
#include "dialog7cc2ec.h"
#include "dialogbase.h"
#include "dialogs.h"
#include "focus.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "radiogroup.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"
#include "dialog74fb20.h"
#include "panel7b9940.h"
#include "../core/textarg.h"

namespace {

u8 *dialog() { return gDialog7cb25c_7cb25c; }

const Dialog7cb25cOpenEnv kOpenGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &HoverLabel_Attach, &Money_LoanOffer,
                                          &Window7059f0_BuildRows, &Dialog7cb25c_FillList, &RadioGroup_Select, &Dialog7cb25c_SetTexts};
const Dialog7cb25cOpenEnv *g_open = &kOpenGameEnv;

const Dialog7cb25cHandlerEnv kHandlerGameEnv = {&Window_SendMessage, &Game_CancelAdvisor, &Window_FindChild, &Window7059f0_NthItemData, &GameTable7cfb90_Get1da726_5195d0, &Money_LoanOffer,
                                                reinterpret_cast<u32(__fastcall *)(u32)>(&Source_Field0c), &Charset_FormatWide,
                                                reinterpret_cast<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)0x0047f020), &Dialog7ca210_Open, &Dialog7ca2e8_Open, &Dialog7cc2ec_Open, &Dialog_KeyHandler};
const Dialog7cb25cHandlerEnv *g_handler = &kHandlerGameEnv;

u8 *game() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cf83c); }

// The text of the number formatted into text and a box of it (the white record, its last colour byte never written, sic).
void loanBox(u16 *text, u32 entry, u32 value, u32 buttons, u32 index, u32 callback) {
    const u32 arg[5] = {(u32)(uintptr_t)TextArgInt_vtable, 0x5641524d, 1, value, 0};   // (TextArgInt_Ctor (1, the value, unsigned), inline)
    g_handler->format(gLanguage_78ba94, text, 0xff, entry, arg);
    struct {
        u16 *text;
        u8 colour[4];
        u32 kind, data;
    } record;
    record.text = text;
    record.colour[0] = record.colour[1] = record.colour[2] = 0xff;
    record.kind = 4;
    record.data = index;
    g_handler->box(reinterpret_cast<const u8 *>(&record), buttons, 0, callback ? 1 : 0, callback, 0, 0, 0);
}

void chosen(s32 data) {
    u32 offer[8];
    u16 text[0x100];
    if (data < 0) {
        const u32 index = (u32)(-1 - data);
        g_handler->offer(reinterpret_cast<u8 *>((uintptr_t)g_handler->table26(game())), nullptr, offer, index);
        const u32 owed = (offer[3] - offer[6]) * offer[4];
        if ((s32)g_handler->field(g_handler->table26(game())) > (s32)owed) loanBox(text, 0xb5, owed, 2, index, 0x0049f460);
        else loanBox(text, 0xb6, owed, 1, index, 0);
    } else {
        const u32 index = (u32)data - 1;
        g_handler->offer(reinterpret_cast<u8 *>((uintptr_t)g_handler->table26(game())), nullptr, offer, index);
        loanBox(text, 0xb7, offer[1], 2, index, 0x0049f4a0);
    }
}

}  // namespace

void Dialog7cb25c_SetOpenEnv(const Dialog7cb25cOpenEnv *env) { g_open = env ? env : &kOpenGameEnv; }
void Dialog7cb25c_SetHandlerEnv(const Dialog7cb25cHandlerEnv *env) { g_handler = env ? env : &kHandlerGameEnv; }

extern "C" {

s32 __cdecl Dialog7cb25c_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 5:
        g_handler->send(dialog(), 4, 0, 0);
        return 1;
    case 0x14:
        gDialog7cb25c_7cb25c = nullptr;
        g_handler->cancel(0xc3);
        break;
    case 0x100:
        switch (a) {
        case 0xfffffffe:
        case 0xffffffff:
            g_handler->send(dialog(), 4, 0, 0);
            return 0;
        case 0x12c4ba:
            g_handler->open1();
            return 0;
        case 0x12c4bb:
            g_handler->open2();
            return 0;
        case 0x12c4bc:
            g_handler->open3();
            return 0;
        default:
            break;
        }
        break;
    case 0x400: {
        u8 *const list = g_handler->find(window, nullptr, a);
        if (list && b != 0xffffffff) {
            s32 data;
            g_handler->nth(list, nullptr, (s32)b, reinterpret_cast<u32 *>(&data));
            chosen(data);
        }
        break;
    }
    default:
        break;
    }
    return g_handler->keys(window, message, a, b);
}

void __cdecl Dialog7cb25c_Open() {
    if (dialog()) return;
    u8 &guard = gDialog7cb25c_fb34c2;
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_open->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (gDialog74fb20_fb3b7c == 2) return;
    Window7c24c8_Close();
    Global7cb28c_Select(3, 8);
    u8 *const built = WindowScript_Build(gPanel7b9940_7cb2ac, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x007525a0), 0x0049f4e0);
    if (!built) return;
    gDialog7cb25c_7cb25c = built;
    Window7c24c8_Set(built);
    g_open->attach(dialog(), 0x12c4ca, (u32)(uintptr_t)Language_WideText(0xaa));
    if (u8 *const figures = Window_FindChild(dialog(), nullptr, 0x12c4c1)) {
        u8 *const game = *reinterpret_cast<u8 **>((uintptr_t)0x007cf83c);
        const u8 *const rings = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(game));
        panelparts::text(figures, 0x12c4c4, 0x165);
        panelparts::redText(figures, 0x12c4c9, 0x168);
        panelparts::redText(figures, 0x12c4c6, 0x167);
        panelparts::text(figures, 0x12c4c2, 0x169);
        const u32 first = panelparts::newestOfRing(rings + 0x1fc94);
        vcall<void>(panelparts::numberPart(figures, 0x12c4c5), 0x1c, first);
        u8 *child = Window_FindChild(figures, nullptr, 0x12c4c8);
        u32 sum = 0;
        for (u32 i = 0; i < 8; ++i) {
            u32 out[8];
            u8 *const money = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(*reinterpret_cast<u8 **>((uintptr_t)0x007cf83c)));
            const u32 *const offer = g_open->loanOffer(money, nullptr, out, i);
            if (offer[5]) sum += offer[4];
        }
        u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        Window_SetImpl(child, nullptr, memory ? TextPart6ff3b0_Ctor(memory, nullptr, 0) : nullptr);
        Window_NotifyC00(child, nullptr, 2, 1);
        panelparts::font6(child);
        vcall<void>(child, 0x1c, sum);
        panelparts::red(child);
        const u32 third = panelparts::newestOfRing(rings + 0x1f5a4);
        child = panelparts::numberPart(figures, 0x12c4c7);
        vcall<void>(child, 0x1c, third - sum);
        panelparts::red(child);
        vcall<void>(panelparts::numberPart(figures, 0x12c4c3), 0x1c, first - third);
    }
    u8 *const list = Window_FindChild(dialog(), nullptr, 0x12c4bd);
    g_open->buildRows(list, nullptr, reinterpret_cast<u8 *(__cdecl *)()>((uintptr_t)0x004a02b0), reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)0x004a03d0));   // (List7cb264_NewRow, List7cb264_InitRow, by the original's addresses: the hooks)
    Window7059f0_SetColumnBottom(list, nullptr, 0, 0);
    for (u32 column = 1; column < 6; ++column) Window7059f0_SetColumnBottom(list, nullptr, column, 1);
    Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))));
    Window_SetHandler(list, nullptr, 0x004a06b0);
    g_open->fillList(list);
    static const struct {
        u32 id, text, value;
    } kButtons[3] = {{0x10, 0xac, 0xde}, {0x13, 0xaf, 0xe1}, {0x14, 0xb0, 0xe2}};
    u32 &font = gDialog7cb25c_Open_752938;
    for (const auto &button : kButtons) {
        u8 *const child = Window_FindChild(list, nullptr, button.id);
        ButtonText_Init(child, (u32)(uintptr_t)Language_WideText(button.text), font, 0xff, 0xef, 0, button.value);
    }
    static const u32 kBlank[3] = {0x11, 0x12, 0x15};
    for (u32 id : kBlank) ButtonText_Init(Window_FindChild(list, nullptr, id), 0, font, 0xff, 0xef, 0, 0xffffffff);
    u8 *const group = Window_FindChild(list, nullptr, 0x12c4be);
    g_open->selectRadio(group, nullptr, gDialog7cb25c_7cb260 ? 0x12c4bf : 0x12c4c0);
    g_open->setTexts();
    Advisor_PostId(0xc3);
}

void __cdecl Dialog7cb25c_SetTexts() {
    if (!dialog()) return;
    u8 *const panel = Window_FindChild(dialog(), nullptr, 0x12c4bd);
    static const u32 kPlain[3][3] = {{0x11, 0xae, 0xdf}, {0x12, 0xad, 0xe0}, {0x15, 0xb1, 0xe3}};
    static const u32 kFlagged[3][3] = {{0x11, 0xb2, 0xe5}, {0x12, 0xb3, 0xe6}, {0x15, 0xb4, 0xe7}};
    const bool flagged = gDialog7cb25c_7cb260 != 0;
    for (const auto &row : flagged ? kFlagged : kPlain) {
        u8 *const child = Window_FindChild(panel, nullptr, row[0]);
        Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_WideText(row[1]));
        *reinterpret_cast<u32 *>(child + 0x90) = row[2];
        *reinterpret_cast<u32 *>(child + 0x94) = row[2];
    }
    u8 *const title = Window_FindChild(dialog(), nullptr, 0x12c4ca);
    vcall<void>(title, 0x24, Language_WideText(flagged ? 0xab : 0xaa));
}

void __cdecl Dialog7cb25c_FillList(u8 *list) {
    Window7059f0_Clear(list);
    for (u32 i = 0; i < 8; i++) {
        u32 offer[8];
        const u32 *const got = Money_LoanOffer(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(
                                                   *reinterpret_cast<u8 **>((uintptr_t)0x007cf83c))), nullptr, offer, i);
        for (u32 k = 0; k < 8; k++) offer[k] = got[k];
        const u32 flag = gDialog7cb25c_7cb260;
        u32 data;
        u32 left;   // (the months left: the term, less those paid for a loan taken)
        if (flag == 0 && offer[5] == 0 && offer[0] != 0) {
            left = offer[3];
            data = i + 1;
        } else if (flag == 1 && offer[5] != 0) {
            left = offer[3] - offer[6];
            data = ~i;   // (-1 - i: the loans taken counted down from -1)
        } else {
            continue;
        }
        const u8 *const text = Records_DataIfTerminated(*reinterpret_cast<const u8 *const *>((uintptr_t)0x0078bac4), nullptr, offer[7]);
        const u32 texts[6] = {(u32)(uintptr_t)Language_Widen(text), offer[1], left, offer[2], offer[4], offer[4] * left};
        Window7059f0_AddItem(list, nullptr, texts, data, -1, 1);
    }
}

void __cdecl Dialog7cb25c_Refresh() {
    if (dialog() && gDialog7cb25c_7cb260 == 1)
        Dialog7cb25c_FillList(Window_FindChild(dialog(), nullptr, 0x12c4bd));
}

s32 __cdecl Dialog7cb25c_ListProc(u8 *list, u32 message, u32 a, u32 b) {
    if (message != 0x15 && message == 0x101 && b == 1) {
        gDialog7cb25c_7cb260 = a == 0x12c4bf ? 1 : 0;
        Dialog7cb25c_FillList(list);
        Dialog7cb25c_SetTexts();
    }
    return Window_CallDefault(list, nullptr, message, a, b);
}

static u8 *money() {
    return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(*reinterpret_cast<u8 **>((uintptr_t)0x007cf83c)));
}

u32 __cdecl Dialog7cb25c_PayOffLoan(u32 index) {
    Money_PayOffLoan(money(), nullptr, index);
    Dialog7cb25c_FillList(Window_FindChild(dialog(), nullptr, 0x12c4bd));
    return 1;
}

u32 __cdecl Dialog7cb25c_TakeLoan(u32 index) {
    Money_TakeLoan(money(), nullptr, index);
    Dialog7cb25c_FillList(Window_FindChild(dialog(), nullptr, 0x12c4bd));
    return 1;
}
}  // extern "C"
