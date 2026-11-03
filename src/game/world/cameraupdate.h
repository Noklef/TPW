// The camera's step each frame (0x42b1c0). x87 in the original's order.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What it calls in the game, so that it can run outside it (tests/fncmp/cameraupdate.py points these at recorders; the ports by default).
struct CameraUpdateEnv {
    u32(__fastcall *now)(u8 *clocks);                                    // Clocks_RealNow (0x402d80)
    u16(__fastcall *isHeld)(const u8 *map, void *, u32 action);          // KeyMap_IsHeld (0x40ca20)
    u32(__cdecl *stepBlocked)(u32 x, u32 z, u32 dir, s32 mode);          // MapCell_StepBlocked (0x4d8750)
    u8 *(__cdecl *findRideable)(s32 x, s32 z);                           // Cell_FindRideableObject (0x42a340)
    u8 *(__fastcall *findDef)(u8 *defs, void *, u32 id);                 // ObjectDefs_Find (0x412e90); id as pushed, a word over a stale one
    void(__fastcall *follow)(const u8 *object, void *, u32 arg);         // Object_FollowCamera (0x4e15b0)
    double(__cdecl *height)(u32 x, u32 z, u32 any);                      // Terrain_GridHeightAt (0x452560); x and z as float bits
    u32(__cdecl *matricesClose)(const float *a, const float *b);         // Camera_MatricesClose (0x42b070)
};
// Its calls go to env (nullptr: the game's).
void CameraUpdate_SetEnv(const CameraUpdateEnv *env);

// 0x42b1c0 (cdecl; name invented, unconfirmed): with gCameraFlags (0x790ab0) bit 0x80 only: the view [0x87afe8] assigned 0x7909f0, the current matrix (0x790a98) that one. Else:
// - The time: with 0x7908e8 set it is cleared, the time step is 1 and (with 0x78d912 set, a turn-by-steps game) the yaw 0x790a38 snaps to its quarter turn; else now + 1 less the last
//   time 0x7909e4, 100 past 200, 1 at or below 0.
// - The keys: 0x790aac keeps bits 0x87 and takes 8, 0x10, 0x20 and 0x40 from the actions (6 or 0x12), (7 or 0x13), (0 or 0x15), (1 or 0x16); 0x100 resets zoom, yaw and point (and loses
//   flag 0x1000). In a camera mode (flags 0x16, not 0x3c) they turn the yaw and step the tilt; out of one they turn it and zoom (0x7909ec), by whole quarter turns when 0x78d912 is set
//   on a key's first frame (0x7909d0 the keys before).
// - The speed 0x790a88-0x790a90: with nothing pressing it decays; else from the mouse's offset (0x790ac8 / 0x790acc less 0x7a1c54 / 0x7a1c58) past a dead zone or the arrow actions 0xd-0x10,
//   through the yaw's sine and cosine, scaled by the square root of the zoom; at the map's edges (the view [0x7a0854] +0x6c: cells times size) it turns back. Below 1e-4 on all three it stops.
// - The point 0x7908f0 moves by it: out of a walking mode directly; walking, one cell at a time, stopped at blocked steps (MapCell_StepBlocked mode 2) and following any ride
//   object it enters (Cell_FindRideableObject: Object_FollowCamera (1)), with its height 0.
// - The zoom held to 40-300 (0x790aac bit 0x200) or 70-130 when 0x80 is set. The eye 0x790aa0-0x790aa8 from the point, the tilt 0x790a3c, the ground's height with a smoothed bob (0x790ae4) and
//   the lean 0x7908e0 / 0x7908e4 from the speed; the matrix 0x790990 from them (Matrix_FromPitchYaw, Matrix_RotateEuler order 9), in a following mode blended from the followed part's
//   (0x790a30, by the time step) into 0x790948. When it did not change (Camera_MatricesClose against 0x790a48) yaw and angle wrap to 2 pi, else the view [0x87afe8] takes it; 0x7909e4 = now.
// Last, 0x790900 is the current matrix inverted (Matrix_Invert). Quirks kept: the height's arguments and the definition's id can be stale frame slots (see the source).
void __cdecl Camera_Update();
}
