#include "msgboxbutton.h"

#include <cstdint>

#include "../core/vcall.h"
#include "window.h"
#include "windowreg.h"

extern "C" {

void __cdecl MsgBox_SetButton(u8 *window, u32 kind) {
    const char *name;
    u32 tag = 0x200 + kind;
    bool tagged = true;
    switch (kind) {
    case 3: name = "mb_locate"; tag = 0x204; break;
    case 4: name = "mb_locate"; tag = 0x203; break;
    case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: name = "mb_buy"; break;
    case 18: name = "b_loan"; break;
    case 19: name = "mb_resrch"; break;
    case 20: name = "b_pkinfo"; break;
    case 21: name = "b_staffcost"; break;
    case 22: return;
    case 23: name = "mb_callmech"; break;
    case 24: name = "b_entry"; tag = 0xec; break;
    case 26: name = "mb_accept"; tagged = false; break;
    case 27: name = "mb_decline"; tagged = false; break;
    default:
        Window_SetObject98(window, nullptr, nullptr);
        vcall<void>(window, 0x14, 0u);
        return;
    }
    Window_SetObject98(window, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(name)));
    vcall<void>(window, 0x14, 1u);
    if (tagged) *reinterpret_cast<u32 *>(window + 0x90) = tag;
}
}  // extern "C"
