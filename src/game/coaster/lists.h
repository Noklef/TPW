// Linked lists of 0x10-byte items from a pool, per coaster.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43fcd0: takes the next 0x10-byte item from the pool (cursor at 0x79c604, advanced by
// 0x10) and links it at the list's tail: item +8 next (cleared), +0xc previous; list +4
// count, +8 head, +0xc tail. A list without a slot (+0x10) first gets the next one of the
// current coaster's (0x792218 -> +0x38 -> +0x24 -> +4 = mesh) slots, mesh+0x78 -> +0x6c
// + 0x10 * the counter at 0x7925c8, which is advanced, with its +0xc byte = list flag bit 0;
// mesh+0x3c and (mesh+0x78)+0x5a (words) count it. Returns the item.
u8 *__cdecl CoasterList_Append(u8 *list);

// 0x43f150 (cdecl (keep); name unconfirmed): the current coaster's (0x792218) mesh finished, then (keep 0) a fresh one begun. The coaster
// is +4 the build ({+0x14 the sizes, +0x18 the animator to clone, +0x1c the lists' count (the word +4)}) and +0x38 the animator. With an
// animator: its mesh (+0x24 -> +4) and the mesh's first part (+0x78) take the bounds (max at 0x7925b8, min at 0x7925a8, three floats each;
// the mesh at +0x80 / +0x8c, the part at +0x78 / +0x84), the mesh's dword +0xa8 the bytes of __ftol(max x, max z, min x, min z * 0.1),
// high to low, and its flags +0x30 0xc00; each list (0x1c bytes from [0x7925dc]) with a slot (+0x10) gives it the mesh's +0x50 + 8 i
// (+0), its count (+6, from +4), the counts before it (+4) and its words +0x14, +0x18 (+8, +0xa), and its items' (from +8, on along +8)
// two dwords go on at the animator's +0x1c, 8 bytes each; the mesh's words +0x38, +0x3a (the vertex counts 0x79c600, 0x7925c4, + 3,
// / 4), +0x3e and the part's +0x58, +0x5c, +0x5e (0x79c600, 0x7923e8, 0x7925c4) set; the animator woken (Animator_Wake). Then the lists'
// +4..+0x18 to 0, 0, 0, 0, 10000, 0; the pool cursor 0x79c604 back to 0x792408; the track's ([0x790fe4]) float +0x38 to its fraction
// (CRT modf). For keep 0 the animator is replaced by a clone of the build's (Animator_Clone, flags 0x21ef), its +0x38 the old one, its
// holder placed at the origin (Holder_SetPosition), its mesh's word +0x3c and part's +0x5a cleared; 0x7923e0, 0x7925d4, 0x7925cc and
// 0x792410 take the sizes' words +6, +8, +0xc, +4; the counts 0x79c600, 0x7925c4, 0x7925d8, 0x7923e8, 0x7925c8 0; the bounds +-1e6.
void __cdecl CoasterMesh_Finish(u32 keep);
}
