// An engine object's model (the class with vtable 0x7019f8, LiveObject7019f8_Construct in core/liveobjects): reimplementation of its
// unload, 0x57a100.
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
}
