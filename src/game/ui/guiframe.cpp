#include "guiframe.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/framecountdown.h"
#include "../core/globals.h"
#include "../core/thunks.h"
#include "guimanager.h"
#include "imecandidates.h"
#include "msgqueue.h"
#include "window.h"
#include "windowtimers.h"
#include "guisystem.h"
#include "cursor.h"
#include "guistartup.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const GuiFrameEnv kGameEnv = {
    reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x0047f620),
    &WindowTimers_FireAll,
    &Fn_669c30,
    &Countdown7cc150_Step,
    reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x004a9180),
    &MsgQueue_Pump,
    &Window_FireTimers,
    &Window_PostMessage,
    reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x00658f6c),
    &Global7b4b88_Get_47eaa0,
    &GuiClock_Now,
    reinterpret_cast<u32(__cdecl *)(u32)>((uintptr_t)0x0067a7d0),
    &Wrap_Obj_Get64Plus10_5f5640,
    &Gui_MouseMove,
    reinterpret_cast<void(__cdecl *)(u32, u32, u32, u32)>((uintptr_t)0x005f5590),
    &MsgQueue_PumpSecond,
    &ImeHost_Update,
};
const GuiFrameEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void GuiFrame_SetEnv(const GuiFrameEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Gui_Frame() {
    const GuiFrameEnv *const E = g_env;
    E->begin();
    E->fireTimers();
    gGui_Frame_7b9a4c = E->count(gGuisystem_faa5f0);
    global<u32>(0x007b9bac) = E->count(gGuisystem_faa5f4);
    if (E->countdownStep()) E->countdownDone();
    E->pump(0x400);
    E->fireWindowTimers();
    E->post(global<u8 *>(0x00faa5d8), 2, 0, 0);
    E->afterPost();
    if (E->active()) {
        if (gCursor_faa608) {
            u32 *const vtable = global<u32 *>(reinterpret_cast<u32 &>(gCursor_faa608));
            const u32 now = E->clock();
            reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)vtable[6])(gCursor_faa608, nullptr, now);
        }
        if (!(gGui_Frame_7c20e4 & 1)) {
            gGui_Frame_7c20e4 |= 1;
            gGui_Frame_7b9d00 = 0;
            gGui_Frame_7b9d04 = 0;
            E->atExit(0x0048a6d0);
        }
        const u32 *const mouse = reinterpret_cast<const u32 *>((uintptr_t)E->mouse());
        if (mouse[0] != gGui_Frame_7b9d00 || mouse[1] != gGui_Frame_7b9d04) {
            const u32 x = mouse[0];
            const u32 y = mouse[1];
            gGui_Frame_7b9d00 = x;
            gGui_Frame_7b9d04 = y;
            const s32 scaledY = static_cast<s32>(y * 3 << 9) / global<s16>(0x00faa5c0);
            const s32 scaledX = static_cast<s32>(x << 11) / global<s16>(0x00faa5c4);
            E->mouseMove(static_cast<u32>(scaledX), static_cast<u32>(scaledY));
        }
        E->update(reinterpret_cast<uintptr_t>(&gGui_7b9fe0), 0, 0, 0);
    }
    E->pumpSecond(0x1000);
    E->ime(reinterpret_cast<u8 *>((uintptr_t)&gImeHost_7b4e38));
}
}
