// An object's ride, claimed or made anew. Reimplementation of 0x4dd500; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct ObjectRideEnv {
    u8 *(__fastcall *find)(u8 *defs, void *, u16 id);                                                                    // ObjectDefs_Find
    u32(__cdecl *claim)(u32 index, u32 type, u32 key, const u32 *desc, u32, u32 value, u32 a, u32 b, u32 ninth);         // Ride_Claim (its ninth unread)
    u32(__cdecl *create)(u32 type, u32 key, const u32 *desc, u32 flag, u32 id, u32 value, u32 a, u32 b, s32 place, u32); // 0x463060
    void(__cdecl *setRecordId)(u32 index, u32 value);                                                                    // Ride_SetRecordId
    void(__cdecl *requeue)(s32 id);                                                                                      // Coaster_RequeueWaitingPeeps
};
void ObjectRide_SetEnv(const ObjectRideEnv *env);

extern "C" {

// 0x4dd500 (thiscall (claim)): nothing without the definition of its word +0xe. The description: (def +0x20 or else [0x785594], def +0x24 or else [0x785598], object +0x10, def +0x18) with a def +0x38; without it
// ((cell +0x34 - 1) & 0x7f, (cell - 1) >> 7 as a dword, object +0x10, def +0x18). With a def +0x38 the key 0x361, the arguments def +0x28, +0x2c; without it the key 0x32f and 0, 0. `claim` not 0: Ride_Claim of the ride +0x20 (no +0x24 as an argument but the ninth with def +0x38), and
// without a def +0x38 Ride_SetRecordId (+0x20, +0x24) and Coaster_RequeueWaitingPeeps (+0x20). `claim` 0: +0x20 = the answer of 0x463060.
void __fastcall Object_ClaimRide(u8 *object, void *, u32 claim);
}
