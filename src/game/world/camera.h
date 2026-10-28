// The camera's globals (module KAME): their static initialisers 0x42a0f0-0x42a182, the
// follow test 0x42a410 and the small helpers around them.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x42a0f0 / 0x42a110: 0x7909e8 = 1.0f, 0x790a94 = 1.0f / 8388608.0f (0x6fdd54 / 0x6fdd10);
// nothing reads either.
void __cdecl Camera_InitUnusedOne();
void __cdecl Camera_InitUnusedEpsilon();

// 0x42a130: 0x7909ec = 110.0f (0x6fdd40; stepped by 0x42a8xx). 0x42a140: the yaw 0x790a38 (set
// to multiples of pi / 2 by 0x42a2xx) = 0.0f; 0x42a150: 0x7909e0 = 0.0f, both from 0x6fdd44.
// 0x42a160: the point at 0x7908f0 = (475, 0, 175) (0x6fdd48).
void __cdecl Camera_Init7909ec();
void __cdecl Camera_InitYaw();
void __cdecl Camera_Init7909e0();
void __cdecl Camera_InitPoint();

// 0x42a410: 1 when the camera follows thing: gCameraFlags (0x790ab0) bit 4, 0x790988 set and
// the followed thing (0x790a44) non-null and thing; else 0.
u32 __cdecl Camera_IsFollowing(const void *thing);

// 0x42b070: 1 when two matrices (4x4 floats) are within 0x74c980 (1e-5) of each other: the
// sum of |a - b| over the first three columns of each row is less (or NaN). The sum runs in
// the order the original's schedule gives, with its running total stored to a float after
// every second term (from the third on), so overflow matches.
u32 __cdecl Camera_MatricesClose(const float *a, const float *b);

// 0x42a4e0: the followed thing (0x790a44) when the camera follows one (flag 4, 0x790988);
// else null.
const void *__cdecl Camera_Followed();

// 0x42a720: the face of part *part of the instance's model ((*instance)+4): +0x70 (0xa0 each)
// below the model's +0x44 count, else +0x74 (0x58 each); the index is offset by +0x46.
u8 *__cdecl Camera_PartFace(u8 *const *instance, const u32 *part);

// 0x42aab0: clears 0x790a88-0x790a90, then sets each given value: the point's x and z
// (0x7908f0 / 0x7908f8; z is read whenever x is given), 0x7909ec, the yaw 0x790a38 and
// 0x74c9bc.
void __cdecl Camera_SetView(const u32 *x, const u32 *z, const u32 *zoom, const u32 *yaw, const u32 *other);

// 0x4868d0 (cdecl; &cell; name unconfirmed): Camera_SetView of the point only: the cell id less 1 (a u16 widened, so 0 is 0xffffffff) split
// into x (its low 7 bits) and z (the rest), each times 10 (wrapping) as a float.
void __cdecl Camera_CentreOnCell(const u16 *cell);

// 0x4867b0 (cdecl; handle; name unconfirmed): for a thing's handle (its low word; 0 nothing) whose slot (0x7cfb90) holds a thing, the camera
// centred (as Camera_CentreOnCell) on the cell id of its cell bytes +5 (x) and +7 (z): z * 128 + x + 1 as a word. (Its debug check of the
// thing table left out.)
void __cdecl Camera_CentreOnThing(u32 handle);

// 0x486860 (cdecl; sprite; name unconfirmed): for a sprite (0 nothing), the camera's point (Camera_SetView, x and z only) put where the
// sprite is drawn this moment: Sprite_InterpolatedPos at t = (the time [0x8786bc] less the last tick's [0x878a1c], an int) times the float
// [0x6ff16c] (1 / 248) on the x87, the second position (1).
void __cdecl Camera_CentreOnSprite(const u8 *sprite);

// 0x42afd0: sets (on) or clears gCameraFlags bit 0x80.
void __cdecl Camera_SetFlag80(u32 on);

// 0x42a500 / 0x42a520 (cdecl; key map action handlers, names unconfirmed): bit 0x100 of the camera's control flags 0x790aac set / bit
// 0x200 toggled. Return 1.
u32 __cdecl Camera_SetControl100();
u32 __cdecl Camera_ToggleControl200();

// 0x42cd40: clamps a cell to [0, 0x7a0314 - 1] x [0, 0x7a0374 - 1] (signed; the lower bound
// first).
void __cdecl Camera_ClampCell(s32 *x, s32 *z);

// 0x42cd80 (cdecl; position, x, z; name unconfirmed): the cell of a position: *x = __ftol(position[0] 0.1) and *z = __ftol(position[2] 0.1),
// each when given.
void __cdecl Camera_CellOf(const float *position, s32 *x, s32 *z);

// 0x4bd020 (cdecl; x, z as float bits, outX, outZ; name unconfirmed): each coordinate in tenths (c 0.1) as two bytes: [1] = its __ftol,
// [0] = __ftol((c 0.1 - that low byte) 256); x's bytes stored before z's.
void __cdecl Camera_PackCellFractions(u32 x, u32 z, u8 *outX, u8 *outZ);

// 0x48ce80 (cdecl; name unconfirmed): the camera centred on the thing [0x7c2658] points to (Camera_CentreOnThing with its handle).
void __cdecl Camera_CentreOnGlobalThing();

// 0x42a1d0 (cdecl; name unconfirmed): the cell under the camera's point (x, z = __ftol(0x7908f0 / 0x7908f8 * 0.1), each held in [0, 0x7a0314 - 1] /
// [0, 0x7a0374 - 1], below 0 first); for the ride object there (Cell_FindRideableObject) the point moves to the centre of the object's cell (its
// u16 at +0x38, less 1 as a dword: x = 5 - (low 7 bits) * -10, z = 5 - (the rest) * -10) and, when that cell's direction (MapCell_Direction of the
// world [[0x7cf6ec]]'s cell) has exactly one of the bits 1 / 4 / 0x10 / 0x40 among 0x55, the yaw (0x790a38) = pi, pi / 2, 0 or 3 pi / 2.
void __cdecl Camera_SnapToRide();

// 0x42a990 (cdecl; name unconfirmed): the camera back to its defaults: 0x7909ec, the yaw 0x790a38 and the point 0x7908f0 from 0x6fdd40-0x6fdd50,
// gCameraFlags 0x790ab0 cleared; the matrices 0x790990, 0x790a48 and 0x790948 the identity; 0x7909c0 a copy of the point (y and z through the
// x87: a signalling NaN comes out quiet); [0x87afe8] assigned 0x790990 (Mat4_Assign); the field of view [0x74c9b8] [0x6fdd80] (x87, stored as
// a float) to the perspective 0x87b110 (Mat4_Perspective 0.1, 1000) and the view quad [0x879d54] (ViewQuad_SetFov); 0x790aac and 0x7909d0 =
// 0x790aac's bit 0x200 | 0x80; 0x7909e4 the game clock's real time (Clocks_RealNow); 0x790a88-0x790a90 and gCameraFlags 0, 0x7908e8 1.
void __cdecl Camera_Reset();

// 0x42aff0 (cdecl; name unconfirmed): the yaw 0x790a38 snapped to the nearest quarter turn: r = fmod(yaw, pi / 2) (_CIfmod); for r below
// -pi / 4 (or a NaN) yaw - (r + pi / 2), for r above pi / 4 (pi / 2 - r) + yaw, else yaw - r (x87 at the game's precision, stored as a float).
void __cdecl Camera_SnapYaw();

// 0x42a760 (cdecl; message, param; name unconfirmed): the camera's mouse input. y = (param >> 16) * the screen height ([0xfaa5c4], a
// signed word) / 2048 and x = (param & 0xffff) * the width ([0xfaa5c0]) / 1536 (truncated). 0x1e: with 0x790aac bit 2, the yaw 0x790a38
// += ((0x790ac8 * 2048 / height) - 1024) * [0x74c9c0]. 0x20: 0x790aac loses bits 0x78; 0x22: loses 0x7f and gCameraFlags (0x790ab0) gets
// 0x100. 0x10001: gCameraFlags loses 0x100, then as 0x10003: the point 0x790ac8 = y, 0x790acc = x. 0x10002: gCameraFlags gets 0x100.
// 0x10004 / 0x10005: button param 0, 1 or 2 clears / sets 0x790aac bit 1, 4 or 2. 0x1000d: 0x7909ec less -10 for a negative param,
// else less 10. Others nothing.
void __cdecl Camera_Input(s32 message, u32 param);

// What the camera's mode change calls in the game, so that it can run outside it (the benches point these at recorders; the ports by
// default).
struct CameraEnv {
    void(__cdecl *suspendHighlight)(u32 suspend);              // Highlight_Suspend (0x45d660)
    void(__fastcall *setChat)(void *buttons, void *, u32 on);  // ChatButtons_SetChat (0x481470, thiscall on 0x7b51f0)
    void(__cdecl *leaveProcedural)();                          // Terrain_ProceduralLeave (0x457a40)
    void(__cdecl *enterProcedural)();                          // Terrain_ProceduralEnter (0x457a00)
    void(__cdecl *addToGrid)(u8 *holder, u32 from, u32 to);    // Holder_AddToGrid (0x45bcb0)
    void(__cdecl *wake)(u8 *holder);                           // Holder_Wake (0x45bd00)
};
// The camera's calls go to env (nullptr: the game's).
void Camera_SetEnv(const CameraEnv *env);

// 0x42ab20 (cdecl; mode, toggle, ride, force, out, fresh): gCameraFlags (0x790ab0) loses 0x800; was = its 0x16 (a camera mode on); *out =
// 0 when given. Mode 0x40 (the park view): the low byte keeps only 0x41; changed, and leaving when was. Mode 4 (follow) unless the flags
// have 0x92 and not force: the low byte keeps 0x45, bit 4 cleared when toggle and set, else set; set: was gives 0x400, else entering
// (fresh: 0x800 and 0x790ae0 / 0x790adc cleared); changed either way. Cleared and was: with 0x400 it becomes 2 (*out = 1), else leaving.
// Mode 2 (walk) unless 0x84 and not force: the low byte keeps 0x43, bit 2 as bit 4 in mode 4; set: changed, entering unless was;
// cleared: leaving when was. Entering: Highlight_Suspend (1), ChatButtons_SetChat (1), with procedural terrain on (Terrain_ProceduralOn)
// flag 0x200 and Terrain_ProceduralLeave, and the instance at 0x790a34 back into the grid (Holder_AddToGrid over +0xc0 / +0xc4 with +4's
// 0x4000, else Holder_Wake). Leaving: Highlight_Suspend (0), ChatButtons_SetChat (0) and with flag 0x200 it cleared and
// Terrain_ProceduralEnter. Then every ride's (0x7a4610, 1 to holes + count) model parts (+8 -> +4: +0x48 of them from +0x46, parts +0x7c
// 0x14 bytes each; faces +0x70 0xa0 bytes below +0x44, then +0x74 0x58 bytes): face bit 0x8000 set by the part's 0x4000000, else
// cleared by its 0x8000000 while the flags' byte has 0x16, else cleared. With a ride and flag 4 its parts likewise by 0x10000000 /
// 0x20000000. The flags lose 0x1000. Answers changed.
u32 __cdecl Camera_SetMode(u32 mode, u32 toggle, u32 ride, u32 force, u32 *out, u32 fresh);
}
