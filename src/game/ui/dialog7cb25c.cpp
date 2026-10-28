#include "dialog7cb25c.h"

#include <cstdint>

#include "../core/language.h"
#include "../core/methods.h"
#include "../core/vcall.h"
#include "../world/money.h"
#include "../world/world.h"
#include "window.h"
#include "window7059f0.h"

namespace {

u8 *dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cb25c); }

}  // namespace

extern "C" {

void __cdecl Dialog7cb25c_SetTexts() {
    if (!dialog()) return;
    u8 *const panel = Window_FindChild(dialog(), nullptr, 0x12c4bd);
    static const u32 kPlain[3][3] = {{0x11, 0xae, 0xdf}, {0x12, 0xad, 0xe0}, {0x15, 0xb1, 0xe3}};
    static const u32 kFlagged[3][3] = {{0x11, 0xb2, 0xe5}, {0x12, 0xb3, 0xe6}, {0x15, 0xb4, 0xe7}};
    const bool flagged = *reinterpret_cast<u32 *>((uintptr_t)0x007cb260) != 0;
    for (const auto &row : flagged ? kFlagged : kPlain) {
        u8 *const child = Window_FindChild(panel, nullptr, row[0]);
        Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_WideText(row[1]));
        *reinterpret_cast<u32 *>(child + 0x90) = row[2];
        *reinterpret_cast<u32 *>(child + 0x94) = row[2];
    }
    u8 *const title = Window_FindChild(dialog(), nullptr, 0x12c4ca);
    vcall<void>(title, 0x24, Language_WideText(flagged ? 0xab : 0xaa));
}

void __cdecl Dialog7cb25c_FillList(u8 *list) {
    Window7059f0_Clear(list);
    for (u32 i = 0; i < 8; i++) {
        u32 offer[8];
        const u32 *const got = Money_LoanOffer(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(
                                                   *reinterpret_cast<u8 **>((uintptr_t)0x007cf83c))), nullptr, offer, i);
        for (u32 k = 0; k < 8; k++) offer[k] = got[k];
        const u32 flag = *reinterpret_cast<const u32 *>((uintptr_t)0x007cb260);
        u32 data;
        u32 left;   // (the months left: the term, less those paid for a loan taken)
        if (flag == 0 && offer[5] == 0 && offer[0] != 0) {
            left = offer[3];
            data = i + 1;
        } else if (flag == 1 && offer[5] != 0) {
            left = offer[3] - offer[6];
            data = ~i;   // (-1 - i: the loans taken counted down from -1)
        } else {
            continue;
        }
        const u8 *const text = Records_DataIfTerminated(*reinterpret_cast<const u8 *const *>((uintptr_t)0x0078bac4), nullptr, offer[7]);
        const u32 texts[6] = {(u32)(uintptr_t)Language_Widen(text), offer[1], left, offer[2], offer[4], offer[4] * left};
        Window7059f0_AddItem(list, nullptr, texts, data, -1, 1);
    }
}

void __cdecl Dialog7cb25c_Refresh() {
    if (dialog() && *reinterpret_cast<const u32 *>((uintptr_t)0x007cb260) == 1)
        Dialog7cb25c_FillList(Window_FindChild(dialog(), nullptr, 0x12c4bd));
}
}  // extern "C"
