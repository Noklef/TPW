// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x650000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d8a80: removes the first node with a value from the linked list at +0xc (unlinks it, counts it off, frees it).
void __fastcall ListRemovec_5d8a80(u8 *o, void *, u32 v0);
// 0x5da180: removes the first node with a value from the linked list at +0x58 (unlinks it, counts it off, frees it).
void __fastcall ListRemove58_5da180(u8 *o, void *, u32 v0);
// 0x5da450: removes the first node with a value from the linked list at +0x4 (unlinks it, counts it off, frees it).
void __fastcall ListRemove4_5da450(u8 *o, void *, u32 v0);
// 0x5dd4b0: removes the first node with a value from the linked list at +0x6c (unlinks it, counts it off, frees it).
void __fastcall ListRemove6c_5dd4b0(u8 *o, void *, u32 v0);
// 0x5e15c0: removes the first node with a value from the linked list at +0x8 (unlinks it, counts it off, frees it).
void __fastcall ListRemove8_5e15c0(u8 *o, void *, u32 v0);
// 0x5e4250: removes the first node with a value from the linked list at +0x18 (unlinks it, counts it off, frees it).
void __fastcall ListRemove18_5e4250(u8 *o, void *, u32 v0);
}
