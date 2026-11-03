// A hoarding's boards built as a mesh around a grid of cells (name invented, unconfirmed): reimplementation of 0x452e70.
#pragma once

#include "../../../re/types.h"

// Its calls (a bench's recorders in their place).
struct HoardingMeshEnv {
    void *(__cdecl *alloc)(u32 size, u32 zero);                                    // GlobalMem_Alloc (0x45b9d0)
    s32(__cdecl *format)(char *to, const char *format, const char *level);         // sprintf (0x67a650)
    u32(__cdecl *create)(u8 *animator, u8 *record, const char *name, u32 flags);   // Animator_Create (0x424ef0)
};
void HoardingMesh_SetEnv(const HoardingMeshEnv *env);

extern "C" {
// 0x452e70 (cdecl; level, object, grid; name invented, unconfirmed). Nothing when the u16 at [object + 8] + 0xc is under 2. The grid: width +0, height +4, from +0xc its cells of 8 bytes, 0x14 to a row, each
// byte's bits 1, 4, 0x10 and 0x40 a board on a side. The boards are numbered by the bearing of each one's middle from the grid's middle (fmod of the angle + 5 pi / 4 by 2 pi; a stable sort). An animator of
// 0x40 bytes (3, 1 part) and its part of 0x30 (4, boards * 4 vertices, boards, 1; a box of -5, -15, -5 and 5 + 10 * width, 25, 5 + 10 * height) go to Animator_Create with the record at 0x74d008, whose
// +8 the textures' folder made from the format at [0x74cfc0] and the level (a local of the frame: sic, kept in the record). The part's matrix (+0x28) the identity, its first range (+0x2c) from the record's
// +0x18. With no mesh at [object + 8] + 0x2c: no more (the blocks kept by nobody, sic). Else the mesh's +0x58 points through its matrix +0x10 (x, z kept). Each cell's corners move by half the way to the
// point second nearest (the sum's less or unordered taken as nearer, as x87 compares; a grid of 1 by 1: 0.3 each way). Each board, 10 long and 10 high, four vertices of x, y, z (+0x10 by fours), its
// normal four times and its back's once after all boards (+0x14), its neighbours (an edge's two cell ids) at the edges block, u, v (+0x18), four triangles' words (+0x1c, each first word's top bit kept;
// the normals' index the rest). Then a block of 0x10 (boards, width, height, edges) at object + 0xb0, the animator at +0xb4, object + 4's bit 2. Sic: with a mesh of under 2 points and a grid over 1 by 1 the
// original's corner moves are unset stack but for the first (the port's are 0; not benched), and a mesh's points over 1023 run off the frame.
void __cdecl HoardingMesh_Build(const char *level, u8 *object, const u8 *grid);
}
