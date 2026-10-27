// The scalar deleting destructors that call the destructor of one of the classes of core/vdtor, core/vobj2 and core/buffers and free the object when asked, and the
// constructor and release of the class with the vtable pointer 0x702a90: reimplementation of 0x5b55e0, 0x5b58a0, 0x5bced0, 0x5bd310, 0x5bd720, 0x5e4000, 0x4f1260,
// 0x5ccd80, 0x5d8b00, 0x5d8b40 and 0x5d8b20.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5b55e0 / 0x5b58a0 / 0x5bced0 / 0x5bd310 / 0x5bd720 / 0x5e4000 / 0x4f1260 / 0x5ccd80 (thiscall; flags, pops 4): Class702880_Dtor / Class702890_Dtor /
// Class7028d0_Dtor / Class7028f8_Dtor / Class702920_Dtor / Class702da0_Dtor / Class7099a4_Dtor / Counted2_Clear on the object; when bit 0 of the flags is set the object is
// freed too. Returns the object.
u8 *__fastcall Class702880_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702890_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7028d0_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7028f8_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702920_ScalarDtor(u8 *object, void *, u32 flags);
// 0x5b5780 (thiscall; flags, pops 4): the same with the destructor 0x5b57a0 (core/vdtor).
u8 *__fastcall Class702828_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702da0_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7099a4_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Counted2_ScalarDtor(u8 *object, void *, u32 flags);

// 0x5bdbf0 / 0x5be300 / 0x5be730 / 0x5cf7c0 (thiscall; flags, pops 4): the same over Class702938_Dtor / Class702940_Dtor / Class702948_Dtor / Class709914_Dtor.
u8 *__fastcall Class702938_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702940_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702948_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class709914_ScalarDtor(u8 *object, void *, u32 flags);

// 0x5e3fa0 / 0x5e3fd0 (thiscall; flags, pops 4): the same over the core/vdtor destructors of the 0x702e70 and 0x702e20 classes.
u8 *__fastcall Class702e70_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702e20_ScalarDtor(u8 *object, void *, u32 flags);

// 0x5da270 / 0x5e0330 / 0x5e3c90 (thiscall; flags, pops 4): the same over Class702b08_Dtor / Class702cb0_Dtor / Class702d78_Dtor (core/vdtor).
u8 *__fastcall Class702b08_ScalarDtor(u8 *object, void *, u32 flags);

// 0x5bde10 / 0x5be510 (thiscall; flags, pops 4): the same over the core/vdtor destructors of the 0x70293c and 0x702944 classes.
u8 *__fastcall Class70293c_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702944_ScalarDtor(u8 *object, void *, u32 flags);

// 0x617a60 / 0x617b60 / 0x6180a0 (thiscall; flags, pops 4): the same over Class703780_DtorA / B / C (core/vdtor).
u8 *__fastcall Class703780_ScalarDtorA(u8 *object, void *, u32 flags);
u8 *__fastcall Class703780_ScalarDtorB(u8 *object, void *, u32 flags);
u8 *__fastcall Class703780_ScalarDtorC(u8 *object, void *, u32 flags);
u8 *__fastcall Class702cb0_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702d78_ScalarDtor(u8 *object, void *, u32 flags);

// 0x5d8b00 (thiscall): Class7029d0_Ctor on the object, then the vtable pointer 0x702a90 and +0x20 cleared. Returns the object.
// 0x5d8b40 (thiscall): the vtable pointer 0x702a90 is set and Class7029d0_Release run (returns what it returns).
// 0x5d8b20 (thiscall; flags, pops 4): the release, and the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class702a90_Ctor(u8 *object);
u32 __fastcall Class702a90_Release(u8 *object);
u8 *__fastcall Class702a90_ScalarDtor(u8 *object, void *, u32 flags);

// The same, then the object is freed when bit 0 of flags is set. Returns the object.
u8 *__fastcall Class702ec0_ScalarDtor(u8 *object, void *, u32 flags);

u8 *__fastcall Class702dd0_ScalarDtor(u8 *object, void *, u32 flags);

// 0x569c80 (thiscall; flags, pops 4): Vtbl701260_Set_565230; when bit 0 of the flags is set the block 0x2c bytes before the object is freed.
// Returns that address (the object - 0x2c).
u8 *__fastcall Deleter_Minus2c_569c80(u8 *object, void *, u32 flags);

// 0x5f4400 (thiscall; flags, pops 4): sets the vtable pointer 0x703088, takes the object out of the list at [0xfa6e38] (head +4, tail +8, count +0xc; nodes {value, next, previous}:
// the first node holding the object is unlinked and freed), removes the pointer at +0x28 from the array at 0x879ce0 (PtrArray_RemoveAndFree, which frees it) and frees the object
// when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class703088_ScalarDtor(u8 *object, void *, u32 flags);

// 0x42a090 (thiscall; flags, pops 4): the vtable pointer at +0 = 0x6fdce8; when bit 0 of the flags is set the block 0x60 bytes before the
// object is freed. Returns that address (the object - 0x60), freed or not.
u8 *__fastcall Deleter_Minus60_42a090(u8 *object, void *, u32 flags);

// 0x56a660 (thiscall; flags, pops 4): Vtbl701260_Set_565230; when bit 0 of the flags is set the block 8 bytes before the object is freed.
// Returns that address (the object - 8).
u8 *__fastcall Deleter_Minus8_56a660(u8 *object, void *, u32 flags);

// 0x458970 (thiscall; flags, pops 4; name unconfirmed): the vtable pointer at +0 = 0x6fdce8; when bit 0 of the flags is set the block 8
// bytes before the object is freed. Returns that address (the object - 8), freed or not.
u8 *__fastcall Deleter_Minus8_458970(u8 *object, void *, u32 flags);

// 0x579020 / 0x5790c0 / 0x579150 (thiscall on the object 8 bytes above the base; flags, pops 4): the base's (the pointer - 8) virtual base
// offset (dword +4 of its vtable) names where the vtable 0x7018d0 / 0x701920 / 0x7018f8 is stored; the dword at (the offset - 4, read
// again from the base's new vtable) = the offset - 8; the dword at +8 = 0x701940; when bit 0 of the flags is set the base is freed.
// Returns the base.
u8 *__fastcall VBase_Deleter_7018d0(u8 *object, void *, u32 flags);

u8 *__fastcall VBase_Deleter_701920(u8 *object, void *, u32 flags);

u8 *__fastcall VBase_Deleter_7018f8(u8 *object, void *, u32 flags);

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

// 0x407c70: calls ListDtor6fd708_407c30, then frees the object when bit 0 of the flags is set.
u8 * __fastcall ScalarDeleteVia407c30_407c70(u8 *o, void *, u32 v0);

// 0x409cf0 (name unconfirmed): ListDtor6fd708_407c30 written out (the vtable pointer 0x6fd708, +8 and +4 cleared, the chain from the old +4
// (next at +4) freed, +0xc cleared), then frees the object when bit 0 of the flags is set. Returns the object.
u8 *__fastcall ScalarDeleteList6fd708_409cf0(u8 *o, void *, u32 flags);

// 0x421800 (name unconfirmed): the same with ListDtor6fdb10_421640 (vtable 0x6fdb10).
u8 *__fastcall ScalarDeleteList6fdb10_421800(u8 *o, void *, u32 flags);

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

// 0x41f6a0: a linked list scalar deleting destructor: sets the vtable pointer 0x6fd9e4, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete6fd9e4_41f6a0(u8 *o, void *, u32 v0);

// 0x51aeb0: a linked list scalar deleting destructor: sets the vtable pointer 0x700a9c, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete700a9c_51aeb0(u8 *o, void *, u32 v0);

// 0x51af00: the same with the vtable pointer 0x700aa0.
u8 * __fastcall ListScalarDelete700aa0_51af00(u8 *o, void *, u32 v0);

// 0x5ba7c0: a linked list scalar deleting destructor: sets the vtable pointer 0x7028a0, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete7028a0_5ba7c0(u8 *o, void *, u32 v0);

// 0x5ba810: a linked list scalar deleting destructor: sets the vtable pointer 0x7028b4, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete7028b4_5ba810(u8 *o, void *, u32 v0);

// 0x5beba0: a linked list scalar deleting destructor: sets the vtable pointer 0x70294c, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete70294c_5beba0(u8 *o, void *, u32 v0);

// 0x5bebf0: a linked list scalar deleting destructor: sets the vtable pointer 0x702950, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702950_5bebf0(u8 *o, void *, u32 v0);

// 0x5d9750: a linked list scalar deleting destructor: sets the vtable pointer 0x702a88, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702a88_5d9750(u8 *o, void *, u32 v0);

// 0x5da4d0: a linked list scalar deleting destructor: sets the vtable pointer 0x702afc, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702afc_5da4d0(u8 *o, void *, u32 v0);

// 0x5ddaf0: a linked list scalar deleting destructor: sets the vtable pointer 0x702c0c, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702c0c_5ddaf0(u8 *o, void *, u32 v0);

// 0x5d97a0 (slot 0 of the vtable 0x702a8c): a linked list scalar deleting destructor: sets the vtable pointer 0x702a8c, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702a8c_5d97a0(u8 *o, void *, u32 v0);

// 0x5ad0d0 (no direct callers; name invented): a linked list scalar deleting destructor: sets the vtable pointer 0x7027fc, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete7027fc_5ad0d0(u8 *o, void *, u32 v0);

// 0x5ba860 (no direct callers; name invented): a linked list scalar deleting destructor: sets the vtable pointer 0x7028b0, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete7028b0_5ba860(u8 *o, void *, u32 v0);

// 0x5bec40 (no direct callers; name invented): a linked list scalar deleting destructor: sets the vtable pointer 0x702954, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702954_5bec40(u8 *o, void *, u32 v0);

// 0x5da520 (no direct callers; name invented): a linked list scalar deleting destructor: sets the vtable pointer 0x702b28, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702b28_5da520(u8 *o, void *, u32 v0);

// 0x5ddb40 (no direct callers; name invented): a linked list scalar deleting destructor: sets the vtable pointer 0x702c10, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702c10_5ddb40(u8 *o, void *, u32 v0);

// 0x41f6f0 (no direct callers; name invented): a linked list scalar deleting destructor: sets the vtable pointer 0x6fdb08, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete6fdb08_41f6f0(u8 *o, void *, u32 v0);

// 0x5e2b70 (no direct callers; name invented): a linked list scalar deleting destructor: sets the vtable pointer 0x702d00, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702d00_5e2b70(u8 *o, void *, u32 v0);

// 0x5e4370 / 0x5d5ef0 (slot 0 of the vtables 0x702d98 / 0x7029f8; thiscall, flags, pops 4): the same with their own vtable pointers.
u8 *__fastcall ListScalarDelete702d98_5e4370(u8 *o, void *, u32 flags);
u8 *__fastcall ListScalarDelete7029f8_5d5ef0(u8 *o, void *, u32 flags);

// 0x5e4320: a linked list scalar deleting destructor: sets the vtable pointer 0x702f10, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702f10_5e4320(u8 *o, void *, u32 v0);

// 0x5f46e0: a linked list scalar deleting destructor: sets the vtable pointer 0x703084, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete703084_5f46e0(u8 *o, void *, u32 v0);

// 0x618110 (thiscall; flags, pops 4): Class7037b8_Dtor, then the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class7037b8_ScalarDtor(u8 *object, void *, u32 flags);
}  // extern "C"
