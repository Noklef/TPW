// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x650000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x41f6a0: a linked list scalar deleting destructor: sets the vtable pointer 0x6fd9e4, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete6fd9e4_41f6a0(u8 *o, void *, u32 v0);
// 0x51aeb0: a linked list scalar deleting destructor: sets the vtable pointer 0x700a9c, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete700a9c_51aeb0(u8 *o, void *, u32 v0);
// 0x561a00: frees the block the dword at 0x879a20 points to (if any), clears it and returns 1 (0 when there was none).
u32 __cdecl FreeGlobal879a20_561a00();
// 0x56b760: frees the block the dword at 0x879d54 points to (if any), clears it and returns 1 (0 when there was none).
u32 __cdecl FreeGlobal879d54_56b760();
// 0x5740e0: frees the block the dword at 0x87a248 points to (if any), clears it and returns 1 (0 when there was none).
u32 __cdecl FreeGlobal87a248_5740e0();
// 0x580500: frees the block the dword at 0x8bcbb8 points to (if any), clears it and returns 1 (0 when there was none).
u32 __cdecl FreeGlobal8bcbb8_580500();
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
// 0x5e4320: a linked list scalar deleting destructor: sets the vtable pointer 0x702f10, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete702f10_5e4320(u8 *o, void *, u32 v0);
// 0x5f46e0: a linked list scalar deleting destructor: sets the vtable pointer 0x703084, frees every node and, when bit 0 of the flags is set, the list.
u8 * __fastcall ListScalarDelete703084_5f46e0(u8 *o, void *, u32 v0);
}
