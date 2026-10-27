// Readying a zero-ended array of meshes: reimplementation of 0x5744d0 and 0x574110 (x87 at the game's 24-bit precision).
//
// The meshes (at least 0x5c bytes) have their flags at +0x50 and a link at +0x58. The object at
// [0x87a248] keeps +0x30 = the address of the last mesh with flag 0x20's +0x38; the meshes without
// flag 0x10000000 are pushed on the list headed at 0x879dec. The object's +0x34-+0x3c is a direction (0x574110, set by the drawing
// at 0x57ab29 from the camera's position to a point). The list's entries look like lights (0x5741b0 walks it by kind +0x50 & 0xf: 2,
// 4 and 6, adding their colours at +4-+0x10 into 0x879db0), so the names may be wrong.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5744d0 (thiscall on the object; list): for each mesh in the zero-ended array: flag 0x20 sets
// the object's +0x30 = mesh + 0x38; without flag 0x10000000, the mesh's +0x58 = the list head
// (0x879dec) and the head = the mesh (the flag isn't set, so a mesh readied twice links to itself
// (sic)). An empty array does nothing. Returns 1.
u32 __fastcall Meshes_Ready(u8 *object, void *, const u32 *list);

// 0x574110 (thiscall on the object; x, y, z: pops 0xc; name unconfirmed): +0x34 / +0x38 / +0x3c = (x, y, z) made unit length:
// len = sqrt((x * x + y * y) + z * z), each (1 / len) * coordinate; when len is 0 or a NaN, the three copied as they are (as dwords).
void __fastcall Meshes_SetDirection(u8 *object, void *, u32 x, u32 y, u32 z);

// 0x5743a0 (thiscall on the object; matrix, a second argument never read (its slot the len's spill), pops 8; name unconfirmed): the view. 0x879dcc = the matrix's address; its sixteen dwords
// copied to 0x879df0 and inverted there (Mat4_Invert); then for each mesh on the list (0x879dec, read before the copy; through +0x58):
// flag 2 (+0x50) transforms the direction +0x2c into +0x38 (Mat4_Transform mode 3) and makes it unit length: with
// len = (y y + z z) + x x stored as a float, unless len is 0, 1 or a NaN, inv = 1 / sqrt(len) and (x inv, inv y, inv z); flag 4
// transforms the point +0x14 into +0x20 (mode 2). Last the object's direction +0x34 in place (mode 3). Returns 1.
u32 __fastcall Meshes_SetView(u8 *object, void *, const u32 *matrix, u32 unused);

// 0x5741b0 ((normal, x, y, z), pops 16; ECX only seeds a spill slot; name unconfirmed): the light at a point. 0x879db0-0x879dbc =
// the ambient 0x879dd8-0x879de4; then for each entry on the list (0x879dec, through +0x58) by its kind (+0x50 & 0xf), its colour
// (+4-+0x10) times a factor added in (x87, each sum stored): kind 2 f = (-(+0x38 nx) - +0x3c ny) - nz +0x40, when f's sign is clear;
// kind 4 d = (dz dz + (dy dy + dx dx)) (each partial stored as a float; dx = +0x20 - x and so on), the factor (+0x48 - d) * +0x4c when
// d's bits are below +0x48's as signed ints (sic); kind 6 f = (dy ny + dz nz) + dx nx, when f's sign is clear the factor
// ((+0x48 - d) * +0x4c) * f with d the last kind 4 entry's (before any: ECX's bits, sic).
void __fastcall Meshes_LightAt(u32 seed, void *, const float *normal, float x, float y, float z);

// 0x574530 (thiscall on the object (colours, normals, points, count), pops 16; called at 0x56f040; name unconfirmed): a colour for each
// of count points (x, y, z at 0x10 bytes a point, the normals 12 bytes apart): with flag 0x800 in [0x8bcbc8] each is the object's +0x2c;
// else the light at it (Meshes_LightAt, the object as the seed) as red, green and blue: 0x879db4-0x879dbc * [0x7016e4], each
// rounded (fistp through 0xfb34a4, which keeps the last) and held to 0-0xff, alpha 0xff, the whole masked by the object's +0x2c.
void __fastcall Meshes_LightPoints(u8 *object, void *, u32 *colours, const float *normals, const float *points, u32 count);

// 0x574660 (thiscall on the object (colours, mesh), pops 8; called at 0x57ab6e and 0x57ab92; name unconfirmed): Meshes_LightPoints for
// the word +0x5e vertices of a mesh: vertex i the word i of the index list +0x94 into the positions +0x60 (x, y, z each four floats
// a group of four), its normal the i-th of +0x64.
void __fastcall Meshes_LightMesh(u8 *object, void *, u32 *colours, const u8 *mesh);
}
