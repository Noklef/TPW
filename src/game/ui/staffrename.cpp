#include "staffrename.h"

#include <cstdint>

#include "../core/keymap.h"
#include "../core/vcall.h"
#include "../files/wrapsources.h"
#include "../world/staff.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../render/textparts.h"
#include "focus.h"
#include "hoverlabel.h"
#include "window.h"
#include "window705a20.h"
#include "windowreg.h"
#include "windowscript.h"
#include "panel7cc218.h"

namespace {

const StaffRenameEnv kGameEnv = {WindowScript_Build, Window_SetDepth, Window_SetDwordA0, GameHeap_Alloc, TextPart6ff530_Ctor, TextPart6ff4d0_Ctor, Window_SetImpl, Language_Font, Window_Set4c, Window_NotifyC00, Language_WideText,
                                 HoverLabel_SetYellow, Window_Notify40000, Window705a20_SetBuffer, GuiWindows_FindByName, Window705a20_SetCaret, Staff_GetName, Window705a20_SelectAll, KeyMaps_ResetAllAndSetFlag, Window_SetFocus};
const StaffRenameEnv *g_env = &kGameEnv;

}  // namespace

void StaffRename_SetEnv(const StaffRenameEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall StaffRename_Open(u8 *self) {
    u8 *&dialog = *reinterpret_cast<u8 **>(self + 0xc);
    if (dialog) return;
    u8 *const thing = gPanel7cc218_7c2658;
    if (!thing || (u8)(thing[2] - 4) > 4) return;
    u8 *const parent = *reinterpret_cast<u8 **>(self + 0x10);
    const s32 depth = *reinterpret_cast<s16 *>(parent + 0xe0);
    u8 *const window = g_env->build(parent, nullptr, gStaffRename_Open_7572e8, 0x004b6320);
    dialog = window;
    g_env->setDepth(window, nullptr, (u32)(depth + 10));
    g_env->setDwordA0(window, nullptr, 0, (u32)(uintptr_t)thing);
    g_env->setDwordA0(window, nullptr, 1, (u32)(uintptr_t)self);
    u8 *const label = static_cast<u8 *>(g_env->alloc(0x1a4));
    u8 *const labelPart = label ? g_env->labelPart(label, nullptr, 0) : nullptr;
    g_env->setImpl(Window_FindChild(window, nullptr, 0x5daa), nullptr, labelPart);
    u8 *const labelFont = g_env->font(5);
    g_env->set4c(Window_FindChild(window, nullptr, 0x5daa), nullptr, labelFont);
    g_env->notifyC00(Window_FindChild(window, nullptr, 0x5daa), nullptr, 1, 1);
    vcall<void>(Window_FindChild(window, nullptr, 0x5daa), 0x24, g_env->text(0xc3));
    g_env->hoverYellow(Window_FindChild(window, nullptr, 0x5daa));
    u8 *const edit = Window_FindChild(window, nullptr, 0x5dab);
    u8 *const memory = static_cast<u8 *>(g_env->alloc(0x1a4));
    g_env->setImpl(edit, nullptr, memory ? g_env->editPart(memory, nullptr, 0) : nullptr);
    g_env->set4c(edit, nullptr, g_env->font(7));
    g_env->notify40000(edit, nullptr, 0xff, 0xff, 0xff, 0xff);
    g_env->setBuffer(edit, nullptr, 0x007cc360, 0x20);
    g_env->setCaret(edit, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_env->findByName(reinterpret_cast<const char *>((uintptr_t)0x0074fa84))));
    vcall<void>(edit, 0x24, g_env->staffName(thing));
    g_env->selectAll(edit);
    g_env->keysOff();
    g_env->setFocus(edit, nullptr, 0);
}

s32 __cdecl StaffRename_Handler(u8 *window, u32 message, s32 a, u32 b) {
    if (message > 0x100) {
        if (message == 0x802) {
            Window_PostMessage(window, 0x100, 0xffffffff, 0);   // (the confirmation, as a click on the -1 button)
        } else if (message == 0x804) {
            Window_SendMessage(window, 4, 0, 0);
            return 0;
        }
    } else if (message == 0x100) {
        if (a == -1) {
            Staff_SetName(reinterpret_cast<u8 *>((uintptr_t)Window_DwordA0(window, nullptr, 0)), nullptr,
                          reinterpret_cast<const u16 *>((uintptr_t)0x007cc360));
            vcall<void>(reinterpret_cast<u8 *>((uintptr_t)Window_DwordA0(window, nullptr, 1)), 4);
            Window_SendMessage(window, 4, 0, 0);
            return 0;
        }
        if (a == -2) {
            Window_SendMessage(window, 4, 0, 0);
            return 0;
        }
    } else if (message == 0x14) {
        KeyMaps_EnableAllAndClearFlag();
        TrackedSource_Clear(reinterpret_cast<TrackedSource *>((uintptr_t)Window_DwordA0(window, nullptr, 1)));
        Focus_Restore();
        return Window_CallDefault(window, nullptr, message, (u32)a, b);
    }
    return Window_CallDefault(window, nullptr, message, (u32)a, b);
}
