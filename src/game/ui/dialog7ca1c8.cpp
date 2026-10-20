#include "dialog7ca1c8.h"

#include <cstdint>

#include "../coaster/registry.h"
#include "cursor.h"
#include "radiogroup.h"
#include "window.h"
#include "windowreg.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

using SetCheck = void(__fastcall *)(u8 *, void *, bool);

void setCheck(u8 *panel, u32 id, bool on) {
    u8 *const child = Window_FindChild(panel, nullptr, id);
    reinterpret_cast<SetCheck>(at<void **>(child, 0)[5])(child, nullptr, on);
}
}

extern "C" {

void __cdecl Dialog7ca1c8_SetChecks(u32 flags) {
    u8 *const panel = Window_FindChild(global<u8 *>(0x007ca1c8), nullptr, 0x18);
    setCheck(panel, 0x1f, (flags & 1) != 0);
    bool second = (flags & 2) != 0;
    if (second && at<u8>(global<u8 *>(0x007ca1d0), 0x50) == 0) second = false;
    setCheck(panel, 0x20, second);
    setCheck(panel, 0x1b, (flags & 4) != 0);
    setCheck(panel, 0x19, (flags & 8) != 0);
    setCheck(panel, 0x1a, (flags & 0x10) != 0);
    setCheck(panel, 0x1c, (flags & 0x20) != 0);
    setCheck(panel, 0x21, (flags & 0x40) != 0);
    setCheck(panel, 0x1e, (flags & 0x80) != 0);
    setCheck(panel, 0x1d, (flags & 0x100) != 0);
}

void __cdecl Dialog7ca1c8_SetChecksIfUp(u32 flags) {
    if (global<u8 *>(0x007ca1c8) && global<u32>(0x007ca1cc)) Dialog7ca1c8_SetChecks(flags);
}

void __fastcall Dialog7ca1c8_SetChecksIfUpThis(void *, void *, u32 flags) { Dialog7ca1c8_SetChecksIfUp(flags); }

u32 __cdecl Dialog7ca1c8_Focus() { return Window_SetFocus(global<u8 *>(0x007ca1c8), nullptr, 0); }

void __cdecl Dialog7ca1c8_SelectButton(u32 flags) {
    u8 *const group = Window_FindChild(global<u8 *>(0x007ca1c8), nullptr, 0x18);
    struct Pick { u32 bit, id, cursor; };   // (cursor 0: none)
    static const Pick picks[] = {{4, 0x1b, 0},     {1, 0x1f, 0},     {2, 0x20, 0},     {8, 0x19, 0xe},    {0x10, 0x1a, 0xc},
                                 {0x20, 0x1c, 0xd}, {0x40, 0x21, 0xa}, {0x80, 0x1e, 0xf}, {0x100, 0x1d, 0x10}};
    for (const Pick &p : picks) {
        if (!(flags & p.bit)) continue;
        RadioGroup_Select(group, nullptr, p.id);
        if (p.cursor) GameCursor_Set(p.cursor);
        return;
    }
    RadioGroup_Select(group, nullptr, 0xffffffffu);
}

void __cdecl Dialog7ca1c8_SelectButtonIfUp(u32 flags) {
    if (global<u8 *>(0x007ca1c8) && global<u32>(0x007ca1cc)) Dialog7ca1c8_SelectButton(flags);
}

void __fastcall Dialog7ca1c8_SelectButtonIfUpThis(void *, void *, u32 flags) { Dialog7ca1c8_SelectButtonIfUp(flags); }

void __cdecl Dialog7ca1c8_UpdateLoopButton() {
    u8 *const button = Window_FindChild(Window_FindChild(global<u8 *>(0x007ca1c8), nullptr, 0x18), nullptr, 0x20);
    using Show = void(__fastcall *)(u8 *, void *, u32);
    const auto show = [button](u32 on) { reinterpret_cast<Show>(at<void **>(button, 0)[5])(button, nullptr, on); };
    if (at<u8>(global<u8 *>(0x007ca1d0), 0x50) == 0) {
        Window_SetObject98(button, nullptr, nullptr);
        show(0);
        return;
    }
    u32 tools;
    CoasterEdit_Tools(&tools);
    show(1);
    const bool trough = (tools & 0x200) != 0;
    Window_SetObject98(button, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(trough ? "cb_trough" : "cb_loop")));
    at<u32>(button, 0x90) = at<u32>(button, 0x94) = trough ? 0x134 : 0x132;
    show((tools & 2) ? 1 : 0);
}
}
