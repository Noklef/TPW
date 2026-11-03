#include "dialog7ca1c8.h"

#include <cstdint>

#include "../coaster/edittools.h"
#include "../coaster/registry.h"
#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/globals.h"
#include "../core/vcall.h"
#include "../world/focuscell.h"
#include "../world/gamemode.h"
#include "../world/mousemode.h"
#include "cursor.h"
#include "dialogs.h"
#include "radiogroup.h"
#include "window.h"
#include "windowreg.h"
#include "windowscript.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

using SetCheck = void(__fastcall *)(u8 *, void *, bool);

void setCheck(u8 *panel, u32 id, bool on) {
    u8 *const child = Window_FindChild(panel, nullptr, id);
    reinterpret_cast<SetCheck>(at<void **>(child, 0)[5])(child, nullptr, on);
}

const Dialog7ca1c8OpenEnv kOpenGameEnv = {FocusCell_Update, Dialog7cb2b8_ResetChild21, Dialog7cb2b8_Child21Post, Window_SetFocus, Object_LinkOpen, MouseMode_EnterTile15, CoasterEdit_EditRide,
                                          MouseMode_Ensure6feaa0, GameCursor_Set, CoasterEdit_Tools, Dialog7ca1c8_SetChecks, CoasterEdit_StartChosenTool, Dialog7ca1c8_SelectButton,
                                          Dialog7ca1c8_UpdateLoopButton};
const Dialog7ca1c8OpenEnv *g_open = &kOpenGameEnv;
}

void Dialog7ca1c8_SetOpenEnv(const Dialog7ca1c8OpenEnv *env) { g_open = env ? env : &kOpenGameEnv; }

namespace {
const Dialog7ca1c8ResetEnv kResetGameEnv = {Global81ae2c_Get_52f840, GameHeap_Alloc, MouseModeOnlineBase_Ctor, MouseMode_Set, GameMode_Set};
const Dialog7ca1c8ResetEnv *g_reset = &kResetGameEnv;

u8 *newBaseMode() {
    u8 *const block = static_cast<u8 *>(g_reset->alloc(8));
    return block ? g_reset->baseCtor(block) : nullptr;
}

// MouseModes_EnsureHolder, inline in the original.
void ensureHolder() {
    u8 *&holder = *reinterpret_cast<u8 **>((uintptr_t)0x007b05e8);
    if (holder) return;
    u8 *const block = static_cast<u8 *>(g_reset->alloc(1));
    if (block) {
        g_reset->setMode(newBaseMode());
        *reinterpret_cast<u32 *>((uintptr_t)0x007b05e0) = 0xffffffffu;
        *reinterpret_cast<u32 *>((uintptr_t)0x007b05d4) = 0xffffffffu;
    }
    holder = block;
}
}  // namespace

void Dialog7ca1c8_SetResetEnv(const Dialog7ca1c8ResetEnv *env) { g_reset = env ? env : &kResetGameEnv; }

void __cdecl Dialog7ca1c8_ResetMouseMode() {
    *reinterpret_cast<u32 *>((uintptr_t)0x007ca1d0) = 0;
    if (g_reset->mode() == 3) return;
    ensureHolder();
    u8 *const current = *reinterpret_cast<u8 **>((uintptr_t)0x007b05d8);
    if ((current ? vcall<u32>(current, 0x24) : 0) == 1) return;
    u8 *const base = newBaseMode();
    ensureHolder();
    g_reset->setMode(base);
    g_reset->gameMode(0, 1);
    g_reset->gameMode(0, 1);
}

extern "C" {

void __cdecl Dialog7ca1c8_OpenFor(u8 *thing) {
    if (global<u32>(0x007ca1cc) && global<u8 *>(0x007ca1d0) == thing) return;
    if (global<u8>(0x007ca1d4)) return;
    global<u8 *>(0x007ca1d0) = thing;
    Global7c2654_Set((u32)(uintptr_t)thing);
    g_open->focusCell(1);
    if (!global<u32>(0x00785a2c)) {
        g_open->resetChild(1);
        g_open->childPost((u32)(uintptr_t)global<u8 *>(0x007ca1c8));
    }
    global<u32>(0x007ca1cc) = 1;
    g_open->setFocus(global<u8 *>(0x007ca1c8), nullptr, 0);
    if (!g_open->linkOpen(thing)) {
        g_open->enterTile();
    } else {
        g_open->editRide(global<u8 *>(0x007ca1d0), 1);
        g_open->ensureMode();
        g_open->cursor(0xc);
        Global7c2654_Set((u32)(uintptr_t)thing);
        g_open->focusCell(1);
    }
    if (global<u32>(0x007ca1cc)) {
        u32 slot = (u32)(uintptr_t)thing;   // (the argument's own stack slot in the original)
        g_open->tools(&slot);
        g_open->setChecks(slot);
        g_open->startTool(&slot);
        g_open->selectButton(slot);
    }
    g_open->updateLoop();
    global<u32>(0x007ca1cc) = 1;
}

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

void __cdecl Dialog7ca1c8_Create() {
    global<u8 *>(0x007ca1c8) = WindowScript_Build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>(0x00751530), 0x004982a0);
    Window_Unlink(global<u8 *>(0x007ca1c8));
    Window_SetDwordA0(global<u8 *>(0x007ca1c8), nullptr, 0, 2);
    Window_SetShown(global<u8 *>(0x007ca1c8), nullptr, 0);
}
}
