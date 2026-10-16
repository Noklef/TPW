// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x650000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x40f270: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7a0_40f270(u8 *o, void *, u32 v0);
// 0x40f2b0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7a8_40f2b0(u8 *o, void *, u32 v0);
// 0x40f2e0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7b0_40f2e0(u8 *o, void *, u32 v0);
// 0x40f360: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7c0_40f360(u8 *o, void *, u32 v0);
// 0x40f3b0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7c8_40f3b0(u8 *o, void *, u32 v0);
// 0x40f400: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7d0_40f400(u8 *o, void *, u32 v0);
// 0x40f450: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7d8_40f450(u8 *o, void *, u32 v0);
// 0x40f4a0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7e0_40f4a0(u8 *o, void *, u32 v0);
// 0x40f4e0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7e8_40f4e0(u8 *o, void *, u32 v0);
// 0x40f520: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7f0_40f520(u8 *o, void *, u32 v0);
// 0x40f560: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd7f8_40f560(u8 *o, void *, u32 v0);
// 0x40f5c0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd800_40f5c0(u8 *o, void *, u32 v0);
// 0x40f610: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd808_40f610(u8 *o, void *, u32 v0);
// 0x40f660: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd810_40f660(u8 *o, void *, u32 v0);
// 0x40f6b0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd818_40f6b0(u8 *o, void *, u32 v0);
// 0x40f720: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd820_40f720(u8 *o, void *, u32 v0);
// 0x40f780: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd828_40f780(u8 *o, void *, u32 v0);
// 0x40f820: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd830_40f820(u8 *o, void *, u32 v0);
// 0x40f890: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd838_40f890(u8 *o, void *, u32 v0);
// 0x40f8e0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd840_40f8e0(u8 *o, void *, u32 v0);
// 0x40f930: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fd848_40f930(u8 *o, void *, u32 v0);
// 0x429b60: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fdce8_429b60(u8 *o, void *, u32 v0);
// 0x47e3a0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fef90_47e3a0(u8 *o, void *, u32 v0);
// 0x47e730: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete6fefb8_47e730(u8 *o, void *, u32 v0);
// 0x4f18b0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete709b44_4f18b0(u8 *o, void *, u32 v0);
// 0x4ff500: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete7007a8_4ff500(u8 *o, void *, u32 v0);
// 0x5061b0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete700868_5061b0(u8 *o, void *, u32 v0);
// 0x564710: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete701178_564710(u8 *o, void *, u32 v0);
// 0x565210: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete701260_565210(u8 *o, void *, u32 v0);
// 0x58b310: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete702198_58b310(u8 *o, void *, u32 v0);
// 0x591980: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete7026e8_591980(u8 *o, void *, u32 v0);
// 0x5d5c30: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete7029fc_5d5c30(u8 *o, void *, u32 v0);
// 0x617930: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete703748_617930(u8 *o, void *, u32 v0);
// 0x617ae0: sets the vtable pointer(s), then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDelete703770_617ae0(u8 *o, void *, u32 v0);
}
