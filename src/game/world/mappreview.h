// The game mode's map preview (0x5234d0, cdecl (x, y, flags); name invented, unconfirmed): what the current game mode ([0x81ae2c]) shows for
// the map cell (x, y) under the pointer, before any click: the tile mouse modes' Assign (0x46d5a0) and GameMode_MapClick (0x524960, without
// [0x8187f8]) call it. Its calls outside it go through MapPreviewEnv (the game's by address), so that the bench (tests/fncmp/mappreview.py)
// can hand each a recorder.
//
// [0x8186b8] / [0x8186b4] = (x, y); 0x816d48, 0x816d50, 0x816d58, 0x816d54, 0x816d60, 0x816d5c and 0x816d4c cleared, Global871ee0_Clear,
// [0x818c10] = [0x818c14] = 0. A start is set when [0x81ede4] and [0x81ede8] are both not -1; "pulled" below: the end (x, y) moved onto the
// start's row (|dx| >= |dy|) and then onto its column (|dx| <= the new |dy|), kept at 0x8186b8 / 0x8186b4. "Mode" is the mode | 0x100
// (Global81ae2c_Get_52f840) at each call. Each mode:
// - 0: nothing (the end below, without the cell test).
// - 1 and 3: [0x818c10] 2 / 3; Map_SendLine (mode, start pulled to, or (x, y) to itself; the end written back to x, y).
// - 0x38: Map_SendLine as 1 without the pull, its code Global81af64_Get_52f850 | 0x100.
// - 4 (a path or queue piece): [0x8186d4] 1 while it runs. The definition of [0x8186e0] (ObjectDefs_Find on 0x7890a0, found again each
//   time) with a positive +0xc4 that the save lacks (GlobalSave_HasKey of [0x7cf83c] + 0x2c0, the low byte) costs 0 (Global81af4c_Set_52fb90);
//   else its +0x1b8, and unless the game is free ([0x785a2c]) a cost over the money (Source_Field0c of GameTable7cfb90_Get1da726_5195d0,
//   signed) sets 0x816d50 and 0x816d5c. [0x8186a4] / [0x8186a8] = (x, y), [0x81b604] = 0; with an id: [0x818c10] = the definition's +0x54,
//   and when [0x8187f8] and (x, y) is on the map (Map_InBounds) x and y go to the focus 0x7639a4 / 0x7639a8's parity (rounded down; into
//   0x8186a4 / a8 too) and, still on the map, a track cell there (its parent for 0xc / 0x11) records its direction's angle (1: 0, 4: 0x10e,
//   0x10: 0xb4, 0x40: 0x5a, another the angle Global81d7a4_Get_52f1f0 had; Global81d7a4_SetRecorded (angle, 0)); the angle then (0, 0x5a,
//   0xb4, 0x10e) moves 0x8186a4 / a8 by the definition's +0xa4 / +0xa8, +0xbc / +0xc0, +0xb4 / +0xb8 or +0xac / +0xb0 and the thing
//   [0x8186ac]'s facing (+0x10: 0x5a, 0xb4, 0x10e) by (-1, 0), (-1, +1) or (0, +1). Then 0x8186b8 / b4 = 0x8186a4 / a8 and the footprint
//   (PlaceObject_Footprint (0x8186a4, 0x8186a8, 4, the angle, 0, 1): 0 sets [0x81b604] = 1; then with 0 last), [0x874fc4] = 0.
// - 0x3b: as 4's end without the money test: [0x8186d4] 1, [0x81b604] 0, 0x8186a4 / a8 = (x, y), the cost the definition's +0x1b8, [0x818c10]
//   its +0x54 (with no id test), the footprint.
// - 0xb, 0x10 and 0x19: [0x818c10] 3; as 1 with Map_ApplyLine.
// - 0x39 (a hoarding area): x and y held to 1 .. [0x81d740] - 2 and 1 .. [0x81b1b0] - 3, then to 0x7639f0 / 0x7639f4's parity; with a start
//   0x816d50 = (HoardingGrid_CheckArea (start, end, 1) == 0) and the rectangle's four sides by Map_ApplyLine (mode): the top from the left + 2
//   to the right - 2 (the corners kept out when the sides differ), the right from the top down, the bottom likewise, the left from the bottom
//   up; without, Map_ApplyLine (mode, x, y to itself).
// - 0x3a: the cost -10000 (0xffffd8f1); with a start 0x816d50 = (Const1_424ec0 == 0) and the rectangle (insets 1) by Map_SendLine 0x13a;
//   without, Map_SendLine 0x13a to itself.
// - 0x36 (a patrol area): the cost -10000; with a start 0x816d50 = (HoardingGrid_AreaBlocked (the corners) == 0) and the rectangle (insets
//   1) by Map_SendLine (mode); without, Map_SendLine (mode) to itself.
// - 0xe and 0x13: FocusCell_TrackDrag (x, y, flags) unless both are 0.
// - 0x1a: with a focus cell ([0x818698]): x and y to the focus 0x7639a4 / a8's parity (and into 0x8186b8 / b4); with no track end
//   ([0x7639b0] 0xffff) Map_ApplyLine (mode, x, y to itself). Without [0x7639c0]: CellRun_Corners, GameMode_MapClick (start, flags, 0) and,
//   still in 0x1a, GameMode_SetOnly (0x19, 1). With it and a track end: [0x818684] = (x, y)'s id, [0x7639b4] its cell's MapCell_Word24; a cell
//   that MapCell_CanBuildTrack (mode, 0, 0) does not answer 9 for gives [0x818684] = [0x818694] and [0x7639b4] = [0x7639b0]; then
//   CellRun_Connect (from the higher of 0x7639b0 / 0x7639b4 (signed words): 0x818684's cell to 0x818694's, else back, 0).
// - 0x15: [0x818c10] 3, 0x816d50 = (CoasterPiece_CanPlaceAt (x, y, 0...) == 0), Map_SendLine 0x115 to itself; then over the coaster's
//   rectangle (Coasters_GetState: columns from a below c, rows from b below d, unsigned) each cell that CoasterPiece_CanPlaceAt allows whose
//   0x44-byte cell has no flag 0x40, is no track nor of a parent of type 0x19 (MapHandle_*), nor of types 9, 0xa, 3, 4, 7, 0x1e, nor of type 1
//   with flag 0x20, gets Map_SendLine 0x11f to itself (the column and row read back, the loops going on from them).
// - 0x16: [0x818c10] 3; with [0x818c54] 0x816d50 as 0x15's; [0x7639a0] -1 becomes (x, y)'s MapCell_Word40 (sign-extended); Map_SendLine
//   0x116 to itself; with [0x818c54] (again) 0x15's rectangle.
// - Others nothing.
// Then (except mode 0): a type 4 cell at the end (x, y), on the map, whose parent's (+0x10) first object (Cell_FirstObject) is a thing of kind
// 3 (+2 in its 0x7cfb90 slot) sets [0x818c14] to its definition's +0x54. Last GameCursor_Update and [0x818688] = (the start is the end).
#pragma once

#include "../../../re/types.h"

struct MapPreviewEnv {
    // cdecl
    u32(__cdecl *sendLine)(u32 code, s32 x, s32 y, s32 *xEnd, s32 *yEnd);                // Map_SendLine 0x536100
    u32(__cdecl *applyLine)(u32 arg, s32 x, s32 y, s32 *xEnd, s32 *yEnd);                // Map_ApplyLine 0x53aac0
    s32 *(__cdecl *footprint)(s32 x, s32 y, u32 unused, s32 angle, u32 place, u32 check);  // PlaceObject_Footprint 0x528a70
    void(__cdecl *setRecorded)(u32 value, u32 force);                                    // Global81d7a4_SetRecorded 0x52f1b0
    void(__cdecl *trackDrag)(s32 x, s32 y, u32 flags);                                   // FocusCell_TrackDrag 0x527960 (a third pushed)
    void(__cdecl *mapClick)(s32 x, s32 y, u32 flags, u32 useStored);                     // GameMode_MapClick 0x524960
    void(__cdecl *setOnly)(u32 mode, u32 record);                                        // GameMode_SetOnly 0x52f580
    void(__cdecl *corners)();                                                            // CellRun_Corners 0x530e80
    s32(__cdecl *connect)(u8 *from, u8 *to, u32 link);                                   // CellRun_Connect 0x52b400
    s32(__cdecl *checkArea)(s32 x1, s32 y1, s32 x2, s32 y2, u8 charge);                  // HoardingGrid_CheckArea 0x531a50
    u32(__cdecl *areaBlocked)(s32 x1, s32 y1, s32 x2, s32 y2);                           // HoardingGrid_AreaBlocked 0x532a50
    s32(__cdecl *canPlaceAt)(u32 x, u32 z, u8 *piece, u32 px, u32 pz, u32 nx, u32 nz);   // CoasterPiece_CanPlaceAt 0x434740
    void(__cdecl *cursorUpdate)();                                                       // GameCursor_Update 0x52f950
    // thiscall
    u32(__fastcall *canBuildTrack)(u8 *cell, void *, u32 kind, u32 alone, u32 direction);  // MapCell_CanBuildTrack 0x539760
    u32(__fastcall *hasKey)(u8 *save, void *, u32 key);  // GlobalSave_HasKey 0x4d4b70 (ECX [0x7cf83c] + 0x2c0; the id's low word pushed)
    u16 *(__fastcall *firstObject)(const u8 *cell, void *, u16 *out);                    // Cell_FirstObject 0x4d99e0
    u8 *(__fastcall *findDef)(u8 *defs, void *, u16 id);                                 // ObjectDefs_Find 0x412e90
    u32(__fastcall *table5195d0)(u8 *o);                                                 // GameTable7cfb90_Get1da726_5195d0
};
// nullptr restores the game's.
void MapPreview_SetEnv(const MapPreviewEnv *e);

extern "C" {
// 0x5234d0: see above.
void __cdecl GameMode_MapPreview(s32 x, s32 y, u32 flags);
}
