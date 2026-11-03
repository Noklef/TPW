// The ride instances: the table at 0x7a4610 (an instance per ride index, from 1), with its live count 0x7a4084, its holes count 0x7a3ebc
// and its first hole 0x7aedf4 (a slot, the index - 1). Reimplementations of 0x463c80 (Ride_Destroy; agent B's hub of the 0x463060
// closure) and 0x463060 (Ride_Create). Its calls into the unported 0x42a190 (the camera's following stopped) and into 0x436fa0 (CoasterLink_Destroy, coaster/registry:
// the ride's coaster link dropped, which calls Ride_Destroy back; in the game the port) go through an env.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What Ride_Destroy and Ride_Create call in the game, so that they can run outside it (the benches point these at recorders; Ride_Create's
// are the ports by default).
struct RidesEnv {
    void(__cdecl *stopFollowing)();     // 0x42a190
    void(__cdecl *forgetCoaster)(u32 ride);   // 0x436fa0
    u32(__cdecl *clone)(const u8 *instance, u32 heap, u32 flags, u8 **out, u32 mask);          // Instance_Clone (0x4557c0)
    s32(__cdecl *reset)(u8 *animator, s32 clear, s32 realTime, s32 keepNodes);                 // Animator_Reset (0x473e30)
    s32(__cdecl *hasMotion)(u8 *const *animator, u32 bank, u32 entry);                        // Animator_HasMotion (0x472d10)
    u32(__cdecl *play)(u8 *animator, u32 bank, u32 entry, u32 mode, float speed, u32 index);  // Animator_Play (0x4732a0)
    double(__cdecl *footprintHeight)(s32 x, s32 y, const u8 *cells, s32 width, s32 depth, u32 flags,
                                     s32 angle);                                               // Terrain_FootprintHeight (0x466d70)
    void(__cdecl *place)(u8 *animator, s32 x, s32 y, s32 cellX, s32 cellY, s32 angle, u32 height, u32 flags, s32 width,
                         s32 depth);                                                           // Animator_Place (0x467030)
    void(__cdecl *fitToGrid)(const u8 *object, const u8 *owner, u32 lift, u32 flags);          // Model_FitToGrid (0x469940)
    u32(__fastcall *blendTiles)(u8 *object, void *, s32 x0, s32 y0, s32 x1, s32 y1);          // ObjModel_BlendTerrainTiles (0x57ace0)
    void(__cdecl *transformBounds)(const u8 *source, u8 *model);                              // Model_TransformBounds (0x46ed80)
    void(__cdecl *fitGround)(const u8 *object);                                                // StripAnim_FitGround (0x454190)
    void(__cdecl *addToGrid)(u8 *holder, u32 from, u32 to);                                    // Holder_AddToGrid (0x45bcb0)
    void(__cdecl *wake)(u8 *holder);                                                           // Holder_Wake (0x45bd00)
    void(__cdecl *partStates)(u8 *instance);                                                   // PartStates_Update (0x44ab30)
    u32(__cdecl *createLink)(u32 typeId, u32 ride, u32 height, u32 a3, u32 loaded, u32 a5);    // CoasterLink_Create (0x4368f0)
    void(__cdecl *setPlaying)(u8 *animator);                                                   // Animator_SetPlaying (0x472f40)
    void(__cdecl *flash)(u8 *holder);                                                          // Highlight_Flash (0x469580)
    void(__cdecl *updateNormals)(u8 *model);                                                   // Model_UpdateNormals (0x473c30)
};
// Ride_Destroy's and Ride_Create's calls go to env (nullptr: the game's).
void Rides_SetEnv(const RidesEnv *env);

// 0x463060 (cdecl; name unconfirmed): a ride's instance made from the prototype of type (table 0x7ae6a8; type 0 or none: 0) and put in the
// ride table 0x7a4610, answering its ride index (0 when Instance_Clone does not answer 1). The prototype (with flags 0x400 its +0xd0 when
// set) is cloned (Instance_Clone (it, 0, flags 0x800 ? 0x231f : 0x211f, &instance, mask)); +4 loses 1, +0xe4 = flags, +0xd4 = d4. ride -1:
// the first hole's slot (0x7aedf4) takes it, and with no holes before the count's end (first hole == count 0x7a4084) both go up; else the
// first hole becomes the next empty slot up to holes (0x7a3ebc) + count (none: unchanged, sic), holes down, count up. Any other ride: that
// index. Flags 1 / 2 / 0x80: +4 | 4 / 0x4000 / 0x8000. Animator_Reset (1, 0, 0) and, with a motion (Animator_HasMotion (&instance, 0, 0)),
// bank 0 then 0xd played (entry 0, mode 0, 1.0f, layer 0). Flags 0x26: the description's x, y, angle and size (desc[0..3]); +0xe8 / +0xec
// = x, y; with a map (0x7a0854 -> +0x6c) Terrain_RotateFootprint (&x, &y, angle, &cx, &cy, +0xf0, +0xf4); the height Terrain_FootprintHeight
// (cx, cy, size, +0xf0, +0xf4, flags, angle) with a size and flags 8, else the map's height at x, y (+0x28, (width + 1) per row); then
// Animator_Place (cx, cy, x, y, angle, height, flags, +0xf0, +0xf4). Flags 8: Model_FitToGrid (+8, the map, height, 0), and with 0x7a1a8c
// 0x4000 and the object's (+8) byte flag 0x40 the prototype's footprint (+0 -> +0xcc: +8 columns, +0xa rows) is written into the map's
// cells (+0x2c, 4 bytes each) from the object's cell words (+0x18, two each, row by row) whose flag 0x80 is set: the low word (cell & 6)
// | 0x281 | 0x40 of the word | its bits 0x38 turned by the object's angle (+0x10), the high word copied; the cells from __ftol of the
// root's (+4 -> +0x78) +0x40 / +0x48 over the map's +0x10 / +0x14, less one by the angle, turned (any other angle: every cell at flags & 2,
// twice, sic); then ObjModel_BlendTerrainTiles over +0xc0 / +0xc4's words (its answer's Debug_Printf left out). Flags 2:
// Model_TransformBounds (+8 -> +4 into itself); 4: StripAnim_FitGround. A frame (+8 -> +0x2c): each triangle's normal (Vec3_Cross of
// v0 - v1 and v0 - v2, normalised, turned by the object's angle) below 0.999f in y (or NaN) flags the triangle (+1 | 0x80) and is copied
// into 0x7aecd4's buffer; with any, the object's +0x30 = the frame's +0x64 + 12 * +0x58. Flags 0x100: Holder_AddToGrid (+0xc0, +0xc4)
// when +4 has 0x4000, else Holder_Wake. PartStates_Update when the object has +0x28 and +4 lacks 0x20000000; flags 0x200 with the type's
// bit 18 (Table7ae6a8_Bit18): CoasterLink_Create (type, the ride, cx, cy, loaded, a5). The model (+8 -> +4) +0xb4 = the instance, the
// object's +0x1c's +0x140 = 0x463030 when there is one, Animator_SetPlaying / Highlight_Flash by +4's 0x1000000 / 0x2000000, the model's
// +0x44 entries (+0x70, 0xa0 bytes) | 0x10000, its +0x30 | 0x40000 and Model_UpdateNormals. Uninitialised: cx, cy and the height without
// flags 0x26 (the callers pass 0x26 whenever they pass 8 or 0x200); 0 here.
u32 __cdecl Ride_Create(u32 type, u32 flags, const u32 *desc, u32 mask, u32 d4, u32 a5, u32 a6, u32 a7, s32 ride, u32 loaded);

// 0x463c80 (cdecl (ride); name invented): nothing for ride 0 or an empty slot (the original's Debug_Printf (5, ..., ride) left out);
// else for its instance: Highlight_Clear when it is the highlighted one (0x7a1ce0), 0x7afda0 cleared when it is that, the env's
// stopFollowing when the camera follows the ride (Camera_IsFollowing (the index)), Highlight_Unflash with its flag 0x4000000 (+4), the
// env's forgetCoaster (ride), Instance_Dispose; its slot emptied. With no holes the count goes down, and unless it was the last slot the
// holes count becomes 1 and the first hole this slot (the last: the first hole goes down by one, sic). With holes: the last slot (holes +
// count - 1) drops the count and the trailing empty slots from the holes count (scanning down from the new end); any other slot is one
// hole more, the count one less, and the first hole when below it.
void __cdecl Ride_Destroy(u32 ride);

// 0x464580 (cdecl; name unconfirmed): a debug dump of a ride's slot. Unless the slot's +4 is 0xc, the original formats the slot's index in the system (by 0x38-byte records,
// from the system's +0x10), the system's word +0xe and name, and the slot's fields into a stack buffer with sprintf and hands each line to the empty Debug_Printf:
// nothing is kept. Left out here.
void __cdecl RideSystem_DumpSlot(const u8 *slot, const u8 *system);
}
