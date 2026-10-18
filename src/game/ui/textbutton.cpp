#include "textbutton.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/soundplay.h"
#include "textpool.h"
#include "window.h"

extern "C" {

s32 __cdecl TextButton_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x10006) {
        SoundUi_Play(0xc1);
        Window_SendMessage(reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(window)), 0x100001, Fn_65d7bc(window), 0);
        return 0;
    }
    if (message == 0x14) TextPool_Release(reinterpret_cast<u8 *>((uintptr_t)Window_DwordA0(window, nullptr, 1)));
    return Window_CallDefault(window, nullptr, message, a, b);
}

}
