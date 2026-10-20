// The wavelet-compressed (WCT) image class (vtable 0x7011d8): reimplementation of its constructor
// Obj_CtorVirtual564ba0 (0x564ba0), deleting destructor 0x565000, its load 0x564c00, read 0x564d00 and decoding 0x564e20 / 0x564ef0.
//
// +0 its virtual base table (0x7011d8), +0x20 and +0x24 tracked global blocks, the part at +0x2c (vtable 0x701178) and the virtual base
// (vtable 0x7011c8, whose slots are the thunks 0x564ff0 (this destructor), 0x565080, 0x565090 and 0x5650a0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x565000 (thiscall on the part at +0x2c, reached through the virtual base thunk 0x564ff0; flags, pops 4; name unconfirmed): the virtual
// base's vtable 0x7011c8 and the dword before it (its offset - 0x2c) set back, the blocks at +0x20 and +0x24 (when set) freed
// (TrackedGlobal_Free) and cleared, the seven dwords +4 to +0x1c cleared, the part's vtable 0x701178; the image freed (GameHeap_Free) when bit 0
// of the flags is set. Returns the image.
u8 *__fastcall WctImage_DeletingDtor(u8 *part, void *, u32 flags);

// 0x564ba0 (thiscall; mostDerived, pops 4): the same for vtable 0x7011d8 (part at +0x2c: 0x701178), the
// base's vtable 0x7011c8, the dword before it = its offset - 0x2c, and +4 to +0x24 zeroed (+4-+0x1c,
// then +0x24 and +0x20). Returns the object.
u8 *__fastcall Obj_CtorVirtual564ba0(u8 *object, void *, u32 mostDerived);

// 0x564d00 (thiscall; stream, pops 4; name unconfirmed): the 0x2c-byte header read into 0x879a38 (the stream's slot 2 as
// Object_CallSlot8Product: size 0x2c, count 1); then +6 = 0x20, the u16 sizes +8 / +0xa from the header's +4 / +6, and from its floats
// (__ftol, x87) +0xc = 2 * +0x14 - 2, +0x10 = 4 * +0x18 - 4, +0xe = 4 * +0x1c - 4, +0x12 = +0x20 - 1 (u16 each), +0x14 / +0x18 the
// block sizes at +0x24 / +0x28. Each block is a tracked one (TrackedGlobal_Alloc; +0x24 of the first size, then +0x20 of the second)
// read in turn (count 1 of the size); a failed first returns 0, a failed second frees the first, clears +0x24 and returns 0. Else 1.
u32 __fastcall WctImage_Read(u8 *image, void *, u8 *stream);

// 0x564e20 (thiscall on the part at +0x2c, reached through the virtual base thunk 0x565090; pixels, pops 4): the wavelet codec at
// [0x879a64] decodes (its slot 2, WaveletCodec_Decode) with the kind byte +4, the u16 sizes +8 / +0xa, the format's pixel size (0x7011a0
// indexed by [0x8bd554]), the blocks +0x24 and +0x20, the pointer at pixels and the quantiser steps (floats) 1 + q / 2 of the u16 +0xc,
// 1 + q / 4 of +0x10 and of +0xe, and q + 1 of +0x12. Returns 1.
u32 __fastcall Texture_DecodeWct(const u8 *part, void *, u8 *const *pixels);

// 0x564ef0 (thiscall on the part, through the thunk 0x565080; surface, flip, pops 8; name unconfirmed): 0 for a kind byte (+4) up to
// 0x12. Else with flip the surface (pixels +0, width +4, height +8, pitch +0xc) starts at its last row with the pitch negated (both kept),
// then the codec's slot 1 (WaveletCodec_DecodeScaled) gets the kind, the sizes +8 / +0xa, the surface's width, height and pitch, the
// pixel size, the blocks +0x24 and +0x20, its pixels and the four steps as Texture_DecodeWct's. Returns 1.
u32 __fastcall Texture_DecodeWctInto(const u8 *part, void *, u32 *surface, u32 flip);

// 0x564c00 (thiscall; name, pops 4; name unconfirmed): the file opened (VFS_Open; 0 when not), its 0x1c-byte header read into +4
// (Object_CallSlot8Product: size 0x1c, count 1). A kind byte (+4, read after) other than 0x12 / 0x13 closes it (Object_FinishAndDelete) and
// returns 0. Then the block +0x24 of the size +0x14 (TrackedGlobal_Alloc; closed and 0 when not got) is read (count +0x14 of size 1); with
// +6 = 0x20 the block +0x20 of the size +0x18 likewise, a failed one closing, freeing +0x24 (TrackedGlobal_Free), clearing it and returning
// 0. Then +7 &= 3, the file closed and 1.
u32 __fastcall WctImage_Load(u8 *image, void *, const char *name);
}
