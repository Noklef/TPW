// A button window whose object (built by 0x492e40, Class492d80_Ctor, its handler 0x492d80 at +0) keeps a text pool row at the window's
// Window_DwordA0 1. Reimplementation of 0x492d80. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x492d80 (cdecl (window, message, a, b); also called from 0x48b305): on 0x10006 the interface sound 0xc1 (SoundUi_Play) and the
// parent (+0x60) sent 0x100001 (the window's id +0x8c, 0) (Window_SendMessage); 0. On 0x14 the row released (TextPool_Release) before
// the default; any other message straight to Window_CallDefault, its result returned.
s32 __cdecl TextButton_Handler(u8 *window, u32 message, u32 a, u32 b);

}
