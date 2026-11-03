// The coaster editor's frame update: the adjust tools follow the mouse, the span and move tools follow the hovered car and tile.
// Reimplementation of 0x443340 (name invented, unconfirmed).
//
// The editor's flags are 0x79c658 (edittools.h): bits 0-5 the tool in use, 0x200 / 0x400 a value adjusted by the mouse's x / y
// (0x79c614 / 0x79c6fc pointing at it, 0x79c610 / 0x79c6f8 at a second one or none), 0x800 / 0x1000 a vector turned by them instead,
// 0x2000 the mouse's moves turned by the camera first, 0x80 the span tool, 0x100 the move tool (the piece 0x79c6a4).
#pragma once

#include "../../../re/types.h"

// What the update calls (the ports by default; its bench runs them as recorders).
struct EditUpdateEnv {
    u8 *(__cdecl *findCarOnRide)(u32 ride);                                    // CoasterLink_FindCarOnRide (0x443ee0)
    u32(__cdecl *clearSelectionFlags)();                                       // TrackSelection_ClearFlags (0x444040)
    void(__cdecl *setChanged)(u32 *flags, u32 also);                           // Flags_SetChanged (0x4346c0)
    void(__cdecl *setFirst)(u8 *piece, u32 keep);                              // TrackSelection_SetFirst (0x444090)
    u8 *(__cdecl *findAtFocus)(s32 *x, s32 *z, u32 *key);                      // CoasterLinks_FindAtFocus (0x433f70)
    void(__cdecl *dropCar)(u8 *owner, u8 *car, u8 *pylon, s32 x, s32 y, float *outX, float *outY, u32 unused,
                           u32 noShrink);                                      // Coaster_DropCar (0x442f40)
    void(__cdecl *moveCostChange)(const u8 *car, const float *cost, const float *extra, const float *x, const float *z, u32 a,
                                  u32 b);                                      // Car_SetMoveCostChange (0x443df0)
    s32(__cdecl *canPlaceAt)(u32 x, u32 z, u8 *piece, u32 px, u32 pz, u32 nx, u32 nz);   // CoasterPiece_CanPlaceAt (0x434740)
    void(__cdecl *carEdited)(u32 what, u8 *car, u8 *coaster, u32 unused, u32 keepContacts);   // Car_Edited (0x443f40)
    double(__cdecl *travelCost)(const u8 *ride, u32 withBase, const u8 *car, const float *x, const float *z, const float *fromX,
                                const float *fromZ);                           // Car_TravelCost (0x447830)
};
// The calls go to env (nullptr: the ports).
void EditUpdate_SetEnv(const EditUpdateEnv *env);

extern "C" {
// 0x443340 (cdecl; name invented, unconfirmed; the game's frame at 0x54fa8a calls it): only with a current link (0x790fec) whose +0x40 is
// set. The camera's turn h: 0x790a38 less the selected car's (0x79c640) +0xe0, or 0x790a38 without one.
// - With a tool (flags bits 0-5) and any of 0x1e00: the mouse's move dx = (0x79c630 - 0x790ac8) * 0x79c6ec, dy = (0x79c634 - 0x790acc)
//   * 0x79c6a8, turned by h (the sine table, gSinTable) with 0x2000. With 0x400 / 0x1000, unless both dx and dy (dy alone without
//   0x200 / 0x800) equal the last ones (0x79c668, 0x79c618; a NaN counts as equal): 0x400: *0x79c6fc = dy + 0x79c6d4 held to 0x79c6e4 ..
//   0x79c66c (held and without 0x2000: 0x79c634 = (held - 0x79c6d4) / 0x79c6a8 + 0x790acc), and *0x79c6f8 (if any) = dy + 0x79c6d0
//   held to 0x79c6e8 .. 0x79c670; 0x1000: the vector at 0x79c6fc (and at 0x79c6f8, if any) set to 0x79c6b0 (0x79c6c0) and turned about
//   y by minus its heading (Vec3_AngleY), about x by dy and about y back. 0x79c618 = dy and the car counts as edited. Then the same for
//   dx with 0x200 / 0x800 (0x79c614 and 0x79c610; 0x79c6f0, 0x79c61c .. 0x79c65c, 0x79c6ec, 0x790ac8 into 0x79c630; 0x79c6f4, 0x79c620 ..
//   0x79c654; the vectors 0x79c688 / 0x79c678 turned about y by dx), unless dx (and, with 0x1400, dy) is unchanged; 0x79c668 = dx.
// - With any of flags 0x13f: the focus tile 0x79c650 / 0x79c648 = __ftol of 0x7a1c70 / 0x7a1c78 times 0.1f. With 0x80 and a hovered
//   ride 0x7a1cd4 other than the last (0x79c6ac): its car (CoasterLink_FindCarOnRide; 0 for ride 0) and the span's first end 0x79c6dc
//   make the selection (TrackSelection_ClearFlags, 0x79c62c the one end, TrackSelection_SetFirst (the other, 1)): no car: (0x79c6dc,
//   none); the end itself: unless its definition (+0x100) +4 has flag 2 (then only TrackSelection_ClearFlags and Flags_SetChanged
//   (link, 0)), its neighbours (+0x68 next, +0x64 previous; with flags +8 0x40 and 0x80 the previous' previous, with 0x40 alone the next's
//   next; each the end when none); else the two with the lower index (+0) first, either 0 taken as the link's +0x58, the first stepped to
//   its next with flags 0x40 and 0x80, the second to its previous with 0x40 alone. With 0x100 (without 0x80 or the ride unchanged) and the
//   tile not the piece's (0x79c6a4 +0x100 +0xc / +0x10): the piece there (CoasterLinks_FindAtFocus; Flags_SetChanged (link, 0)) none or
//   of the piece's train (+0x18) and +0x14, its +0x30 below the train's +0xcc: Coaster_DropCar (link, piece, it, x, z, &a, &b, 1, 1),
//   Car_SetMoveCostChange (piece, 0x79c698, 0x79c64c, &a, &b, x, z), the piece and its next (+0x68) lose flag 0x4000 (+8) where
//   CoasterPiece_CanPlaceAt (x, z) answers, else gain it; then (also when the tile is the piece's) unless 0x785a2c the cost 0x79c69c (a
//   negative one plus 0xf1111112, sic) to Global81af4c_Add. Then (with or without 0x180) 0x79c6ac = 0x7a1cd4.
// - Edited and a selected car: Car_Edited (flags, car, link, 0, 0). With a tool, a selected car whose definition lacks flag 2 and 0x785a2c
//   clear: Global81af4c_Add (Float_RoundUp (Car_TravelCost (link, 0, car, 0, 0, 0, 0)) - __ftol (0x79c698), a negative one plus
//   0xf1111112). Bit 0 of 0x79c624: Flags_SetChanged (link, 0).
void __cdecl CoasterEdit_Update();
}
