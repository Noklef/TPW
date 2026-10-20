// The game's choice of the window to take the focus back (after a dialog or a key: 0x4862a0) among its windows and dialogs: the object at
// [0x7c2534] (its window +0x10 while its +0xc is set), the windows [0x7cb2ac], [0x7cb2b0], [0x7cb2b4], [0x7cc8bc], [0x7cc8c0], [0x7c24c8],
// [0x7c24cc], [0x7ca1c8] and the frame's ([0xf82884] + 4); two windows closed before it; and the overlay [0x7cb2b4] toggled.
// Reimplementation of 0x48c8d0, 0x4862a0, 0x485b40, 0x4a27c0 and 0x4a29d0 (cdecl), 0x481490 and 0x4815d0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x48c8d0: 1 when the object [0x7c2534] is set and so is its +0xc (Source_Field0c), else 0.
u32 __cdecl Global7c2534_HasSource();

// 0x4862a0: the focus (Window_SetFocus(0), its result returned) to the first of: with Global7c2534_HasSource, [0x7c2534]'s window +0x10.
// Without [0x7cb2ac]: [0x7cb2b0] when set and shown (bit 0 of its flags +0x48, Fn_65d57c; then the key map [0x787c14] enabled first,
// KeyMap_Enable), [0x7cc8bc], [0x7cc8c0] (each when set and shown), else the frame's window ([0xf82884] + 4; [0xf82884] itself returned
// when that is 0, 0 without a frame). With [0x7cb2ac]: [0x7c24cc] when set; [0x7cb2b0] when shown (not checked for 0) as above; with
// [0x7cb2ac] shown, [0x7c24c8] when set, else (the key map [0x787c14] enabled) Dialog7ca1c8_Focus with Global7ca1cc_Get, else the key maps
// [0x787a60] and [0x78733c] enabled too and [0x7cb2ac]; else [0x7cb2b4] when set and shown, the key map [0x787c14] enabled first. With
// none, the last flags read (Fn_65d57c's) are returned.
u32 __cdecl Focus_Restore();

// 0x485b40: with the window [0x7c24c8] set, it is sent 5 (0, 0) (Window_SendMessage), forgotten and the focus restored (Focus_Restore);
// then 1, else 0.
u32 __cdecl Window7c24c8_Close();

// 0x4a27c0: with the window [0x7cb2cc] set, it is sent 4 (0, 0) (Window_SendMessage); it is forgotten either way and the focus restored
// (Focus_Restore, its result returned).
u32 __cdecl Window7cb2cc_Close();

// 0x4a29d0 (cdecl): with the window [0x7cb2ac] set, the toggle [0x7cb2e8] flipped (1 - it): when set, the overlay window [0x7cb2b4] (when
// there is one) deleted through its slot 2 (1) and forgotten; when not, nothing (0) with any of the flags 0x16 of the byte 0x790ab0, else
// the overlay made when missing: a child of the GUI root [0xfaa5d8] (Window_CreateChild kind 1, flags 1, id 0xddf22, 0, 0 - 0x7ff, 0x5ff)
// that, when made, gets the handler 0x4a2840 (Window_SetHandler) and the focus (Window_SetFocus(0)). Then [0x7cb2ac] shown or hidden by the
// old toggle's word (Window_SetShown) and the toggle flipped; returns the old toggle while the new one is set, else Focus_Restore's result.
// 0 without [0x7cb2ac].
u32 __cdecl Overlay7cb2b4_Toggle();

// 0x481490 / 0x4815d0 (thiscall on the object at 0x7b51f0, this not used): Overlay7cb2b4_Toggle; 0x4815d0 only while the toggle is set
// (Global7cb2e8_Get; 0 otherwise).
u32 __fastcall Obj7b51f0_ToggleOverlay(void *self);
u32 __fastcall Obj7b51f0_CloseOverlay(void *self);
}
