// The inverse colormap (0x642150-0x64285b): for every cell of a bits-per-channel RGB cube, the
// index of the nearest palette colour, built by Spencer Thomas's incremental-distance method
// (Graphics Gems II, inv_cmap.c) almost line for line. The builder object holds what the C
// version kept in file statics; the loops' own statics (here/min/max/increments and the
// "gc" copies) stay in the game's globals at 0xfaa500. Palette entries are bytes R, G, B, x;
// the distance buffer is colormax^3 dwords, filled with ~0 first; the map is colormax^3 bytes
// in R, G, B order (red the slowest).
//
// The originals' quirks are kept: distances are compared unsigned, so an overflowing one
// (bits 0, where a cell is the whole cube) can lose; ties keep the earlier colour.
#pragma once

#include "../../../re/types.h"

struct InverseMap {          // vtable 0x704ac0, 0x58 bytes
    void *const *vtable;
    s32 index;               // +4: the colour being placed
    s32 blueCenter;          // +8: its cell
    s32 greenCenter;         // +0xc
    s32 redCenter;           // +0x10
    u32 greenDist;           // +0x14
    u32 redDist;             // +0x18
    u32 centerDist;          // +0x1c
    u32 blueInc;             // +0x20
    u32 greenInc;            // +0x24
    u32 redInc;              // +0x28
    u32 *greenDp;            // +0x2c
    u32 *redDp;              // +0x30
    u32 *centerDp;           // +0x34
    u8 *greenRgb;            // +0x38
    u8 *redRgb;              // +0x3c
    u8 *centerRgb;           // +0x40
    u32 greenStride;         // +0x44: colormax
    u32 redStride;           // +0x48: colormax^2
    u32 x;                   // +0x4c: 1 << (8 - bits), a cell's width
    u32 xsqr;                // +0x50
    u32 colormax;            // +0x54: 1 << bits
};
static_assert(sizeof(InverseMap) == 0x58, "InverseMap is 0x58 bytes");

struct InverseMapStatics {   // 0xfaa500
    u32 greenCenterDist;     // gcdist
    u32 *greenCenterDp;      // +4 gcdp
    s32 blueMin;             // +8
    s32 blueHere;            // +0xc
    u32 redXx;               // +0x10 rxx
    s32 greenMin;            // +0x14
    s32 blueMax;             // +0x18
    u32 greenXx;             // +0x1c gxx
    u32 blueInc;             // +0x20 binc
    s32 greenHere;           // +0x24
    u32 greenInc;            // +0x28 ginc
    u8 *greenCenterRgb;      // +0x2c gcrgbp
    s32 greenMax;            // +0x30
};
static_assert(sizeof(InverseMapStatics) == 0x34, "InverseMapStatics is 0x34 bytes");

extern "C" {
// 0x642150: returns m. 0x642160 (flags) / 0x642180: the destructors (vtable only).
InverseMap *__fastcall InverseMap_Ctor(InverseMap *m);
void *__fastcall InverseMap_DeletingDtor(InverseMap *m, void *edx, u32 flags);
void __fastcall InverseMap_Dtor(InverseMap *m);
// 0x642190 (first, count, palette, bits, dist, map): inv_cmap for colours first..first+count-1.
void __fastcall InverseMap_Build(InverseMap *m, void *edx, s32 first, s32 count, const u8 *palette, u32 bits,
                                 u32 *dist, u8 *map);
// 0x642340 / 0x642460 (restart) / 0x6426b0 (restart): redloop, greenloop, blueloop.
s32 __fastcall InverseMap_RedLoop(InverseMap *m);
s32 __fastcall InverseMap_GreenLoop(InverseMap *m, void *edx, s32 restart);
s32 __fastcall InverseMap_BlueLoop(InverseMap *m, void *edx, s32 restart);
// 0x642840 (dist, colormax): maxfill.
void __stdcall InverseMap_MaxFill(u32 *dist, s32 colormax);
}

struct InverseMapEnv {
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
    InverseMapStatics *statics;        // 0xfaa500
};
// The calls and statics go through env (nullptr: the game's).
void InverseMap_SetEnv(const InverseMapEnv *env);
