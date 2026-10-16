// Searches over the things for the track ride that has the most of something: reimplementation of 0x4cc4b0, 0x4cc5d0 and 0x4cc6f0 (stdcall). Each walks the things newest
// first (the world at [0x80239c]), considers the objects (model byte at +2 is 3) whose definition (ObjectDefs_Find on the id at +0xe) has its dword at +0x4ac clear and that
// pass a test, measures the track ride its handle (+0x28) names (TrakRide_CountPieces), and keeps the one whose measure is highest (signed; the first of equals). The handle
// is stored to *out (0 when there is none); the measure of the chosen one is returned (0 when there is none).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4cc4b0 (out): the definition's kind (+0x9c of Object_GetDef) is 1; the measure is the kind 11 pieces / 2.
u32 __stdcall Rides_BestKind11Count(u16 *out);

// 0x4cc5d0 (out, id): the definition is of kind 1, 2 or 3 (Obj_DefKind123) and the word id is 0 or the object's id (+0xe); the measure is the pieces / 2.
u32 __stdcall Rides_BestPiecesOfKinds(u16 *out, u32 id);

// 0x4cc6f0 (out, kind): the definition's kind (+0x9c of Object_GetDef) is the given one; the measure is the pieces / 2.
u32 __stdcall Rides_BestPiecesOfKind(u16 *out, u32 kind);

}  // extern "C"
