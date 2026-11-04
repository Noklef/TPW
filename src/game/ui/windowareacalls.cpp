// The window's area object asked or told: see windowareacalls.h.
#include "windowareacalls.h"

#include <cstdint>

#include "guirect.h"
#include "windowarea.h"

namespace {
const WindowAreaCallsEnv kGameEnv = {&WindowArea_GetRect};
const WindowAreaCallsEnv *g_env = &kGameEnv;

u8 *areaOf(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x98); }
}  // namespace

void WindowAreaCalls_SetEnv(const WindowAreaCallsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Window_AreaRectOrEmpty(u8 *self, void *, u8 *out) {
    u8 *const area = areaOf(self);
    if (area) {
        g_env->getRect(area, nullptr, out);
        return;
    }
    u8 rect[0xc];
    GuiRect_Assign(out, nullptr, GuiRect_CtorWith(rect, nullptr, 0, 0, 0, 0));
    GuiRect_Dtor(rect);
}

void __fastcall Window_CallAreaSlot6(u8 *self) {
    u8 *const area = areaOf(self);
    if (!area) return;
    using Slot = void(__fastcall *)(u8 *);
    (*reinterpret_cast<Slot *>(*reinterpret_cast<u8 **>(area) + 0x18))(area);
}
}
