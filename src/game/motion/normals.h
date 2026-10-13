// Face normals of animated nodes: reimplementation of Node_UpdateNormals (0x473a40) and
// Model_UpdateNormals (0x473c30).
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x473a40 (node): recompute the face normals of a 0xa0-byte node whose vertices moved.
// Faces (u16 count +0x5c, 8 bytes each at +0x70): {normal index (top bit masked off),
// vertices v1, v0, v2}, each vertex remapped (u16 table at +0x94) into the positions at
// +0x60 (blocks of four: x[4], y[4], z[4]). The normal (12 bytes at +0x64 + 12 * index) is
// (v2 - v0) x (v1 - v0), normalized; when its x comes out NaN (a degenerate face) it is
// (0, -1, 0). Clears node flag 0x10000.
void __cdecl Node_UpdateNormals(u8 *node);

// 0x473c30 (model): Node_UpdateNormals on each primary node (u16 +0x44, at +0x70) flagged
// 0x10000, then clears model flag 0x40000 (+0x30).
void __cdecl Model_UpdateNormals(u8 *model);
}
