// A ride script's file loaded (RSSE_LoadScriptFile, 0x5587f0): the script's record (0xf4 bytes of global memory) made from the file's
// header, counts and code, and linked at the head of the script list (0x8791b0, +0 next, +4 previous; the count 0x8791ac).
//
// The record: +0 next, +4 previous, +8 the id (0x8791a8, raised), +0x18 the code (+0x50 dwords, read), +0x1c +0x8c dwords (zeroed),
// +0x20 +0x54 dwords (not zeroed), +0x24 +0x58 records of 8 bytes, +0x28 +0x64 of 0x10, +0x2c +0x7c of 0x20 (zeroed), +0x30 the ride's
// keyed parts' dwords (+0x4c of them, zeroed), +0x34 +0x90 bytes (read), +0x38 the file's folder (a string), +0x40 +0x54 - 1, +0x70 1,
// +0x74 -1, the words +0x88 / +0x8a / +0xe4 1000, +0x94 a dword of the file, +0xa8 0xffff, +0xac the thing's handle, +0xc0 (word) 0x32,
// +0xc8 the thing's ride, +0xd0 0, +0xd4 the sound occluder's slot (or 0), +0xd8 -1, +0xe0 0, +0xe6 (word) -1; the rest 0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5587f0 (cdecl (name, handle, quiet); the symbols' name, from its debug strings): 0 without the RSSE system ([0x8791a0] 0) or a stream
// (VFS_Open). The stream's reads go through its slot 8 (Object_CallSlot8Product (buffer, size, count)); it is closed at the end
// (Object_FinishAndDelete) and the record's id returned (0 when the magic is not "RSSE"). The checks of the version against [0x879190] and
// of the allocations only print (left out). After the magic and the version, the record (Mem_GlobalAlloc, zeroed; no check) goes to
// the list's head and the file gives in order: +0x8c, +0x54, +0x94, +0x58, +0x64, +0x7c (each count with its array: Mem_GlobalAlloc of
// count times the size, 0 for a size of 0), four unused dwords, +0x50 and the code, +0x90 and its bytes. Then the fixed fields, the
// file's folder (the name copied to a 0x100-byte buffer, cut after its last backslash scanning down to index 1: a name without one keeps
// its first character, sic; then a copy in global memory) and the thing's handle (its low word). With a handle, +0xc8 the ride of the thing
// (the dword +0x20 of its slot in 0x7cfb90); with a ride, +0x4c counts the ride instance's parts with keys 1, 2, ... (Model_FindPartByKey4,
// mask 0x80) and +0x30 is a zeroed array of them. Unless quiet: the thing's definition (ObjectDefs_Find of its +0xe) has an effect (+0x60)
// and the ride its box (Ride_Bounds): the effect is spawned (Particle_Spawn) at the box's centre (x and z halves of the sums) and the height
// of the instance's root (+8 -> +4 -> +0x78 -> +0x44), each times 1024; with the box again its sizes (x * 512, y * 256, z * 512) go to the
// system (ParticleSystem_SetVectorB), and +0xd4 = Sound_AddOccluder (the centre's x, the height, the centre's z, half the x and z sizes'
// sum, the y size), all as float bits.
u32 __cdecl RSSE_LoadScriptFile(const char *name, u32 handle, u32 quiet);
}
