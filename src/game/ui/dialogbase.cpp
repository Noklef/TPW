#include "dialogbase.h"

#include <cstdint>

#include "../core/accessors.h"
#include "focus.h"
#include "window.h"

extern "C" {

s32 __cdecl Dialog_BaseHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        Window_PostMessage(reinterpret_cast<u8 *>(static_cast<uintptr_t>(Fn_65d55a(window))), 4, 0, 0);
        Focus_Restore();
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}
}
