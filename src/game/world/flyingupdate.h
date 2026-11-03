// The flying rides' step each frame. Reimplementation of 0x55abf0 (name invented, unconfirmed).
// The records are the table 0x8791f8 (flyingrides.h); their 20 slots are 0xe4 bytes each from +0x5c, in use with +0x80 (the state) set.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; its bench runs them as recorders).
struct FlyingUpdateEnv {
    u32(__cdecl *launch)(s32 index);                                                  // FlyingRide_Launch (0x55a720)
    void(__cdecl *playMotion)(s32 index, u32 slotIndex, u32 bank, u32 entry, u32 loop);  // FlyingRide_PlaySlotMotion (0x55aa80)
    u32(__cdecl *linkedElement)(s32 id, s32 index);                                   // Rides_GetLinkedElement (0x55a3e0)
    u32(__cdecl *setParam)(u32 sound, u32 parameter, u32 value);                      // Sound_SetParam (0x51bc40)
    u32(__cdecl *setPosition)(u32 sound, s32 x, s32 y, s32 z);                        // Sound_SetPosition (0x51c270)
    double(__cdecl *surfaceHeight)(s32 x, s32 y, u32 *slot);                          // Terrain_SurfaceHeight (0x452ae0)
    void(__cdecl *nextCorner)(s32 *slot);                                             // FlyingRide_NextCameraCorner (0x55c270)
    void(__cdecl *pickWaypoint)(u8 *slot);                                            // FlyingRide_PickWaypoint (0x55c350)
    void(__cdecl *clearSlot)(s32 index, u32 slotIndex);                               // FlyingRide_ClearSlot (0x55cda0)
};
// The calls go to env (nullptr: the ports).
void FlyingUpdate_SetEnv(const FlyingUpdateEnv *env);

extern "C" {
// 0x55abf0 (cdecl; name invented, unconfirmed): every record 1-99 in the table: one short of flyers (+0xc below +0) launches one, then each slot in use, in order:
// its position and angles (+4 to +0x18) kept at +0x1c to +0x30; a slot past the first with more flyers than wanted and over one flying gets +0xb4 set (going
// home); then by state, each with its motion and the sound's parameter (Sound_SetParam (+0xe0, Rides_GetLinkedElement (the record's +0x54, 10), value), the
// answer kept; most states bank 5, 0x1e, or bank 9, 0x3c when +0xb8):
// 1 parked, on the ground: +0x14 halved, 0x50; off when the record's +0x3c and +0xb8 are clear and it is not the only flyer or +0x38 is set: state 2, 20000 ahead
//   (of the record's +0x40 when +0x44, else of its heading) from home, the climb to 14000 at least and 5000 over home and the ground.
// 2 climbing: bank 4, 0xf; at the height, the record's +0x2c clear and state 4 (+0xb4) or 3 and the leg's start the square behind.
// 3 cruising: state 4 when its +0x84 runs out or +0xb4 (a waypoint, height 14000); a waypoint on reaching the last within 5000 (FlyingRide_PickWaypoint).
// 4 leaving the path: within 5000, state 8 (FlyingRide_NextCameraCorner); or 9 going home off the playfield by a random side.
// 8 circling the corners; 5 and 6 on the approach (one the record's +0x30 * 6 out along +0x40, then one +0x30 out, along the bearing +0xbc); 7 landing, state 1 on
//   reaching the home height (+0x7c and the climb zero, a positive step negated); 9 off the playfield: within 5000, state 0 and FlyingRide_ClearSlot.
// Then its speed +0x78 eased towards +0x7c (and the record's +0x50 tenths), moved along its heading; on the ground +0x14 falling by 50; in the air, each later slot
// not parked pushes it apart (slower, climb 0x28; closer still, 500 up and rolled by 1000), turns it towards the target (+0x70, +0x10), height and +0x14 by the climb
// to +0x50 (state 7: gliding to home by the distance), +0x18 rolled by the turn. The sound moved to the place / 300.
void __cdecl FlyingRides_Update();
}
