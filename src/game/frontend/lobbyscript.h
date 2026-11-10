// The lobby scene's script read (0x5e3210).
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

extern "C" {

// Its calls that the bench replaces go by the game's addresses; a table of replacements (tests/fncmp/lobbyscript.py gives every one a recorder).
void LobbyScript_SetEnv(const AddrCall *table, u32 count);

// 0x5e3210 (thiscall (name, a2, index, a4, a5), pops 0x14; name invented, unconfirmed): the text file of the String at 0xf855b8, name and ".txt" read whole into a Mem_GlobalAlloc block (no file: its null stream passed on, sic), its
// '\n's made NULs, then line by line by each line's start:
// - ISLAND(i,"a","b","c","text",degrees,y): a new Class702ca8 (Class702ca8_Ctor, then Class702ca8_AcquireModels; none: on a null, sic) of this, i (or index, when not negative), name, the texts, degrees, the point (the float 0xf82884's +0x10 + 0x1c + 4 * i, y, the float + 0x44 + 4 * i), the wide text, a2, a4, a5.
// - FLYINGMESH("a","b",n,x,y,z,last): a new Class702af0 (Class702af0_Ctor, Class702af0_AcquireModels) of n, the texts, the scene's +0x10's +0x3c triple with its y less the double 0x702d40, x, y, z, last, 500.
// - LIGHTNING(n), RAINY(n): the scene's +0x10's +0x5c, +0x60. SKYCOLOUR(r,g,b): its +0x64, +0x68, +0x6c, each << 16. Any other line: nothing.
// A missing delimiter runs on past it, sic.
void __fastcall LobbyScript_Load(u8 *self, void *, const char *name, u32 a2, s32 index, u32 a4, u32 a5);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const double, gLobbyscript_702d40, 0x00702d40, 0x8);
GAME_DATA(char[11], gLobbyScript_Load_774d4c, 0x00774d4c, 0xb);
GAME_DATA(char[7], gLobbyScript_Load_774d58, 0x00774d58, 0x7);
GAME_DATA(char[11], gLobbyScript_Load_774d60, 0x00774d60, 0xb);
GAME_DATA(char[12], gLobbyScript_Load_774d6c, 0x00774d6c, 0xc);
GAME_DATA(char[8], gLobbyScript_Load_774d78, 0x00774d78, 0x8);
