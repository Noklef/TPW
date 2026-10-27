// An engine object's model (the class with vtable 0x7019f8, LiveObject7019f8_Construct in core/liveobjects): reimplementation of its
// unload, 0x57a100, and load, 0x579e00, and of the object's matrices set (0x57a260).
//
// The object keeps its model at +8 (flags +0x30; +0x36 texture sets as 8-byte pairs from +0x50, the set at +4 of each; +0x44 meshes
// of 0xa0 bytes from +0x70), its flags at +4 and a tracked block at +0x144 (the load, 0x579e00, makes it and unloads on failure).
#pragma once

#include "../../../re/types.h"

struct ObjModelEnv {
    s32(__cdecl *atExit)(void(__cdecl *fn)());   // crt_atexit (0x67a7d0)
};
// The atexit of the engine's static matrix (0xfb3400) goes to env (nullptr: the game's).
void ObjModel_SetEnv(const ObjModelEnv *env);

extern "C" {

// 0x57a100 (thiscall; name unconfirmed): with a model (+8): when the renderer flags (0x8bd550) have 0x20, each mesh without flag
// 0x200 whose faces (+0x5a, 16-byte records from +0x6c) include one, searching from the last, whose first pointer is set and points
// at a word with 0x1000 gets its tracked block (+0x74) freed (TrackedGlobal_Free; the pointer kept); then the saved pairs at +0x130
// and +0x124 released and the block at +0x13c freed (core/vobj2); each texture set released (TexSet_Release, gfx/texset) and, when
// that was its last user, freed (the game's heap) and cleared; the tracked block at +0x144 freed (even when 0), +0x148 and +0x144
// cleared and each mesh's +0x9c cleared; with the model's flag 2 (+0x30) the procedural texture object [0x879d60] (when there is
// one) unloaded (terrain/procedural). Then (with a model or not) +8 = 0 and flag 0x10000000 cleared. The object's +8 and the
// model's arrays are read again where the original reads them. Returns 1.
u32 __fastcall ObjModel_Unload(u8 *object);

// 0x579e00 (thiscall (model), pops 4; name unconfirmed; 0x45bbc0 asks it): +8 = the model, +4 = 0. With the model's flag 2 (+0x30):
// without the procedural texture object [0x879d60] or the model's terrain header (+0x6c) error (3, 0xb0754006) pushed on the engine's
// list (0x879ad0) and 0 returned; else that object set up from the model (TerrainTex_Setup). A model named "BlueprintMesh" (+0x18,
// 0x763e54) gets bit 2 in the byte +0xc of each of its +0x3c 16-byte parts (+0x64). Textures are "sharp" with the model's flag 0x200
// (dword +0x30), render flag 2 (0x8bcbc8) clear and a screen 0x200 wide with Flag54dcc0_Test, or 0x280 wide without. With 0x100 in the
// renderer flags (0x8bd550) its 0x40000000 is set (the sets' loads wait). Each texture set: its pair's set (+4) when 0 made (Mem_Alloc
// 0x10, TexSet_Ctor; 0 when that failed); when there is one, set up (TexSet_Init: the pair, the info +0x54 + 16 i) and, when sharp,
// each of the info's (+0xa) textures got (TexSet_Texture) gets flags 0x4100000. With 0x40000000 in 0x8bd550 then: the waiting sets
// loaded (TexSet_LoadDeferred) and it cleared. A tracked block of ((+0x44) + 4 (+0x3a)) * 4 bytes (TrackedGlobal_Alloc) at +0x144;
// none: error (2, 0xb0754000) pushed, the model unloaded (ObjModel_Unload) and 0 returned. +0x148 walks the block: each mesh with
// faces (+0x5e) gets it at +0x9c and 0x7fffffff at it; it moves on 4 (+0x5e) + 4 bytes. With 1 in 0x8bd508 the loading screen
// steps (LoadScreen_Tick on 0x8c4748). +4 gets 0x10000000; returns 1. The object's +8 and the model's arrays are read again where the
// original reads them.
u32 __fastcall ObjModel_Load(u8 *object, void *, u8 *model);

// 0x57a260 (thiscall; which, matrix: pops 8; name unconfirmed): the matrix (16 dwords) copied to the stack beside a flip diag(1, -1, 1,
// 1); for each of which's bits 1 and 2 the vertex transformer's ([0x87b1d8]) slot 6 called (cdecl) as (transformer, the copy, the flip,
// the engine's static matrix 0xfb3400) and that static matrix copied to the object's +0x10 (bit 1) or +0x50 (bit 2), the object's +4
// getting the bit. The static matrix is built the first time (the identity) under the guard 0xfb33e0 & 1 that 0x578473 and others
// share, its (empty) destructor 0x5d1750 given to atexit. The two stack matrices keep the game's layout (the flip, then the copy 0x40
// on), which the bench hashes through the flip's pointer.
void __fastcall ObjModel_SetMatrices(u8 *object, void *, u8 which, const u32 *matrix);

// 0x57aa10 (thiscall on the engine object; mesh, matrix, lights, flags: pops 0x10; name unconfirmed): a mesh's vertex colours lit, once
// a frame. Nothing (1) for a mesh of no vertices (+0x5e) or with flag 0x1000000, or whose colour buffer's stamp (the dword at [+0x9c])
// is the frame number 0x8bd50c already, with 0x8bd508's 0x40000000 and the mesh's 0xc0000 clear. Else the stamp set; then, unless the
// mesh's flag 0x1000000 (read again) or the render flag 0x800 (0x8bcbc8): the light list 0x879dec emptied, [[0x87a6d0] + 0x30] drained
// (ListNodes_Drain) and [+0x2c] too without flags' 0x3000000, the zero-ended light list readied (Meshes_Ready) without render flag
// 0x1000, the direction from the camera [0x87afe8]'s +0x30 to the mesh's +0x40 set (Meshes_SetDirection) with render flag 0x400, and
// the view (Meshes_SetView of the matrix and the mesh's +0x78). Then Meshes_LightMesh into the buffer after the stamp: with the object's
// [+8]+0x30 flag 0x800000 and 0x8bd524 = 4, under the mask 0xffff0000 at the lights object's +0x2c (its own put back). Returns 1.
u32 __fastcall ObjModel_LightMesh(u8 *object, void *, u8 *mesh, const u32 *matrix, const u32 *lights, u32 flags);
}
