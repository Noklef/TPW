#include "dialog7cb25c.h"

#include <cstdint>

#include "../core/language.h"
#include "../core/vcall.h"
#include "window.h"

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
}  // extern "C"
