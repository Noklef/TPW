#include "windowarea.h"

#include <cstdint>

#include "guirect.h"

extern "C" {

u8 *__fastcall WindowArea_GetRect(u8 *area, void *, u8 *out) { return GuiRect_Assign(out, nullptr, area + 0x20); }

u8 *__fastcall WindowArea_GetRect2c(u8 *area, void *, u8 *out) { return GuiRect_Assign(out, nullptr, area + 0x2c); }

void __fastcall Window_GetAreaRect(u8 *window, void *, u8 *out) {
    u8 *const area = *reinterpret_cast<u8 **>(window + 0x9c);
    if (area) {
        WindowArea_GetRect(area, nullptr, out);
        return;
    }
    u8 zero[0x18];
    GuiRect_CtorWith(zero, nullptr, 0, 0, 0, 0);
    GuiRect_Assign(out, nullptr, zero);
    GuiRect_Dtor(zero);
}
}
