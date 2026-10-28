// A ride script killed (RSSE_KillScript, 0x559060) and its record freed (RSSE_FreeContext, 0x558500): the undoing of
// RSSE_LoadScriptFile (scriptfile.h). The script list is 0x8791b0 (+0 next, +4 previous, +8 the id), its count 0x8791ac; the record's
// object list (+0xb0) counts in 0x8791b4.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x558500 (cdecl (record, flags); name invented): nothing for a null record (0 returned). Else, in order: the byte +0xb9 set clears
// Sound_Set803ad0(0); a ride (+0xc8) frees the flying ride +0x9c (FlyingRide_Free); a particle system +0xcc is aged out
// (ParticleSystem_SetAge -2) and cleared; a sound +0xd0 is stopped (Sound_Stop). The +0x30 dwords (+0x4c of them, both read again each
// step) that are set are cleared, with flags & 4 each one's part released first (key index + 1); so are the +0x2c walker slots (0x20
// bytes, +0x7c of them) whose words +0x16 / +0x18 are 4 / 2 (+0x18 cleared; the part's key the s16 +2). Releasing a part: the ride's
// (0x7a4610 + 4 +0xc8) Model_FindPartByKey4 (mask 0x80, the key), when found PartStates_Release (the ride's +8, the part). Then the count
// drops by one, the record is unlinked, its blocks +0x18 .. +0x38 (each one set) go (Mem_GlobalFree), and each node of its object list
// (+0 next, +4 previous, +8 the type, +0xc the handle) has its handle ended (types 1 / 2 ParticleSystem_SetAge -2, 3-10 Sound_Forward6;
// another only prints, left out), is unlinked (the record's head moved on when it is the node), freed and counted off 0x8791b4. Last
// the list's head moves on when it is the record, and the record is freed. Returns the record's next.
RsseContext *__cdecl RSSE_FreeContext(RsseContext *ctx, u32 flags);

// 0x559060 (cdecl (id, flags)): nothing without the system ([0x8791a0] 0; prints, left out) or a record of the id. A thing (the word
// +0xac, its slot in 0x7cfb90) has its definition's (ObjectDefs_Find of the thing's +0xe) kill effect (+0x64) spawned with flags & 2:
// at the ride's box centre (as RSSE_LoadScriptFile's, all zero without a ride (+0xc8) or a box: Particle_Spawn), then with the box again
// its sizes go to the system (ParticleSystem_SetVectorB: x * 512, y * 256, z * 512); then a sound occluder +0xd4 is removed
// (Sound_RemoveOccluder) and cleared. Then the records of the ids +0x14 and +0xc are freed (RSSE_FreeContext; one not in the list a null
// one), the record of the id +0x10 (its parent) gets +0xc = 0 (sic: a parent not in the list faults, the write going to 0xc), and the
// record is freed.
void __cdecl RSSE_KillScript(s32 id, u32 flags);
}
