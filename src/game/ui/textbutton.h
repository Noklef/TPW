// A button window whose object (built by 0x492e40, Class492d80_Ctor, its handler 0x492d80 at +0) keeps a text pool row at the window's
// Window_DwordA0 1. Reimplementation of 0x492d80. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x492d80 (cdecl (window, message, a, b); also called from 0x48b305): on 0x10006 the interface sound 0xc1 (SoundUi_Play) and the
// parent (+0x60) sent 0x100001 (the window's id +0x8c, 0) (Window_SendMessage); 0. On 0x14 the row released (TextPool_Release) before
// the default; any other message straight to Window_CallDefault, its result returned.
s32 __cdecl TextButton_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x48b220 (cdecl (window); name unconfirmed): the flash's step (on the timer message 0x1e): the value Window_DwordA0 2 (signed), when
// positive raised by 0x20 and the grey it gives (white past 0xff, the value then 0) sent with 0xff as 0x40000 (Window_Notify40000);
// when negative lowered by 0x20 (0 once below -0xff) with the colour 0, 0xaf, 0xbe, 0xff sent; the value stored back
// (Window_SetDwordA0 2). Nothing for 0. EAX not kept.
void __cdecl TextButton_FlashStep(u8 *window);

// 0x48b2a0 (cdecl (window, message, a, b); name unconfirmed): a flashing text button's handler: 0x11 the colour 0, 0xaf, 0xbe, 0xff
// (Window_Notify40000); 0x1e TextButton_FlashStep; 0x10001 / 0x10002 the flash value (Window_SetDwordA0 2) 1 / -1. Then
// TextButton_Handler, its answer.
s32 __cdecl TextButton_FlashHandler(u8 *window, u32 message, u32 a, u32 b);

}
