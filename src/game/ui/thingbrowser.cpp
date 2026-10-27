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
#include "../world/mousemode.h"
#include "../world/staff.h"
#include "cursor.h"
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
}  // namespace

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
}
