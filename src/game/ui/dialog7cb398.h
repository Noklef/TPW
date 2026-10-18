// The window at 0x7cb398 (made by 0x4a52a0 with the handler 0x4a4830; the park statistics graph's Gauge is at 0x7cb318 beside it).
// Reimplementation of 0x4a4ab0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a4ab0 (no arguments): the window's child 0x4a68d1 (Window_FindChild; nothing without it), then the three commonest values of the
// park's plain analyser things (Things_TopThreeValues; the original first looks up the game record of [0x7cf83c] for the call's ECX,
// which the callee ignores), shown by its children 0x4a68d4, 0x4a68d3 and 0x4a68d2 in turn: a value 1..0x16 goes to the child's slot 7
// as value - 1, any other value (0x17 for none) hides the child (Window_SetShown 0).
void __cdecl Dialog7cb398_ShowTopThree();

}
