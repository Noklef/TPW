// An engine object's model (the class with vtable 0x7019f8, LiveObject7019f8_Construct in core/liveobjects): reimplementation of its
// unload, 0x57a100, and load, 0x579e00.
//
// The object keeps its model at +8 (flags +0x30; +0x36 texture sets as 8-byte pairs from +0x50, the set at +4 of each; +0x44 meshes
// of 0xa0 bytes from +0x70), its flags at +4 and a tracked block at +0x144 (the load, 0x579e00, makes it and unloads on failure).
#pragma once

#include "../../../re/types.h"

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
}
