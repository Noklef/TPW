// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x7f0000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b0140: calls Wrap_Init_6b4a60_6b0710.
u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b0710_6b0140();
// 0x6b0180: calls Wrap_Init_6b4a60_6b1de0.
u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b1de0_6b0180();
// 0x6b01c0: calls Wrap_Init_6b4a60_6b2a10.
u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b2a10_6b01c0();
// 0x6b0200: calls Wrap_Init_6b4a60_6b3c00.
u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b3c00_6b0200();
}
