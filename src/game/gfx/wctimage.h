// The wavelet-compressed (WCT) image class (vtable 0x7011d8): reimplementation of its constructor
// Obj_CtorVirtual564ba0 (0x564ba0) and deleting destructor, 0x565000.
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
}
