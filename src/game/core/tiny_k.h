// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x650000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x407c70: calls ListDtor6fd708_407c30, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia407c30_407c70(u8 *o, void *, u32 v0);
// 0x41c980: calls ListDtor6fd9e4_41c9a0, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia41c9a0_41c980(u8 *o, void *, u32 v0);
// 0x41eb30: calls ListDtor6fdb08_41eb50, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia41eb50_41eb30(u8 *o, void *, u32 v0);
// 0x421620: calls ListDtor6fdb10_421640, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia421640_421620(u8 *o, void *, u32 v0);
// 0x492040: calls Wrap_Fn_669fee_492060, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia492060_492040(u8 *o, void *, u32 v0);
// 0x4e5bc0: calls Vtbl700620_Set_4e5b90, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia4e5b90_4e5bc0(u8 *o, void *, u32 v0);
// 0x5193b0: calls ListDtor700aa0_5193d0, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5193d0_5193b0(u8 *o, void *, u32 v0);
// 0x5ac530: calls ListDtor7027fc_5ac550, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5ac550_5ac530(u8 *o, void *, u32 v0);
// 0x5b5900: calls ListDtor7028a0_5b58c0, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5b58c0_5b5900(u8 *o, void *, u32 v0);
// 0x5b9a40: calls ListDtor7028b0_5b9a80, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5b9a80_5b9a40(u8 *o, void *, u32 v0);
// 0x5b9a60: calls ListDtor7028b4_5b9ac0, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5b9ac0_5b9a60(u8 *o, void *, u32 v0);
// 0x5bea80: calls ListDtor70294c_5beae0, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5beae0_5bea80(u8 *o, void *, u32 v0);
// 0x5beaa0: calls ListDtor702950_5beb20, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5beb20_5beaa0(u8 *o, void *, u32 v0);
// 0x5beac0: calls ListDtor702954_5beb60, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5beb60_5beac0(u8 *o, void *, u32 v0);
// 0x5d57e0: calls ListDtor7029f8_5d5800, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5d5800_5d57e0(u8 *o, void *, u32 v0);
// 0x5d85d0: calls ListDtor702a88_5d8610, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5d8610_5d85d0(u8 *o, void *, u32 v0);
// 0x5d85f0: calls ListDtor702a8c_5d8650, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5d8650_5d85f0(u8 *o, void *, u32 v0);
// 0x5d9ef0: calls ListDtor702afc_5d9f10, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5d9f10_5d9ef0(u8 *o, void *, u32 v0);
// 0x5da290: calls ListDtor702b28_5da2b0, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5da2b0_5da290(u8 *o, void *, u32 v0);
// 0x5dc680: calls ListDtor702c0c_5dc6c0, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5dc6c0_5dc680(u8 *o, void *, u32 v0);
// 0x5dc6a0: calls ListDtor702c10_5dc700, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5dc700_5dc6a0(u8 *o, void *, u32 v0);
// 0x5e0350: calls ListDtor702d00_5e0370, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5e0370_5e0350(u8 *o, void *, u32 v0);
// 0x5e3cb0: calls ListDtor702d98_5e3cd0, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5e3cd0_5e3cb0(u8 *o, void *, u32 v0);
// 0x5e4080: calls ListDtor702f10_5e40a0, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5e40a0_5e4080(u8 *o, void *, u32 v0);
// 0x5f38e0: calls ListDtor703084_5f3900, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia5f3900_5f38e0(u8 *o, void *, u32 v0);
// 0x6175f0: calls SurfaceSource_Dtor, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia47e390_6175f0(u8 *o, void *, u32 v0);
}
