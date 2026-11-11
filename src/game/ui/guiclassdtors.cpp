// The GUI library's classes' destructors: see guiclassdtors.h.
#include "guiclassdtors.h"

#include <cstdint>
#include "cursor.h"
#include "radiogroup.h"
#include "window705950.h"
#include "window705b20.h"
#include "window705b98.h"
#include "window705bc8.h"
#include "window705c58.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T game(u32 address) { return GameAddress<T>(address); }

const GuiClassDtorsEnv kGameEnv = {game<void(__fastcall *)(u8 *)>(0x0065c025),
                                   game<void(__fastcall *)(u8 *)>(0x0065b6d6),
                                   game<void(__fastcall *)(u8 *)>(0x00659d90),
                                   game<void(__fastcall *)(u8 *)>(0x006618a6),
                                   game<void(__cdecl *)(void *)>(0x0067acb0)};
const GuiClassDtorsEnv *g_env = &kGameEnv;

void windowOf(u8 *self, u32 vtable) {
    *reinterpret_cast<u32 *>(self) = vtable;
    g_env->windowDtor(self);
}

u8 *freedIf(u8 *self, u32 flags) {
    if (flags & 1) g_env->memFree(self);
    return self;
}

}  // namespace

void GuiClassDtors_SetEnv(const GuiClassDtorsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Window705950_Dtor(u8 *self) {
    *reinterpret_cast<u32 *>(self) = (u32)(uintptr_t)Window705950_vtable;
    g_env->clearList(self);
    g_env->rectDtor(self + 0x138);
    g_env->windowDtor(self);
}

void __fastcall SysCursor_Dtor(u8 *self) {
    *reinterpret_cast<u32 *>(self) = (u32)(uintptr_t)SysCursor_vtable;
    g_env->cursorDtor(self);
}

void __fastcall Window705ac0_Dtor(u8 *self) { windowOf(self, (u32)(uintptr_t)Window705ac0_vtable); }
void __fastcall Window705b20_Dtor(u8 *self) { windowOf(self, (u32)(uintptr_t)Window705b20_vtable); }
void __fastcall Window705b98_Dtor(u8 *self) { windowOf(self, (u32)(uintptr_t)Window705b98_vtable); }
void __fastcall Window705bc8_Dtor(u8 *self) { windowOf(self, (u32)(uintptr_t)Window705bc8_vtable); }
void __fastcall Window705c58_Dtor(u8 *self) { windowOf(self, (u32)(uintptr_t)Window705c58_vtable); }

u8 *__fastcall Window705950_Delete(u8 *self, void *, u32 flags) {
    Window705950_Dtor(self);
    return freedIf(self, flags);
}

u8 *__fastcall SysCursor_Delete(u8 *self, void *, u32 flags) {
    SysCursor_Dtor(self);
    return freedIf(self, flags);
}

u8 *__fastcall Window705ac0_Delete(u8 *self, void *, u32 flags) {
    Window705ac0_Dtor(self);
    return freedIf(self, flags);
}

u8 *__fastcall Window705b20_Delete(u8 *self, void *, u32 flags) {
    Window705b20_Dtor(self);
    return freedIf(self, flags);
}

u8 *__fastcall Window705b98_Delete(u8 *self, void *, u32 flags) {
    Window705b98_Dtor(self);
    return freedIf(self, flags);
}

u8 *__fastcall Window705bc8_Delete(u8 *self, void *, u32 flags) {
    Window705bc8_Dtor(self);
    return freedIf(self, flags);
}

u8 *__fastcall Window705c58_Delete(u8 *self, void *, u32 flags) {
    Window705c58_Dtor(self);
    return freedIf(self, flags);
}
}
