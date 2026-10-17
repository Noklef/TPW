// The lobby's model owners: methods of three front-end classes that hold lobby models (src/game/frontend/lobbymodel, set up by
// RideUser_Acquire): reimplementation of 0x5da120, 0x5dfca0 and 0x5e1350 (thiscall). Named by their vtables until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5da120 (class 0x702af0; name unconfirmed): once (the flag at +4 then set to 1), each item of the list at +0x58 (head +4; nodes {item, next})
// whose flag at +0xc is clear has its model at +8 set up (RideUser_Acquire) and the flag set to 1.
void __fastcall Class702af0_AcquireModels(u8 *object);

// 0x5dfca0 (class 0x702ca8; name unconfirmed): once (the flag at +0x10 then set to 1), the models at +4 and +8 set up (RideUser_Acquire).
void __fastcall Class702ca8_AcquireModels(u8 *object);

// 0x5e1350 (slot 6 of the vtable 0x702cb0; name unconfirmed): 0 when the state at +0x14 is 0; otherwise 1, the state cleared, after a pending play
// (state 2) is started: LobbyModel_Play(the model at [+0xc] + 8, entry 1, now 0).
u32 __fastcall Class702cb0_StartPendingPlay(u8 *object);
}
