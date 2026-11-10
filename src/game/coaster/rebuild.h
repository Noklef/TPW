// A coaster's track rebuilt after a change: reimplementation of 0x43eb70.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x43eb70 (cdecl; name unconfirmed). [0x792218] = coaster. A new type (+4, kept at 0x7923ec): its parts' flag 4 (type +0x1c: count word +4, 0x10-byte parts at +0x10)
// into the table [0x7925dc], 0x1c bytes apart. Coaster_PoseCars. Each piece from coaster+0x54 (next +0x64, last +0x68): its nodes (+0x90: count, a, b; 0x58 bytes) are numbered on
// through all pieces (+0) and linked: +0x14 the one before (or the last piece's last), +0x18 the one after (or the next piece's first), in both a and b. Each node's slope (a +0xc, b +0xc): 0 at
// the piece's bank (+0x74's first; the piece off: 0); before it -(distance to the node / distance to the last piece); after it that to the next piece, without the sign and clamped to 0..1;
// 0 with no such piece. The refresh, Coaster_BuildNodes, Coaster_TestRun, Contacts_UpdateBounds, 0x439ab0, Owner_DisposeChildren, TrackMesh_PlaceSections. Then the mesh: 0x792414 = 0x792600,
// 0x79240c and the coaster's +0x34, +0x38 cleared, its search reset; each piece with the next (first +0x18 of [0x790fe4]; flag 0x10000 stops emitting, 0x20000 starts it and restarts): emitting,
// its nodes with themselves and it with the next, Keys_MergeSamples into 0x792418 and TrackMesh_EmitPiece; not, +0x34 takes the node's +8. 0x79240c runs on by each's count less 1, then +1.
// CoasterMesh_Finish(1); the coaster's flag 0x4000 cleared. sic: the first restart argument is what the last-piece slot held (the argument slot itself with no pieces).
void __cdecl Coaster_Rebuild(u8 *coaster);
}

// The calls with large worlds of their own (the bench's recorders; null: the ports).
struct CoasterRebuildEnv {
    void(__cdecl *pose)();
    void(__cdecl *refresh)();
    void(__cdecl *buildNodes)(u8 *coaster);
    void(__cdecl *testRun)(u8 *coaster);
    void(__cdecl *updateBounds)(u8 *coaster);
    void(__cdecl *f439ab0)(u8 *coaster);   // by address
    void(__cdecl *dispose)(u8 *coaster);
    void(__cdecl *placeSections)();
    void(__cdecl *searchReset)(u8 *search, u8 *coaster, u32 zero);
    u32(__cdecl *merge)(const u8 *a, const u8 *b, const float *keys, float *out, float span);
    void(__cdecl *emit)(u8 *piece, u8 *next, s32 count, const float *positions, u32 restart);
    void(__cdecl *finish)(u32 keep);
};
void CoasterRebuild_SetEnv(const CoasterRebuildEnv *env);

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gRebuild_7923e4, 0x007923e4, 0x4);
GAME_DATA(u8 *, gCoaster_Rebuild_7923ec, 0x007923ec, 0x4);
GAME_DATA(float, gRebuild_79240c, 0x0079240c, 0x4);
GAME_DATA(float *, gRebuild_792414, 0x00792414, 0x4);
GAME_DATA(float[100], gRebuild_792418, 0x00792418, 0x190);
GAME_DATA(u32, gRebuild_7925f8, 0x007925f8, 0x4);
