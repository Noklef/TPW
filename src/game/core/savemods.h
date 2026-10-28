// The save game writers of four modules, each a run of Stream_Write calls from the module's globals: reimplementation of Camera_Save 0x42cdc0 (module KAME),
// Particles_Save 0x51f680 (PART), FlyingRides_Save 0x55de70 (FLYR) and RSSE_Save 0x559350 (RSSE). The results of the writes are not looked at (except the camera's).
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x42cdc0 (stream): the camera zoom, yaw and flags (4 bytes each at 0x7909ec, 0x790a38, 0x790ab0), the required and saved POI positions (12 bytes at 0x7908f0, 0x790ad0) and
// the saved yaw (4 bytes at 0x790a9c). Returns 0 as soon as a write reports an error, otherwise 1.
u32 __cdecl Camera_Save(Stream *stream);

// 0x51f680 (stream, full): 'LCTP', the flag at 0x816d40, and when that is set the blocks 0x9e68 bytes from 0x80ced0 and 0x8b60 from 0x804370 (each after its size) and
// the systems (the dword at 0x80cedc times 0x34 bytes from the pointer at 0x80cef8; none when full is 0), after their size. Returns 0.
u32 __cdecl Particles_Save(Stream *stream, s32 full);

// 0x55de70 (stream): 'FLY_', the flag at 0x8791ec, and when that is set 400 bytes from 0x8791f8, the dwords at 0x7660b8 to 0x7660c4, five 'PAD_' words and the 0x122c bytes
// each non-null pointer of the table 0x8791fc.. (below 0x879388) points to, after their size. Returns 0.
u32 __cdecl FlyingRides_Save(Stream *stream);

// 0x559350 (stream): 'RSSE', the 0x14 bytes from 0x8791a0 (the initialised flag, the list head at 0x8791b0), and when the flag is set five 'PAD_' words, the count of script
// contexts and 0xf4, then for each context of the list: the context itself and its arrays (code, stack, variables, the tables of the context at +0x30 / +0x34 / +0x4c / +0x90,
// limbo, riders, walkers, its name) each after its size, then 'OBJ ' and its objects (0x1c bytes each) after their count and size. Returns 0.
u32 __cdecl RSSE_Save(Stream *stream);

// 0x51f7a0 (stream): the module Particles_Save wrote, when the particle flag at 0x816d40 is set: 'LCTP' and a non-zero flag, else nothing is read (returns 0); the system
// pool at 0x80cef8 is freed and the two blocks read in over 0x80ced0 and 0x804370 (their sizes must be 0x9e68 and 0x8b60: otherwise returns 1); the pool is allocated
// (the count at 0x80cedc times 0x34 bytes) and read in after its size, or, for size 0, cleared and linked into a free list (words at +0x30 / +0x32 of each 0x34-byte system:
// next / previous), the systems of the live chain (head word at 0x816d1c, next word at +0xd0 of the 0x140-byte systems from 0x80cefc) reset. The dwords at 0x80ceec and
// 0x80cef4 keep their values through the read. Returns 0.
u32 __cdecl Particles_Load(Stream *stream);

// 0x42cec0 (stream): the mirror of Camera_Save, read into the same globals in the same order; 0 as soon as a read reports an error. Then
// gCameraFlags (0x790ab0) cleared, 0x7909e0 = the yaw, and when the byte 0x78d912 is set the yaw snapped to a quarter turn (Camera_SnapYaw,
// inlined in the original). Returns 1.
u32 __cdecl Camera_Load(Stream *stream);

// 0x5597a0 (stream): the mirror of RSSE_Save. A running system's scripts are freed first (RSSE_FreeContext, flags 0) and the system
// set up again (RSSE_Init, inlined in the original). 'RSSE' (else 1 returned), then its size and that many bytes over 0x8791a0 (the list
// head cleared after), five dwords, the count and the record size. Each record (0xf4 bytes of global memory, zeroed, pushed at the
// list's head, the count 0x8791ac up) is read over with the record size (its links kept), then each block after its size: the code
// +0x18 (always, none for 0; +0x50 its dwords), the stack +0x20 (+0x54), the variables +0x1c (+0x8c), +0x34 (the size into +0x90), the
// limbo +0x24 (+0x58, then / 8), the riders +0x28 (+0x64, then / 16), the walkers +0x2c (+0x7c their count, 0x20 bytes each), +0x30 (+0x4c,
// then / 4) and +0x38 (kept as read for size 0). A record with an occluder (+0xd4) and a ride (+0xc8) with a box gets a new occluder as
// RSSE_LoadScriptFile makes it. Then +0xe6 = -1, 'OBJ ' (else 1 returned), the object count and node size, each node (0x1c bytes, pushed
// at the head of +0xb0, 0x8791b4 up) read over (its links kept). A read that falls short leaves its value as it was. Returns 0.
u32 __cdecl RSSE_Load(Stream *stream);

// 0x55e020 (stream): the mirror of FlyingRides_Save. A running module (0x8791ec) is shut first (FlyingRides_Shutdown, inlined in the
// original). 'FLY_' (else 1 returned), then
// the flag; 0: nothing more (returns 0). Else the module set up (FlyingRides_Init, inlined), a size and that many bytes over the table
// 0x8791f8, the dwords 0x7660b8 to 0x7660c4, five dwords and the record size; each entry 1-99 the read left non-null gets a block of that
// size (Mem_GlobalAlloc) read in. A read that falls short leaves its value as it was. Returns 0.
u32 __cdecl FlyingRides_Load(Stream *stream);

}  // extern "C"
