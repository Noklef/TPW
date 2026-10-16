// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x650000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x407c30: a linked list destructor: sets the vtable pointer 0x6fd708 and frees every node.
void __fastcall ListDtor6fd708_407c30(u8 *o);
// 0x41c9a0: a linked list destructor: sets the vtable pointer 0x6fd9e4 and frees every node.
void __fastcall ListDtor6fd9e4_41c9a0(u8 *o);
// 0x41eb50: a linked list destructor: sets the vtable pointer 0x6fdb08 and frees every node.
void __fastcall ListDtor6fdb08_41eb50(u8 *o);
// 0x421640: a linked list destructor: sets the vtable pointer 0x6fdb10 and frees every node.
void __fastcall ListDtor6fdb10_421640(u8 *o);
// 0x5193d0: a linked list destructor: sets the vtable pointer 0x700aa0 and frees every node.
void __fastcall ListDtor700aa0_5193d0(u8 *o);
// 0x5ac550: a linked list destructor: sets the vtable pointer 0x7027fc and frees every node.
void __fastcall ListDtor7027fc_5ac550(u8 *o);
// 0x5ac590: a linked list destructor: sets the vtable pointer 0x7027fc and frees every node.
void __fastcall ListDtor7027fc_5ac590(u8 *o);
// 0x5b58c0: a linked list destructor: sets the vtable pointer 0x7028a0 and frees every node.
void __fastcall ListDtor7028a0_5b58c0(u8 *o);
// 0x5b9a80: a linked list destructor: sets the vtable pointer 0x7028b0 and frees every node.
void __fastcall ListDtor7028b0_5b9a80(u8 *o);
// 0x5b9ac0: a linked list destructor: sets the vtable pointer 0x7028b4 and frees every node.
void __fastcall ListDtor7028b4_5b9ac0(u8 *o);
// 0x5beae0: a linked list destructor: sets the vtable pointer 0x70294c and frees every node.
void __fastcall ListDtor70294c_5beae0(u8 *o);
// 0x5beb20: a linked list destructor: sets the vtable pointer 0x702950 and frees every node.
void __fastcall ListDtor702950_5beb20(u8 *o);
// 0x5beb60: a linked list destructor: sets the vtable pointer 0x702954 and frees every node.
void __fastcall ListDtor702954_5beb60(u8 *o);
// 0x5d5800: a linked list destructor: sets the vtable pointer 0x7029f8 and frees every node.
void __fastcall ListDtor7029f8_5d5800(u8 *o);
// 0x5d8610: a linked list destructor: sets the vtable pointer 0x702a88 and frees every node.
void __fastcall ListDtor702a88_5d8610(u8 *o);
// 0x5d8650: a linked list destructor: sets the vtable pointer 0x702a8c and frees every node.
void __fastcall ListDtor702a8c_5d8650(u8 *o);
// 0x5d9f10: a linked list destructor: sets the vtable pointer 0x702afc and frees every node.
void __fastcall ListDtor702afc_5d9f10(u8 *o);
// 0x5da2b0: a linked list destructor: sets the vtable pointer 0x702b28 and frees every node.
void __fastcall ListDtor702b28_5da2b0(u8 *o);
// 0x5dc6c0: a linked list destructor: sets the vtable pointer 0x702c0c and frees every node.
void __fastcall ListDtor702c0c_5dc6c0(u8 *o);
// 0x5dc700: a linked list destructor: sets the vtable pointer 0x702c10 and frees every node.
void __fastcall ListDtor702c10_5dc700(u8 *o);
// 0x5e0370: a linked list destructor: sets the vtable pointer 0x702d00 and frees every node.
void __fastcall ListDtor702d00_5e0370(u8 *o);
// 0x5e3cd0: a linked list destructor: sets the vtable pointer 0x702d98 and frees every node.
void __fastcall ListDtor702d98_5e3cd0(u8 *o);
// 0x5e40a0: a linked list destructor: sets the vtable pointer 0x702f10 and frees every node.
void __fastcall ListDtor702f10_5e40a0(u8 *o);
// 0x5f3900: a linked list destructor: sets the vtable pointer 0x703084 and frees every node.
void __fastcall ListDtor703084_5f3900(u8 *o);
}
