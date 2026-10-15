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
}
