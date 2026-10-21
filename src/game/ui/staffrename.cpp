#include "staffrename.h"

#include <cstdint>

#include "../core/keymap.h"
#include "../core/vcall.h"
#include "../files/wrapsources.h"
#include "../world/staff.h"
#include "focus.h"
#include "window.h"

s32 __cdecl StaffRename_Handler(u8 *window, u32 message, s32 a, u32 b) {
    if (message > 0x100) {
        if (message == 0x802) {
            Window_PostMessage(window, 0x100, 0xffffffff, 0);   // (the confirmation, as a click on the -1 button)
        } else if (message == 0x804) {
            Window_SendMessage(window, 4, 0, 0);
            return 0;
        }
    } else if (message == 0x100) {
        if (a == -1) {
            Staff_SetName(reinterpret_cast<u8 *>((uintptr_t)Window_DwordA0(window, nullptr, 0)), nullptr,
                          reinterpret_cast<const u16 *>((uintptr_t)0x007cc360));
            vcall<void>(reinterpret_cast<u8 *>((uintptr_t)Window_DwordA0(window, nullptr, 1)), 4);
            Window_SendMessage(window, 4, 0, 0);
            return 0;
        }
        if (a == -2) {
            Window_SendMessage(window, 4, 0, 0);
            return 0;
        }
    } else if (message == 0x14) {
        KeyMaps_EnableAllAndClearFlag();
        TrackedSource_Clear(reinterpret_cast<TrackedSource *>((uintptr_t)Window_DwordA0(window, nullptr, 1)));
        Focus_Restore();
        return Window_CallDefault(window, nullptr, message, (u32)a, b);
    }
    return Window_CallDefault(window, nullptr, message, (u32)a, b);
}
