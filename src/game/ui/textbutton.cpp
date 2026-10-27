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

void __cdecl TextButton_FlashStep(u8 *window) {
    s32 value = (s32)Window_DwordA0(window, nullptr, 2);
    if (value > 0) {
        value = (s32)((u32)value + 0x20);
        u32 grey = (u32)value;
        if (value > 0xff) {
            grey = 0xff;
            value = 0;
        }
        Window_Notify40000(window, nullptr, grey, grey, grey, 0xff);
        Window_SetDwordA0(window, nullptr, 2, (u32)value);
    } else if (value < 0) {
        value = (s32)((u32)value - 0x20);
        if ((s32)((u32)value + 0xff) < 0) value = 0;
        Window_Notify40000(window, nullptr, 0, 0xaf, 0xbe, 0xff);
        Window_SetDwordA0(window, nullptr, 2, (u32)value);
    }
}

s32 __cdecl TextButton_FlashHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x11)
        Window_Notify40000(window, nullptr, 0, 0xaf, 0xbe, 0xff);
    else if (message == 0x1e)
        TextButton_FlashStep(window);
    else if (message == 0x10001)
        Window_SetDwordA0(window, nullptr, 2, 1);
    else if (message == 0x10002)
        Window_SetDwordA0(window, nullptr, 2, 0xffffffff);
    return TextButton_Handler(window, message, a, b);
}

}
