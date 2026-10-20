// A block of vertices kept four abreast: reimplementation of 0x53ddd0 (its one caller, 0x53df30, adds the corners of the scene's
// generated faces). Its arrays: positions at +0x64 (0x30 bytes a group of four: x, y, z each four floats), normals at +0x68 (three floats
// a vertex) and texture coordinates at +0x6c (0x20 bytes a group: u, v each four floats); the count of vertices at +0x9c.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x53ddd0 (thiscall, pops 0x1c; name unconfirmed): adds a vertex at the count, which goes up by one. With wave not 0, y is raised by
// the ground's height at (x, z) (Terrain_HeightAt) and by the sine table's entry for (the float at 0x874fc0 + z + x) * its scale, and the
// normal is (0, (that entry + 1) / 2, 0); else the normal is the three floats at normal, or (0, 1, 0) for none. Each float is stored by
// the x87 (a signalling NaN quietened).
void __fastcall VertexBlock_Add(u8 *block, void *, float x, float y, float z, float u, float v, u32 wave, const float *normal);

// 0x53e530 (thiscall (positions, unused, uvs, normals), pops 0x10; name unconfirmed; 0x53dc4f adds the scene's generated quads with it): the
// four corners i = 0-3 added (VertexBlock_Add, no wave): x, y, z the three floats at positions + 0xc * i, u and v the first two of
// uvs + 0xc * i, the normal at normals + 0xc * i. Then two triangles of the count before (base) in the index list [+0x94] (8 bytes an
// entry, its words +2, +4, +6 written, +0 left): (base + 2, base + 1, base) and (base + 1, base + 2, base + 3), the list stepping 8 and its
// count +0x98 one up after each. Returns the new +0x98.
u32 __fastcall VertexBlock_AddQuad(u8 *block, void *, const u32 *positions, u32 unused, const u32 *uvs, const u8 *normals);

// 0x53df30 (thiscall (x, y, z, face, turn, type, unused, big, unused, fixed), pops 0x28; name unconfirmed): a face of a cell-sized cube
// (side 20 when big, else 10; one 10 high) added as four vertices (VertexBlock_Add, the normal (0, 1, 0)): face 0 the top (y), 1 the
// side at z, 4 the side at x + side, 0x10 the side at z + side, 0x40 the side at x; any other face adds nothing. Unless fixed, with
// [0x874fc4] 0 and the type below 13, in scenes 4, 0xe and 0x13 (Global81ae2c_Get_52f840, asked up to three times) x and z are moved
// by -10 * the dwords at 0x8186b8 / 0x8186b4 and +the floats at 0x820afc / 0x820af8 first. The vertices wave for a type other than -1
// below 15; their texture coordinates are entries of the 12-byte table at 0x871ee8 (from 5 for the types 8-14), turned by the byte
// turn (4, 0x10, 0x40: a quarter, a half, three quarters). Then the two triangles of the count before (base): (base + 1, base, base + 3)
// and (base, base + 2, base + 3) as VertexBlock_AddQuad's. Returns the new +0x98.
u32 __fastcall VertexBlock_AddFace(u8 *block, void *, u32 x, u32 y, u32 z, u32 face, u32 turn, s32 type, u32 unused6, u32 big, u32 unused8,
                                   u32 fixed);

// 0x53d790 (thiscall (cubes, quads, result, name, head, five dwords), pops 0x2c; name unconfirmed): a block of that many cubes' tops
// and quads built. *result = 1 first; +0 the head, +4 0, the word +8 the quads; +0xc the string 0x763ee4 or 0x74cb98 (Flag54dcc0_Test),
// the path at 0x871f68 (its +0x10) 0x763ed4's text, the one at +8 of the object 0x786b68 (Obj_Self) and 0x763ec0's or 0x763eac's (asked
// again) after it; +0x14 0x763b38. Its record (+4) and animator (+0x54: flags 3, one node, +0x24: flags 3, 2 * cubes triangles, 4 *
// cubes vertices, normals and coordinates, the quads, the five dwords at +0x34..+0x48; the rest cleared, +0xa0 the cubes) made into a mesh
// (Animator_Create with the name and flags 3, its result to *result). When that gives 1: the node's matrix the identity, the counts
// +0x9c and +0x98 0, the index list +0x94 the first part's (+0x70); each cube's top added (VertexBlock_AddFace at the origin, turn 0xff,
// type -1, 1.0f, small, not fixed); quad i's entry (0x10 bytes at +0x50) given +0x1c + 8 * i and its words +4, +6 cleared; the animator
// woken (Animator_Wake). Returns the block.
u8 *__fastcall VertexBlock_Construct(u8 *block, void *, u32 cubes, u32 quads, u32 *result, const char *name, u32 head, u32 a5, u32 a6, u32 a7,
                                     u32 a8, u32 a9, u32 a10);

// 0x53d9e0 (thiscall; name unconfirmed; called for the blocks [0x871f64], [0x871ee4], [0x871ed8] once the list is sorted (0x53c1f0)):
// the block's share of the scene's generated faces added afresh. The index list +0x94 the first part's (+0x70), the counts +0x9c, +0x98
// 0, the first 20 entries (0x10 bytes at +0x50, by type) cleared at +4, +6, +8, +0xa. The faces are the 0xa0-byte records at 0x820b00 +
// (index >> 5) * 0xa0 for the [0x871ee0] (signed) indices at 0x872168: those whose +0x90 is the block's +0 are its, and the first other
// one after them ends the walk. For each: a type (the byte +0x9c) other than the last sets its entry's +4 to the word +0x98 and +8 to
// +0x9c; past the limit +0xa0 (signed) the face is skipped (the empty Debug_Printf); else with +0x94 set it is added as a cube's face
// (VertexBlock_AddFace: +0, +4, +8, the bytes +0xc and +0x10 (over the high bytes of +0x14 and +0x18), the type, +0x14, +0x18, the byte
// +0x1c (over +0xa0's), +0x98), else as a quad (VertexBlock_AddQuad: +0, the type, +0x30, +0x60); the box +0x34..+0x48 becomes x, y, z
// - 10 (at most 1024, a NaN too) and x, y, z + 10 (at least -1024) of the face's point, the entry's +6 the word +0x98 less its +4 and
// +0xa the word +0x9c less 1, and the count goes up. Last the words +0x28 2 * count, +0x2a and +0x2c 4 * count; returns 4 * count.
u32 __fastcall VertexBlock_TakeFaces(u8 *block);
}
