// A cursor on the track: two ends, each a node index into a track piece (+0 index and +4
// piece for the start, +8 and +0xc for the end). A piece links on to +0x64 and back to
// +0x68; +0x90 points at its node list {count, 0x58-byte nodes}.
#pragma once

#include "../../../re/types.h"
#include "spline.h"

extern "C" {

// 0x440ad0: steps the cursor one node on. Within one piece both indices step until the end
// reaches the piece's last node, then the end moves to the next piece's node 0. Across two
// pieces the start moves to the end's piece: to node 0, the end stepping on, when that
// piece has more than one node; else to its only node, the end going on to the next piece
// (keeping its index).
void __cdecl TrackCursor_Next(u32 *cursor);

// 0x440b50: steps the cursor one node back, the mirror of TrackCursor_Next: within one
// piece both step back until the start is at node 0, then the start moves to the last
// node of the previous piece and the end to node 0. Across two pieces the end moves to the
// start's piece: to the start's index, the start stepping back, when it has more than one
// node; else the start goes back to the previous piece's last node.
void __cdecl TrackCursor_Prev(u32 *cursor);

// 0x440bc0: TrackSpline_Point between the cursor's two nodes at t.
void __cdecl TrackCursor_Point(const u32 *cursor, float t, Vec3 *out);

// 0x43e520: piece n of the track: from its first (+0x18), n steps on (+0x64).
u32 __cdecl Track_PieceAt(const u8 *track, u32 n);

// 0x444040: clears flags 0x4000 and 0x1000000 (+8) of the pieces of the selection (ends at
// 0x79c628 and 0x79c62c), from the end with the lower index (+0, unsigned; 0x79c62c on a
// tie) up to but not including the other; 0 when 0x79c628 is null, else 1.
u32 __cdecl TrackSelection_ClearFlags();

// 0x444700: numbers the pieces from piece on (+0x64) after its previous one (+0x68: its +0
// plus 1, else 0), each at +0 and as a float at +0x6c.
void __cdecl TrackPieces_Renumber(u8 *piece);
}
