// The map's cell tables (gMapCells, 0x8023a0: 0x44-byte cells from +0, 0x28-byte ones from
// +0x110000, both named by a 1-based u16 handle) and the "World Struture" memory ranges at
// 0x7cdb30: reimplementation of 0x4d0a70-0x4d0b10.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d0af0 (thiscall on a u16 handle): its 0x28-byte entry, gMapCells + 0x10ffd8 + handle * 0x28.
u8 *__fastcall MapHandle_Entry28(const u16 *handle);

// 0x4d0b10 (thiscall on a u16 handle): its 0x44-byte cell, gMapCells + (handle - 1) * 0x44.
u8 *__fastcall MapHandle_Cell44(const u16 *handle);

// 0x4d0a70 (thiscall on the ranges; index, a, b, name): entry index (8 bytes) = (a, b); the
// name is unused. The world gives (itself, its size); the cell tables below (start, end).
void __fastcall MemRanges_Set(u8 *ranges, void *, u32 index, u32 a, u32 b, const char *name);

// 0x4d0a90 / 0x4d0ac0 (cdecl; base, stride): ranges entry 2 / 3 (0x7cdb40 / 0x7cdb48) =
// (base, base + stride * 0x4000), and the stride to 0x7cdb50 / 0x7cdb54.
void __cdecl MemRanges_SetCells44(u32 base, u32 stride);
void __cdecl MemRanges_SetCells28(u32 base, u32 stride);
}
