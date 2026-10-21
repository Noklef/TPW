#include "dialog7cb2fc.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../render/textparts.h"
#include "window.h"
#include "window705a50.h"
#include "window705b20.h"

namespace {

u8 *dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cb2fc); }

// the row's label 0x1d4c9 dressed (Dialog7cb2fc_DressRow); returns the row
u8 *dressRow(u32 id) {
    u8 *const row = Window_FindChild(dialog(), nullptr, id);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(Window_FindChild(row, nullptr, 0x1d4c9), nullptr, memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr);
    Window_Notify40000(Window_FindChild(row, nullptr, 0x1d4c9), nullptr, 0, 0, 0, 0xff);
    u8 *const font = Language_Font(*reinterpret_cast<u32 *>((uintptr_t)0x007537b4));
    Window_Set4c(Window_FindChild(row, nullptr, 0x1d4c9), nullptr, font);
    Window_NotifyImpl100(Window_FindChild(row, nullptr, 0x1d4c9), nullptr, 0);
    return row;
}

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

void __cdecl Dialog7cb2fc_DressRow(u32 row) { dressRow(row); }

void __cdecl Dialog7cb2fc_DressSlider(u32 id, u32 value) {
    u8 *const row = dressRow(id);
    Window705b20_SetRange(row, nullptr, 0, 100);
    vcall<void>(row, 0x1c, value);
}
}
