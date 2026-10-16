// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x650000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x519450: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da718 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da718_519450(u8 *o);
// 0x519490: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da71a of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da71a_519490(u8 *o);
// 0x5194d0: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da71c of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da71c_5194d0(u8 *o);
// 0x519510: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da720 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da720_519510(u8 *o);
// 0x519550: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da722 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da722_519550(u8 *o);
// 0x519590: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da724 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da724_519590(u8 *o);
// 0x5195d0: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da726 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da726_5195d0(u8 *o);
// 0x519610: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da736 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da736_519610(u8 *o);
}
