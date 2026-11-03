// An object's tick, the two calls Thing_Update makes on a ride or shop. Reimplementation of 0x4e0b90 and 0x4e0e00.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct ObjectUpdateEnv {
    u8 *(__cdecl *ride)(s32 id);                                     // Rides_FindById
    u32(__cdecl *element)(const u8 *record, s32 index);              // Rides_GetElement
    void(__cdecl *setElement)(u8 *record, s32 index, u32 value);     // Rides_SetElement
    u16 *(__cdecl *makeHandle)(u16 *out, u32 x, u32 z);              // Cell_MakeHandle
    void(__fastcall *spreadRemove)(u8 *world, void *, u32 index, u32 cell);  // Map_SpreadRemove
    void(__fastcall *wearTick)(u8 *self);                            // Object_WearTick
    u32(__fastcall *visitorIs)(const u8 *visitor);                   // Visitor_IsState11To14
    void(__fastcall *beginCycle)(u8 *object);                        // Object_BeginCycle
    void(__fastcall *dismissHead)(u8 *object);                       // Object_DismissQueueHead
    void(__fastcall *arriveWaiting)(u8 *object);                     // Object_ArriveWaiting
};

void ObjectUpdate_SetEnv(const ObjectUpdateEnv *env);

// 0x4e0b90 (thiscall): with bit 7 of +0x32 and element 0 of the ride record (Rides_FindById of +0x24) at 1, spread record 7 taken back
// around the cell (bytes +5, +7) and the element set to 2. In the states +0x19c 3 and 4 nothing more. Otherwise, out of state 1, with
// +0x68 and every eighth tick ([0x80239c]+0x1da70c), Object_WearTick; then with either of the percentages +0x48, +0x44 truncating to 0
// (low byte), no bit 0 of +0x32 and element 4 at 0: +0x48 less 5 and clamped to 0-100 unless it truncated to 0, and element 4 set to 1.
// With bit 0 of +0x32 and +0x44 below 25, element 8 set to 1. Last, the handle +0x3c cleared unless its slot of 0x7cfb90 has a thing in
// the states 11-14.
void __fastcall Object_Update(u8 *object);

// 0x4e0e00 (thiscall): by the state +0x19c, 0 Object_BeginCycle; 1, 2 and 4 Object_DismissQueueHead and Object_ArriveWaiting.
void __fastcall Object_PostUpdate(u8 *object);
