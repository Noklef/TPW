// The thing panels' sign slot: see thingsign.h.
#include "thingsign.h"

#include <cstdint>

#include "../world/object.h"
#include "guirect.h"
#include "signrect.h"
#include "window.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const ThingSignEnv kGameEnv = {&Object_GetDef, &Sign_PlaceOverRect};
const ThingSignEnv *g_env = &kGameEnv;

void show(u8 *self, s32 depth) {
    const u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    if (!at<u8 *>(self, 0x18) || !thing) return;
    const u8 *const def = g_env->def(thing);
    u8 rect[12];
    GuiRect_Ctor(rect);
    u8 *const window = at<u8 *>(self, 0x18);
    Window_GetSavedRect(window, nullptr, rect);
    g_env->sign(rect, at<u32>(def, 0x490), at<s16>(window, 0xe0) + depth);
    GuiRect_Dtor(rect);
}

}  // namespace

void ThingSign_SetEnv(const ThingSignEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall ThingSign_Show4997f0(u8 *self) { show(self, 10); }
void __fastcall ThingSign_Show4ad7f0(u8 *self) { show(self, 10); }
void __fastcall ThingSign_Show4afb70(u8 *self) { show(self, 10); }
void __fastcall ThingSign_Show4b0fa0(u8 *self) { show(self, 10); }
void __fastcall ThingSign_Show4b64a0(u8 *self) { show(self, 10); }
void __fastcall ThingSign_Show497060(u8 *self) { show(self, 1); }
