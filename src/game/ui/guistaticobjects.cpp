// The GUI library's static objects: see guistaticobjects.h.
#include "guistaticobjects.h"

#include <cstdint>

#include "../core/initializers.h"
#include "../core/imports.h"
#include "../core/addrcalls.h"

namespace {
template <typename T> T game(u32 address) { return GameAddress<T>(address); }
const GuiStaticObjectsEnv kGameEnv = {&Fn_669fa8, &Fn_669fee, game<u32(__cdecl *)(u32)>(0x0067a7d0)};
const GuiStaticObjectsEnv *g_env = &kGameEnv;
}  // namespace

void GuiStaticObjects_SetEnv(const GuiStaticObjectsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl GuiStatic_Init() {
    GuiStatic_Make();
    GuiStatic_AtExit();
}

void __cdecl GuiStatic_Make() { g_env->make(game<u8 *>(0x00faa550)); }
void __cdecl GuiStatic_AtExit() { g_env->atExit(0x006584d0); }
void __cdecl GuiStatic_Destroy() { g_env->destroy(game<u8 *>(0x00faa550)); }
u32 __fastcall GuiClock_Time(u8 *) { return (*game<u32(__stdcall *const *)()>(IMPORT(Winmm_timeGetTime)))(); }
void __cdecl GuiClock_Init() { GuiClock_Make(); }
void __cdecl GuiClock_Make() { GuiClock_Ctor(game<u8 *>(0x00faa580)); }

u8 *__fastcall GuiClock_Ctor(u8 *clock) {
    Fn_659770(clock);
    *reinterpret_cast<u32 *>(clock) = (u32)(uintptr_t)GuiClock_vtable;
    return clock;
}
}
