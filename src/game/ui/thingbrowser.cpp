#include "thingbrowser.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/idmaps.h"
#include "../core/language.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../render/textparts.h"
#include "../world/advisor.h"
#include "../world/camera.h"
#include "../world/cellmessage.h"
#include "../world/mousemode.h"
#include "../world/staff.h"
#include "../world/thingscan.h"
#include "cursor.h"
#include "dialog7ca098.h"
#include "dialogs.h"
#include "focus.h"
#include "panel7cc3b0.h"
#include "panel7cc408.h"
#include "radiogroup.h"
#include "window.h"
#include "window705a50.h"
#include "windowreg.h"
#include "windowscript.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

bool is_staff(u8 kind) { return kind == 4 || kind == 5 || kind == 6 || kind == 8 || kind == 7; }

u16 *const kText = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);

const ThingPatrolEnv kPatrolGameEnv = {&Camera_CentreOnSprite, &Map_SendLine};
const ThingPatrolEnv *g_patrol = &kPatrolGameEnv;
}  // namespace

void ThingBrowser_SetPatrolEnv(const ThingPatrolEnv *env) { g_patrol = env ? env : &kPatrolGameEnv; }

extern "C" {

void __cdecl ThingBrowser_ShowName() {
    if (!global<u8 *>(0x007cc308)) return;
    u8 *const label = Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x147);
    u8 *const thing = global<u8 *>(0x007cc308);
    if (is_staff(thing[2])) {
        vcall<void>(label, 0x24, Staff_GetName(thing));
    } else {
        const u32 count = at<u32>(thing, 0x1d8);
        if (!count) {
            TextFormat().format(global<void *>(0x0078ba94), kText, 0x3fff, 0x1c9);
        } else {
            u8 arg[0x14];  // (a number argument, as TextArgInt_Ctor (n 0x18, count, unsigned) builds it inline)
            at<u32>(arg, 0) = 0x006ff170;
            at<u32>(arg, 4) = 0x5641524d;
            at<u32>(arg, 8) = 0x18;
            at<u32>(arg, 0xc) = count;
            at<u32>(arg, 0x10) = 0;
            TextFormat().format(global<void *>(0x0078ba94), kText, 0x3fff, 0x1c8, arg);
        }
        vcall<void>(label, 0x24, kText);
    }
    ThingBrowser_ShowDetail();
    Camera_CentreOnSprite(global<u8 *>(0x007cc308));
}

void __cdecl ThingBrowser_ResetIfThing(u8 *thing) {
    if (global<u8 *>(0x007cc308) == thing) Dialog7cb2b8_ResetChild21(1);
}

void __cdecl ThingBrowser_ShowDetail() {
    u8 *const thing = global<u8 *>(0x007cc308);
    if (!thing) return;
    if (is_staff(thing[2])) {
        const u32 code = Index_ToCode485ec0(Mode19c_ToIndex(thing));
        if (code == global<u32>(0x00756b98)) return;
        global<u32>(0x00756b98) = code;
        Window_Notify1(Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x148), nullptr, code);
        return;
    }
    const u32 count = at<u32>(thing, 0x30);
    u8 *const child = Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x14a);
    vcall<void>(child, 0x1c, count ? count - 1 : 0x29au);
}

u32 __cdecl ThingBrowser_GroupHandler(u8 *group, u32 message, u32 a, u32 b) {
    if (message != 0x100) return (u32)Window_CallDefault(group, nullptr, message, a, b);
    u8 *const child = Window_FindChild(group, nullptr, a);
    RadioGroup_Select(group, nullptr, a);
    if (b != 1) Window705a50_SetState(child, nullptr, 1, 1);
    Window_PostMessage(reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(group)), 0x101, a, 1);
    return 1;
}

u32 __cdecl ThingBrowser_PanelHandler(u8 *panel, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        global<u32>(0x007cc304) = 0;
    } else if (message == 0x10001 || message == 0x10002) {
        Window_SetShown(Window_FindChild(panel, nullptr, 0x149), nullptr, message == 0x10001);
    } else if (message == 0x10006 && a == 0) {
        u8 *const thing = global<u8 *>(0x007cc308);
        if (is_staff(thing[2])) Panel7cc3b0_Show(reinterpret_cast<u8 *>(0x007cc3b0), nullptr, thing);
        else Panel7cc408_Show(reinterpret_cast<u8 *>(0x007cc408), nullptr, thing);
    }
    return (u32)Window_CallDefault(panel, nullptr, message, a, b);
}

void __cdecl ThingBrowser_Create() {
    WindowScript_Build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>(0x00756950), 0);
    global<u8 *>(0x007cc2fc) = Window_FindChild(global<u8 *>(0x00faa5d8), nullptr, 0x95c);
    global<u8 *>(0x007cc300) = Window_FindChild(global<u8 *>(0x00faa5d8), nullptr, 0x963);
    Window_SetHandler(global<u8 *>(0x007cc2fc), nullptr, 0x004b3790);
    Window_SetHandler(global<u8 *>(0x007cc300), nullptr, 0x004b3790);
    Window_Unlink(global<u8 *>(0x007cc2fc));
    Window_Unlink(global<u8 *>(0x007cc300));
    Window_SetDwordA0(global<u8 *>(0x007cc2fc), nullptr, 0, 3);
    Window_SetDwordA0(global<u8 *>(0x007cc300), nullptr, 0, 3);
    Window_SetHandler(Window_FindChild(global<u8 *>(0x007cc2fc), nullptr, 0x95d), nullptr, 0x004b3b90);
    Window_SetHandler(Window_FindChild(global<u8 *>(0x007cc300), nullptr, 0x95d), nullptr, 0x004b3b90);
    Window_SetShown(global<u8 *>(0x007cc2fc), nullptr, 0);
    Window_SetShown(global<u8 *>(0x007cc300), nullptr, 0);
    u8 *const panel = WindowScript_Build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>(0x00756ad8), 0x004b3c10);
    global<u8 *>(0x007cc304) = panel;
    Window_SetCursor(panel, nullptr, 2);
    Window_Unlink(global<u8 *>(0x007cc304));
    u8 *const name = Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x147);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(name, nullptr, memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr);
    Window_Notify40000(name, nullptr, 0xff, 0xff, 0xff, 0xff);
    Window_Set4c(name, nullptr, Language_Font(0xb));
    Window_NotifyImpl100(name, nullptr, 0);
    vcall<void>(name, 0x14, 0u);
    vcall<void>(Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x148), 0x14, 0u);
    u8 *const hilight = Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x149);
    Window_SetObject98(hilight, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))));   // "hilight"
    vcall<void>(hilight, 0x14, 0u);
    Window_SetShown(hilight, nullptr, 0);
    Window_SetShown(global<u8 *>(0x007cc304), nullptr, 0);
    global<u32>(0x007cc2f0) = 0;
}

void __cdecl ThingBrowser_Open(u8 *thing) {
    u8 *window;
    if (thing && thing[2] == 1) {
        window = global<u8 *>(0x007cc300);
        global<u32>(0x007cc2f0) = 2;
    } else {
        window = global<u8 *>(0x007cc2fc);
        global<u32>(0x007cc2f0) = 1;
    }
    Dialog7cb2b8_Child21Post((u32)(uintptr_t)window);   // (a second argument 0 pushed, unread)
    u8 *const group = Window_FindChild(window, nullptr, 0x95d);
    if (thing) {
        global<u8 *>(0x007cc308) = thing;
        u32 id = 0;
        switch (thing[2]) {
        case 1: global<u32>(0x007cc30c) = 6; id = 0x964; break;
        case 4: global<u32>(0x007cc30c) = 2; id = 0x95f; break;
        case 5: global<u32>(0x007cc30c) = 1; id = 0x95e; break;
        case 6: global<u32>(0x007cc30c) = 4; id = 0x961; break;
        case 7: global<u32>(0x007cc30c) = 3; id = 0x962; break;
        case 8: global<u32>(0x007cc30c) = 5; id = 0x960; break;
        }
        if (id) RadioGroup_Select(group, nullptr, id);
        Window_AddChild(global<u8 *>(0x007cb2b8), nullptr, global<u8 *>(0x007cc304), 1, 1);
        Window_SetShown(global<u8 *>(0x007cc304), nullptr, 1);
        ThingBrowser_ShowName();
        global<u32>(0x00790ab0) |= 0x1000;
    } else {
        global<u8 *>(0x007cc308) = nullptr;
        global<u32>(0x007cc30c) = 0;
        RadioGroup_Select(group, nullptr, 0xffffffffu);
    }
    const bool six = global<u32>(0x007cc30c) == 6;
    Window_SetShown(Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x148), nullptr, six ? 0 : 1);
    Window_SetShown(Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x14a), nullptr, six ? 1 : 0);
    Advisor_PostId(six ? 0x11f : 0xc6);
    Window7c24c8_Close();
    MouseModes_EnsureHolder();
    u8 *const current = global<u8 *>(0x007b05d8);
    if ((current ? vcall<s32>(current, 0x24) : 0) == 9) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
        u8 *const mode = block ? MouseModeOnlineBase_Ctor(block) : nullptr;
        MouseModes_EnsureHolder();
        MouseMode_Set(mode);
    }
}

void __cdecl ThingBrowser_SendPatrolLines() {
    if (!global<u8 *>(0x007cc308) || !(global<u32>(0x00790ab0) & 0x1000)) return;
    g_patrol->centre(global<u8 *>(0x007cc308));
    u8 *const thing = global<u8 *>(0x007cc308);
    if (!is_staff(at<u8>(thing, 2)) || !Staff_HasPatrolRegion(thing)) return;
    u16 first, last;
    Staff_GetPatrolRegion(thing, nullptr, &first, &last);
    const u32 cell0 = (u32)first - 1, cell1 = (u32)last - 1;
    const s32 column0 = (s32)(cell0 & 0x7f), row0 = (s32)(cell0 >> 7), column1 = (s32)(cell1 & 0x7f), row1 = (s32)(cell1 >> 7);
    const s32 from = column0 == column1 ? column0 : column0 + 1, to = column0 == column1 ? column1 : column1 - 1;
    s32 end[2] = {to, row0};   // (x, y adjacent as the original's: the bench logs the two through the first)
    g_patrol->line(0x136, from, row0, &end[0], &end[1]);
    end[0] = column1;
    end[1] = row1;
    g_patrol->line(0x136, column1, row0, &end[0], &end[1]);
    end[0] = to;
    end[1] = row1;
    g_patrol->line(0x136, from, row1, &end[0], &end[1]);
    end[0] = column0;
    end[1] = row0;
    g_patrol->line(0x136, column0, row1, &end[0], &end[1]);
}

void __cdecl ThingBrowser_OpenShown() {
    ThingBrowser_Open(global<u32>(0x007cc3c0) ? global<u8 *>(0x007c2658) : nullptr);
}
}

// The thing browser's handler (0x4b3790).
namespace {

const ThingBrowserHandlerEnv kGameHandlerEnv = {ThingBrowser_SendPatrolLines, ThingBrowser_ShowDetail, ThingBrowser_Step, ThingBrowser_ShowName, Window_SetShown, Window_Unlink, Window_AddChild, Game_CancelAdvisor, Window_CallDefault};
const ThingBrowserHandlerEnv *g_handler = &kGameHandlerEnv;

u32 &word(u32 address) { return *reinterpret_cast<u32 *>((uintptr_t)address); }
u8 *panel() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cc304); }

void panelAway() {
    g_handler->setShown(panel(), nullptr, 0);
    g_handler->unlink(panel());
}

void panelAdded() {
    g_handler->addChild(*reinterpret_cast<u8 **>((uintptr_t)0x007cb2b8), nullptr, panel(), 1, 1);
    g_handler->setShown(panel(), nullptr, 1);
}

void nameShown() {
    g_handler->showName();
    word(0x00790ab0) |= 0x1000;
}

void modeButton(u32 mode) {
    if (word(0x007cc30c) == 0) {
        word(0x007cc30c) = mode;
        g_handler->step();
        if (!word(0x007cc308)) return;
        panelAdded();
        nameShown();
        return;
    }
    const u32 was = word(0x007cc308);
    if (word(0x007cc30c) != mode) word(0x007cc308) = 0;
    word(0x007cc30c) = mode;
    g_handler->step();
    if (!word(0x007cc308)) {
        panelAway();
        word(0x00790ab0) &= ~0x1000u;
        return;
    }
    if (!was) panelAdded();
    nameShown();
}
}  // namespace

void ThingBrowser_SetHandlerEnv(const ThingBrowserHandlerEnv *env) { g_handler = env ? env : &kGameHandlerEnv; }

extern "C" {

s32 __cdecl ThingBrowser_Handler(u8 *window, u32 message, u32 a, u32 b) {
    static const u8 kModes[7] = {1, 2, 5, 4, 3, 0, 6};
    switch (message) {
    case 0x14: word(0x007cc2fc) = 0; break;
    case 0x1e:
        g_handler->sendPatrol();
        g_handler->showDetail();
        break;
    case 0x101:
        if (word(0x007cc2f0) && a - 0x95e <= 6 && b == 1 && kModes[a - 0x95e]) modeButton(kModes[a - 0x95e]);
        break;
    case 0x1001d8:
        word(0x007cc308) = 0;
        panelAway();
        word(0x007cc2f0) = 0;
        word(0x00790ab0) &= ~0x1000u;
        g_handler->cancelAdvisor(0x11f);
        g_handler->cancelAdvisor(0xc6);
        break;
    default: break;
    }
    return g_handler->callDefault(window, nullptr, message, a, b);
}
}
