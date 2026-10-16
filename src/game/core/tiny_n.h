// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x650000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x59c240: the dword at +0x24 of the object definition with the id (0x266 when there is none).
u32 __stdcall DefField24_59c240(u32 v0);
// 0x59c2b0: the dword at +0x28 of the object definition with the id (0x0 when there is none).
u32 __stdcall DefField28_59c2b0(u32 v0);
// 0x59c490: the dword at +0x2c of the object definition with the id (0x5 when there is none).
u32 __stdcall DefField2c_59c490(u32 v0);
// 0x59c500: the dword at +0x8 of the object definition with the id (0xa when there is none).
u32 __stdcall DefField8_59c500(u32 v0);
}
