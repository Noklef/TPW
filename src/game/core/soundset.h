// The sound module's settings (read by its update, 0x51bd80, when the changed flag 0x803ac8 is
// set, which it then clears): reimplementation of the cdecl setters 0x51bb60-0x51bc20. Each stores
// its value and sets the changed flag. The options code sets them in pairs, a flag around a value
// (0x51bbe0 1, 0x51bb80 value, 0x51bbe0 0; likewise 0x51bc20 / 0x51bbc0 and 0x51bc00 / 0x51bba0);
// the update scales 0x803ac0 and 0x803abc by the percentage at 0x785914, like volumes.
#pragma once

#include "../../../re/types.h"

extern "C" {

void __cdecl Sound_Set803ab8(u32 value);  // 0x51bb60
void __cdecl Sound_Set803ac0(u32 value);  // 0x51bb80
void __cdecl Sound_Set803abc(u32 value);  // 0x51bba0
void __cdecl Sound_Set803ac4(u32 value);  // 0x51bbc0
void __cdecl Sound_Set803adc(u32 value);  // 0x51bbe0
void __cdecl Sound_Set803ad8(u32 value);  // 0x51bc00
void __cdecl Sound_Set803ae0(u32 value);  // 0x51bc20
}
