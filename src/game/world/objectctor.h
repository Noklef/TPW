// The placed object's constructor (0x4db090): the 0x450-byte object over a thing of kind 3, its fields from its definition.
#pragma once

#include "../../../re/types.h"
#include "../core/filetime.h"

extern "C" {

// What the constructor calls (the benches point these at recorders): the ports, 0x4dcf90 by address (not ported).
struct ObjectCtorEnv {
    u8 *(__fastcall *thingConstruct)(u8 *thing, void *, u32 cell, u32 kind);                  // Thing_Construct 0x50afe0
    void(__fastcall *addObject)(u8 *world, void *, u8 *thing);                                // World_AddObject 0x519d80
    u8(__fastcall *addThing)(u8 *centre, void *, const u8 *thing, s32 slot);                  // MessageCentre_AddThing 0x40f9e0
    u32(__fastcall *post)(u8 *centre, void *, u8 *msg);                                       // MessageCentre_Post 0x40fb10
    void(__fastcall *spread)(u8 *world, void *, u32 index, u32 cell);                         // Map_SpreadAdd 0x4d8440
    u32(__fastcall *hasKey)(u8 *save, void *, u32 key);                                       // GlobalSave_HasKey 0x4d4b70
    u8(__fastcall *unlock)(u8 *save, void *, u32 id);                                         // ObjectDef_Unlock 0x4d4ad0
    u32(__fastcall *money)(u8 *world);                                                        // GameTable7cfb90_Get1da726_5195d0
    void(__fastcall *debit)(u8 *money, void *, s32 amount);                                   // Money_Debit 0x4d01f0
    void(__fastcall *setState)(u8 *object, void *, u32 state);                                // Object_SetState 0x4e0e60
    void(__fastcall *claimRide)(u8 *object, void *, u32 claim);                               // Object_ClaimRide 0x4dd500
    void(__fastcall *fn4dcf90)(u8 *object);                                                   // 0x4dcf90 (unported)
    u8 *(__cdecl *findRide)(s32 id);                                                          // Rides_FindById 0x55a070
    void(__cdecl *setWordC0)(u8 *record, u32 value);                                          // Rides_SetWordC0 0x55a300
    void(__fastcall *setCapacity)(u8 *object, void *, u32 capacity);                          // Object_SetCapacity 0x4dd7f0
    void(__cdecl *setElement)(u8 *record, s32 index, u32 value);                              // Rides_SetElement 0x55a0b0
    FileTime *(__fastcall *now)(const u8 *clock, void *, FileTime *out, u32 *year, u32 *month, u32 *day, u32 *dayOfWeek, u32 *hour, u32 *minute,
                                u32 *second, u32 *ms);                                        // FunnyTime_Now 0x4f8690
    u8 *(__fastcall *findControls)(u8 *table, void *, u32 id, u32 quiet);                     // ObjectControls_Find 0x4d3d10
    void(__fastcall *getName)(u8 *defs, void *, u32 id, u16 *out, u32 size);                  // ObjectDefs_GetName 0x413020
    void(__fastcall *getSuffix)(u8 *defs, void *, u32 id, u16 *out, u32 size);                // ObjectDefs_GetSuffix 0x4130b0
    u32(__fastcall *analyser)(u8 *world);                                                     // GameTable7cfb90_Get1da720_519510
    void(__fastcall *stampTime)(u8 *analyser);                                                // ParkAnalyser_StampTime21c30 0x4c7680
    void(__cdecl *startGrow)(u8 *object, u32 frame);                                          // StripAnim_StartGrow 0x454550
};
// The constructor's calls go to env (nullptr: the defaults).
void ObjectCtor_SetEnv(const ObjectCtorEnv *env);

// 0x4db090 (thiscall (cell, id, angle, ride), pops 0x10; name invented, unconfirmed): the object constructed: Thing_Construct (cell, 3), the
// id word +0xe, the angle +0x10, the ride +0x28, its fields zero and its six timers (+0xec, +0x174, +0x224, +0x2ac, +0x334, +0x3bc: -1, 30, 0),
// added to the world and to the message centre (made if there is none) at slot 0xb, a message 06 with its handle posted. From its
// definition (ObjectDefs_Find): the cells +0x34 (the cell plus the pair +0x4b0 turned by the angle + 0xb4), +0x36 and +0x38 (+0x34 plus the
// pairs +0x494 and +0x4a0 turned by the angle), +0x188 - +0x198, the flags +0x32 / +0x33 (+0xf4 and +0xf0 and +0x110 also spread on the map at
// the object's cell word + 1). A definition with +0xc4 > 0 and no key unlocks, else the price +0x1b8 is debited; the state 0 (flag 4) or 3; the
// ride claim dropped; 0x4dcf90. +0x1a8 > 0: the ride's word +0xc0 and +0x58; +0x198 > 0: the capacity; +0x1a0 > 0: the ride's element 3 and
// byte +0x5c, its low byte clamped to +0x12c and +0x130. The start time +0x18, the controls' record +0x18 counted (+0x1c from the world's
// +0x1da70c if none), the name +0x3c8 and suffix +0x40a. Flag 8: +0x68 and +0x6c zero, the ride's element 6 set, the strip [0x7a4610 + 4 * +0x20]
// grows. Returns the object. (The original's id, and the cell given to Map_SpreadAdd, keep the arguments' high halves. Its copy of the
// definition's +0x14 text goes unused but is read. Debug_Printf left out.)
u8 *__fastcall Object_Construct(u8 *object, void *, u32 cell, u32 id, s32 angle, u32 ride);
}
