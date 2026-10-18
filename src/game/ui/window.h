// The GUI library's windows (0x65xxxx): a window's children are a list from +0x5c linked by +0x54, each with its id at +0x8c
// (Fn_65d7bc). Reimplementation of 0x65d7d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65d7d0 (thiscall (id), pops 4; name unconfirmed): the first child of the window whose id is `id`, 0 when none is.
u8 *__fastcall Window_FindChild(u8 *window, void *, u32 id);
}
