#include "dialog7cc61c.h"

#include <cstdint>

#include "dialogbase.h"
#include "focus.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Owner() { return *reinterpret_cast<u8 **>(global<u8 *>(0x00f82884) + 4); }

}  // namespace

extern "C" s32 __cdecl Dialog7cc61c_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        global<u8 *>(0x007cc61c) = nullptr;
        Window_SendMessage(Owner(), 6, 1, 0);
        Focus_Restore();
    } else if (message == 0x15) {
        Window_SendMessage(Owner(), 6, 0, 0);
    } else if (message == 0x100 && a == 0xffffffff) {
        Window_SendMessage(global<u8 *>(0x007cc61c), 4, 0, 0);
        return 0;
    }
    return Dialog_BaseHandler(window, message, a, b);
}
