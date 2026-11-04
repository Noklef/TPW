// The dial-up connection let go (the import [0x6fd5b8] on the Dialup object 0x7c24c0). Reimplementation of 0x4b9f30 and 0x4bae40. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b9f30 (cdecl (handle)): with the state [0x7cc538] 1 or 2 the import on the handle. 1 when the state, read again, is 0, else 0.
u32 __cdecl Dialup_HangUpIfBusy(u32 handle);

// 0x4bae40 (cdecl): with the byte [0x7cc540] set the import on the handle [0x7cc50c] and the byte 0.
void __cdecl Dialup_HangUpIfOpen();
}
