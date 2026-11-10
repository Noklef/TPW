// A lobby model set flying between two random points.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

// Its calls go by the game's addresses; a table of replacements (tests/fncmp/lobbyflight.py gives them recorders).
void LobbyFlight_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x5d9830 (thiscall (owner, a, b, centre, spread, e, f), pops 0x1c; name invented, unconfirmed): vtable 0x702ae8; a new lobby model of (a, b) from [0xf83774], its slot (0, 1) set; start (+0x30) and end (+0x3c) each centre + random(0..1) * spread - spread / 2 on the generator at [0xf82884]+0x34; the direction (+0x48) between them normalised.
// The model placed at the start, the owner's slot 1 told of it. Returns self.
u8 *__fastcall LobbyFlight_Ctor(u8 *self, void *, u8 *owner, u32 a, u32 b, const u32 *centre, const u32 *spread, u32 e, u32 f);
}

#include "../core/gamedata.h"

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[1], Class702ae8_vtable, 0x00702ae8, 0x4);
