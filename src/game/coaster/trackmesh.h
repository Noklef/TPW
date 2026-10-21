// The coaster track's mesh built along its pieces: cross sections placed by the track spline (xsection.h's CrossSection_Emit) and
// their triangles listed (lists.h). The current coaster is [0x792218] (+4 its build, +0x9c its nodes), the track [0x790fe4] (+0x34
// the blend carried on, +0x38 a length counted down, +0x3c the orientation). The section records are 0x10 bytes: a position (the
// first one past which a sample's position falls is the one used), +4 and +8 floats, +0xc the section ({points, indices, point
// buffer, 12-byte index entries}).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43f4e0 (cdecl; piece, next, count, positions, restart; name unconfirmed): a piece's samples (positions[1..count-1], less the
// start [0x79240c]) turned into mesh. The records: for a piece not yet built (+0xb0 0), not flagged 0x1000000 and its next not
// 0x802000, those at 0x7923f0 when the piece has flag 0x800 and the build's byte 0x40, else [0x792414] (the flag 0x7925f8 1 / 0 by
// that, a change of it forcing a section); otherwise the build's section +0x80 has its entries' words +4 set to the build's +0x20 -
// 1 + 2, 3 (next without 0x4000) or [0x7925d0] (a flagged piece without 0x4000), and the records 0x7925e8 (forcing a section). With
// the piece the next (+0x90: count, 0x58-byte nodes, up nodes), nothing when its +0x100's byte +4 has bit 0. The previous point:
// the piece's first node's +0x40 (the same piece) or its last's. When the piece's +0 is 0 or restart is not 0 (as a dword), the
// frame at the piece's start (TrackSpline_Frame at t 0 to the next's second node, or from the last node to the next's first), the
// mesh finished and begun again (CoasterMesh_Finish(0)) and the section emitted there (the first node's +8 as blend, separate), the
// counts (0x79c600, 0x7923e0, 0x7925c4, 0x7925d4) moved on by its points and nothing forced any more. Then for each sample: the
// frame there (same piece: the node of the position's whole part (CRT modf, __ftol), the last one's sample stepping back a node with
// the fraction + 1; else the last node to the next's first at the position), the blend of the nodes' +8 by t; the records walked on
// while their position is below the sample's (or unordered); a section is emitted at the previous point first when it moved, when
// forced, or when the points (x2) would pass 0x7923e0 or the indices (x2) 0x792410 (then also the mesh finished first and separate
// set), with the track's blend and its orientation +0x10..+0x18 as they were before the frame; the track's +0x38 lessened by the
// distance from the previous point / 6 (x87 order kept); the section emitted at the point; two triangles per index entry appended to
// the list ([0x7925dc] + 0x1c * the entry's word +4; its +0x14 / +0x18 the least / most vertex index), wound one way or the other by
// the toggle 0x7923e4; the counts moved on (0x7923e8 and 0x792410 by the indices x2), the blend stored at the track's +0x34.
void __cdecl TrackMesh_EmitPiece(u8 *piece, u8 *next, s32 count, const float *positions, u32 restart);

// 0x4402d0 (cdecl; name unconfirmed): the section records laid along the track by zone. The track's +0x38-+0x54 zeroed and +0x58
// FLT_MAX; the fixed records 0x7925e8 (1e7, 1e7, 0, the build's +0x80) and 0x7923f0 (the same with +0x84); the records from 0x792600
// (the first's +0xc the first zone box's +0x20). Then from position 0 in quarter steps until Coaster_IsPastEnd: the frame there
// (Coaster_FrameAt into the track's orientation; its roll fmod pi, the CRT's _CIfmod), the zone (Coaster_FindZone of the build with
// t 0). A zone change ends a record (+4 the position, +0 the spline value: CoasterNodes_SplineAt or its inlined copy, which reads the
// spline node's +0 unsigned) and with the build's flag +0x74 bit 0 looks for a transition box (+0x24 / +0x28 the zones from and to)
// whose +0x20 zone has a blend length (+0x14): positive, a blend record opened that far ahead (+8 1.0) and closed when the position
// gets there; else (0 or below, or a NaN) the records adjusted: with an open record ahead of where the length reaches, a blend share
// clamped to 0..1 (an open blend) moving the open one's start back, or a plain record of 1.0; otherwise the record before moved to
// the length's end. The record's +0xc the zone (or the transition's). Last: (1e5, the end position + 1). The frame's highest y is
// tracked in a local the original never reads.
void __cdecl TrackMesh_PlaceSections();
}
