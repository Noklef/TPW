// The figures of an object on the dialog [0x7cc1f8]: see dialog7cc1f8stats.h.
#include "dialog7cc1f8stats.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/language.h"
#include "../world/objectdefs.h"
#include "../world/world.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cc1f8StatsEnv kGameEnv = {ObjectDefs_Find, ObjectControls_GetWord18, Language_WideText, Window_NotifyImpl100};
const Dialog7cc1f8StatsEnv *g_env = &kGameEnv;

const u16 *numberFormat() { return reinterpret_cast<const u16 *>((uintptr_t)0x0074f9d8); }  // "%d"

// Slot 7 of the child, when there is one: the value out of 100 as out of 1024 (a dword, wrapping).
void setLevel(u8 *panel, u32 id, u32 value) {
    u8 *const child = Window_FindChild(panel, nullptr, id);
    if (!child) return;
    reinterpret_cast<void(__fastcall *)(u8 *, void *, s32)>(at<uintptr_t>(at<u8 *>(child, 0), 0x1c))(child, nullptr, (s32)(value << 10) / 100);
}

}  // namespace

void Dialog7cc1f8_SetStatsEnv(const Dialog7cc1f8StatsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7cc1f8_ShowFigures(const s16 *id) {
    u8 *const panel = Window_FindChild(global<u8 *>(0x007cc1f8), nullptr, 0x1ed);
    if (!panel || *id < 0) return;
    const u8 *const def = g_env->find(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, (u16)*id);
    GameTable7cfb90_Get1da720_519510(gWorldPtr);   // (its answer only the ECX of the stdcall)
    const u32 control = g_env->control((u16)*id);
    u16 text[10];
    if (control) Crt().swprintf(text, numberFormat(), control);
    else Crt().swprintf(text, g_env->text(0x88));
    if (u8 *const child = Window_FindChild(panel, nullptr, 0x1f5)) g_env->notify(child, nullptr, (u32)(uintptr_t)text);
    const u32 mode = global<u32>(0x007cc1fc);
    if (mode != 0 && mode != 2) return;
    setLevel(panel, 0x1ee, at<u32>(def, 0x13c));
    if (mode == 2) return;
    if (u8 *const child = Window_FindChild(panel, nullptr, 0x1f2)) {
        u16 *const wide = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
        Crt().swprintf(wide, numberFormat(), at<u32>(def, 0x19c));
        reinterpret_cast<void(__fastcall *)(u8 *, void *, u16 *)>(at<uintptr_t>(at<u8 *>(child, 0), 0x24))(child, nullptr, wide);
    }
    setLevel(panel, 0x1ef, at<u32>(def, 0x1b0));
}
}
