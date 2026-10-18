// The wavelet-compressed (WCT) image class (vtable 0x7011d8): reimplementation of its constructor
// Obj_CtorVirtual564ba0 (0x564ba0), deleting destructor 0x565000 and its decoding 0x564e20 / 0x564ef0.
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
}
