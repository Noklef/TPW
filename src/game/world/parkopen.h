// The park opened or closed (0x519ef0, thiscall on the game object): its flag +0x1da710 (set: closed), the gate's ride record (the thing
// whose handle is the word +0x1da732), the objects on the list from the word +0x1da746 (linked by their words +0xc), and a message 13
// posted to the message centre.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the opening calls in the game that is not ported yet (the benches point it at a recorder).
struct ParkOpenEnv {
    void(__fastcall *openObject)(u8 *object);   // 0x4df390 (agent C's; thiscall on an object that can be opened)
    void(__fastcall *endGate)(void *owner, void *, u8 *gate);   // 0x481b50 (agent C's; thiscall on 0x7b51f0, this not used, pops 4)
};
// The calls go to env (nullptr: the game's).
void ParkOpen_SetEnv(const ParkOpenEnv *env);

// 0x519ef0 (thiscall on the game object (open, gateOnly), pops 8; name invented, unconfirmed). With gateOnly: element 0 of the gate's ride
// record (Rides_FindById of the gate's +0x24; the gate the thing whose handle is the word +0x1da732, Thing_ByHandle) set to 2 (its element 1
// read first), nothing else. Otherwise, opening (open not 0) a closed park (+0x1da710 set): the flag cleared, the gate's element 0 set to 1
// (element 1 read first), and every object of the list from the word +0x1da746 (the thing table 0x7cfb90, linked by the words +0xc) whose
// byte +0x32 has bit 2 and that can be opened (Object_CanOpen) opened by 0x4df390; then (also for a park already open) a message 13 of 3
// posted (Object_PostMessage: agent B's unported 0x40fb10 through the objects' env). Closing (open 0) an open park: the flag set; when
// no kind 1 thing stands on a plain cell (Things_CountKind1OnPlainCell; the original loads the game table entry of +0x1da720 for it, unused)
// and the gate's element 1 is 1 (a gate handle of 0: -1), the gate's element 0 set to 0 (element 1 read again first); every object of the
// list with bit 2 of +0x32 restarted (Object_RestartRide); then (also for a park already closed) a message 13 of 4 posted. The debug prints
// ("*** You have just %s your park ***") are left out.
void __fastcall Game_OpenPark(u8 *game, void *, u32 open, u32 gateOnly);

// 0x5168f0 (thiscall on the game object; name invented, unconfirmed): the park ended. The game's state +0x1da738 set to 4; the gate (the
// thing whose handle is the word +0x1da732, read from the thing table 0x7cfb90 inline) passed to 0x481b50 on 0x7b51f0; sprite drawing off
// (SpriteDraw_Disable); every thing in use (from 0x7cf56c, newest first; the next one found through Thing_GetHandle's slot) that is not
// deleted (byte +3 0), is an object (byte +2 3) and has bit 2 of +0x32 gets its ride record's element 4 set (Obj_SetRideElement4One); the
// delete queue's things released (Things_ReleaseDeleted, inlined in the original with the walk's last handle, 0, as the start value);
// then the gate's element 0 set to 2 (Game_OpenPark (0, 1)). The debug prints are left out. Called by 0x4d032a.
void __fastcall Game_EndPark(u8 *game);
}
