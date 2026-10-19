#include "dialog7cb2fc.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "window.h"
#include "window705a50.h"

namespace {

u8 *dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cb2fc); }

}  // namespace

extern "C" {

void __cdecl Dialog7cb2fc_SetToggle(u32 row, u32 label, u8 on, u8 other, u8 select, u32 button) {
    u8 *const item = Window_FindChild(Window_FindChild(dialog(), nullptr, row), nullptr, 0x1d4c9);
    u16 state[32];   // sic: its last unit never set
    Crt().wcsncpy(state, Language_WideText((other ? 0x14e : 0x14c) + (on ? 1 : 0)), 0x1f);
    Language_WideText(label);
    u16 *const text = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
    Crt().wcscat(text, state);
    vcall<void>(item, 0x24, text);
    if (select)
        Window705a50_SetState(Window_FindChild(dialog(), nullptr, button), nullptr, on ? 0 : 1, 1);
}
}
