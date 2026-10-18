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

// 0x5e1210 (class 0x702cb0; its one call 0x5e079b; name unconfirmed): the lobby camera's step. The model at [+0xc]'s +0x3c triple is copied
// to the target (+0x4c) and the eye (+0x34); the eye is then moved by the angle +0x18 and radius +0x20 to (x + sin * r, y + height +0x24,
// z + cos * r). The position +0x28 eases toward the eye by the frame's scale ([0xf82884] + 8) times 0.1, the look-at +0x40 toward the target
// by it times 0.2 (each a + (b - a) * k, x87 in the original's order), and both go to the camera at [0xf83778] (+8 and +0x14).
void __fastcall Class702cb0_StepCamera(u8 *object);

// 0x5d9db0 (class 0x702af0's constructor, thiscall; count, two names, two 12-byte triples by pointer, two dwords; pops 0x1c; the SEH frame left
// out; name unconfirmed): the strings at +0x10 and +0x24 constructed (String_Ctor), the vtable 0x702af0, a new 0x10-byte list (vtable 0x702aec,
// +4..+0xc cleared; null when the allocation failed) at +0x58, +4 = 0; +8 = count * the percentage at 0x7858dc / 100 (signed, the product
// wrapping), at least 1 and at most the count; +0xc = 0; the names assigned to the strings (their slot 5, +0x14); the triples copied to +0x38
// and +0x44, the dwords to +0x50 and +0x54; the object handed to the registry at [0xf83784] (its slot 6, +0x18). Returns the object.
u8 *__fastcall Class702af0_Ctor(u8 *object, void *, s32 count, const char *name, const char *name2, const u32 *first, const u32 *second, u32 a,
                                u32 b);
}
