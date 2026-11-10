// The world's part of a save game: reimplementation of World_Save 0x516c80 and World_Load 0x5179c0. The stream holds a dword, the world's
// handles and clocks (the 25 fields of worldsave.cpp's table), the object controls +0x1d943c, the globals, the map +0x2d8, then the things:
// the first one's handle, and for each its next's handle (0 for none), its kind and its own serializer's fields.
#pragma once

#include "../../../re/types.h"

struct Stream;

// The serializers (thiscall; stream, save, 1, the last argument none of them reads): World_Save passes the 2 it writes as its first dword,
// World_Load the dword it read.
using WorldSerializeFn = u32(__fastcall *)(u8 *object, void *, Stream *stream, u32 save, s32 version, u32 unused);
struct WorldSaveEnv {
    void(__cdecl *initTable)();     // Things_InitTable 0x515f30
    WorldSerializeFn controls;      // ObjectControls_Serialize 0x4d3aa0
    WorldSerializeFn globals;       // WorldGlobals_Serialize 0x4d7a70
    WorldSerializeFn map;           // Map_Serialize 0x4d7ea0
    WorldSerializeFn kinds[19];     // by kind - 1; kinds 2 and 0x12 have none (only the original's prints)
};
void WorldSave_SetEnv(const WorldSaveEnv *env);   // nullptr: the ports

// 0x516c80 (thiscall (stream), pops 4): the stream's position asked for around each part (as the original's prints do, left out); an online base
// mouse mode made (MouseModeOnlineBase_Ctor) and set, the holder first (MouseModes_EnsureHolder); then the parts above in turn. 0 at the first that
// fails, else 1. A kind without a serializer (or past 0x13) writes its next and kind only. The list's first slot [0x7cf56c] must be set (sic: its
// handle is read through it).
u32 __fastcall World_Save(u8 *world, void *, Stream *stream);

// 0x5179c0 (thiscall (stream), pops 4): the things' table cleared (Things_InitTable); the parts above read in turn, 0 at the first that fails. Each
// thing: its slot's next, previous (the slot before it, or 0 for the first) and +0x10 set; a block of the kind's size and its constructor as the
// original inlines them (a failed allocation faults on the kind byte, as there), the kind byte +2, its handle (Fn_50b080), its serializer; the slot's
// +0 set only then. A kind without a serializer leaves the slot's +0 alone. Then every slot's +0x10 cleared and the empty ones linked as the free
// list from [0x801bbc]; the world's +0x1da738 == 4 turns the sprites' draw off (SpriteDraw_Disable), anything else on. 1.
u32 __fastcall World_Load(u8 *world, void *, Stream *stream);

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[12], gText_Worldsave_7627c8, 0x007627c8, 0xc);
GAME_DATA(char[16], gText_Worldsave_7627d4, 0x007627d4, 0x10);
GAME_DATA(char[16], gText_Worldsave_76286c, 0x0076286c, 0x10);
GAME_DATA(char[20], gText_Worldsave_76287c, 0x0076287c, 0x14);
GAME_DATA(char[20], gText_Worldsave_76289c, 0x0076289c, 0x14);
GAME_DATA(char[16], gText_Worldsave_7628b0, 0x007628b0, 0x10);
GAME_DATA(char[16], gText_Worldsave_7628c0, 0x007628c0, 0x10);
GAME_DATA(char[16], gText_Worldsave_7628d0, 0x007628d0, 0x10);
GAME_DATA(char[12], gText_Worldsave_7628e0, 0x007628e0, 0xc);
GAME_DATA(char[16], gText_Worldsave_7628ec, 0x007628ec, 0x10);
GAME_DATA(char[12], gText_Worldsave_7628fc, 0x007628fc, 0xc);
GAME_DATA(char[12], gText_Worldsave_762908, 0x00762908, 0xc);
GAME_DATA(char[16], gText_Worldsave_762914, 0x00762914, 0x10);
GAME_DATA(char[16], gText_Worldsave_762930, 0x00762930, 0x10);
GAME_DATA(char[12], gText_Worldsave_762940, 0x00762940, 0xc);
GAME_DATA(char[28], gText_Worldsave_76294c, 0x0076294c, 0x1c);
GAME_DATA(char[16], gText_Worldsave_762968, 0x00762968, 0x10);
GAME_DATA(char[16], gText_Worldsave_762978, 0x00762978, 0x10);
GAME_DATA(char[16], gText_Worldsave_762988, 0x00762988, 0x10);
GAME_DATA(char[12], gText_Worldsave_762998, 0x00762998, 0xc);
GAME_DATA(char[24], gText_Worldsave_7629a4, 0x007629a4, 0x18);
GAME_DATA(char[16], gText_Worldsave_7629bc, 0x007629bc, 0x10);
GAME_DATA(char[24], gText_Worldsave_7629cc, 0x007629cc, 0x18);
GAME_DATA(char[24], gText_Worldsave_7629e4, 0x007629e4, 0x18);
GAME_DATA(char[24], gText_Worldsave_7629fc, 0x007629fc, 0x18);
GAME_DATA(char[16], gText_Worldsave_7627e4, 0x007627e4, 0x10);
GAME_DATA(char[12], gText_Worldsave_762890, 0x00762890, 0xc);
GAME_DATA(char[12], gText_Worldsave_762924, 0x00762924, 0xc);
