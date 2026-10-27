// The game mode's map click (0x524960; names invented): what a click (or a drag's step) on the map at (x, y) does in the current game mode
// ([0x81ae2c]: Global81ae2c_Get_52f840). A dispatcher of 11.5 KB; its calls outside it go through MapClickEnv (the unported ones by address),
// so that the bench (tests/fncmp/mapclick.py) can hand each a recorder.
//
// The start: unless sound is off ([0x785a2c] 0) or [0x81b4cc] is set, [0x816d50] = 0 and with no [0x8187f8] the click goes to 0x5234d0
// (x, y, flags) as well. [0x8186d4] = 0; with useStored, (x, y) is the kept point ([0x8186b8], [0x8186b4]). Unless [0x816d48] or [0x81b4cc]
// is set (mode 0x16 excepted), the click is recorded on the action record 0x785a08: 0x14 (x, y) with flags' low word 0, else 0x2d (x, y,
// flags). In mode 0 [0x8186b0] = Wrap_Global7c2650_Get_481590(). Unless the mode is 0x38, with [0x816d48] set the click ends there (the
// sound 0xaf first while [0x8186d8]): modes 1, 3, 0xb, 0xe, 0x10, 0x13 and 0x36 with 0x52f200 (0, 0), mode 0x16 with 0x446050 and
// [0x816d48] = [0x818c54] = 0. Else FocusCell_Update (0), and nothing more in mode 0. Each mode then:
// - 1 and 3: (the drag's start and its end) the sound 0x65, [0x818c10] = 2 / 3; a first click (no start: [0x81ede4] and [0x81ede8] -1)
//   that the cell's record (+0x294 of the 0x44-byte one, 0x535670 (1 / 3, 0)) accepts keeps the start and the pair table's first pair
//   (Advisor 0xb8 in mode 1), [0x8186dc] = 0. A later one: the third tells the advisor 0xd6; the end is pulled onto the start's row or column
//   (the nearer one), and when it moved (or [0x820a8c] is 1) the pair table gets it and the line 0x536100 runs 0x87, the mode, 0x80, 0x82,
//   0x85, 0x86, 0x83 and 0x81 from the start ([0x8186c0] cleared). Mode 3 then calls 0x4de1f0 on the thing [0x8186ac] (the sound 0x8b
//   while [0x816d60]). A moved end with the last line answering non-zero becomes the start; otherwise the drag ends (the start -1, the steps
//   [0x763ac0], [0x763b00], [0x763b04] 0x80, 0x52f200 (0, 0); mode 3 with sound and no [0x816d60]: Advisor 0x151).
// - 4 and 0x3b: a path or queue piece from the thing's entrance; see the port.
// - 0xb and 0x10: a track drag as modes 1 / 3 with Map_ApplyLine (0x87, the mode, 0x80, 0x82, 0x83, 0x85, 0x86, 0x81), the sound 0xce;
//   a finished one sets the thing's run (0x52a970, 0x52c6f0, 0x52ccb0, TrackRun_AnimateA / B) and its entrance lines (0x536100 from
//   [0x7639b8] / [0x7639bc]).
// - 0xe and 0x13, 0x1a: a ride's entrance and exit (MapCell_Word24 heights, 0x539760, 0x52b400); 0x14: 0x530120 and 0x4de1f0 on the thing;
//   0x15 and 0x16: the lines 0x87, the mode or 0x15, 0x83 (0x84), 0x81 (Advisor 0xcd); 0x19: lines from the start; 0x32 - 0x35: the line
//   0x32 (and 0x86); 0x33: 0x527ee0; 0x36: a staff member's patrol region; 0x39: a hoarding area (HoardingGrid_CheckArea, Money_Debit);
//   0x3a: 0x52bdd0 over the drag.
// [0x8186d8] = 0 at every end.
#pragma once

#include "../../../re/types.h"

struct ActionRec;

struct MapClickEnv {
    // cdecl
    void(__cdecl *click5234d0)(s32 x, s32 y, u32 flags);                 // 0x5234d0
    void(__cdecl *f446050)();                                            // 0x446050
    void(__cdecl *f446300)(u32 a);                                       // 0x446300
    void(__cdecl *f435570)(u32 a);                                       // 0x435570
    void(__cdecl *f4419b0)(s32 x, s32 y, u32 a, u8 *at);                 // 0x4419b0
    u32(__cdecl *f445a70)(s32 x, s32 y);                                 // 0x445a70
    void(__cdecl *f527ee0)(s32 x, s32 y, u32 a);                         // 0x527ee0
    u32(__cdecl *f528a70)(s32 x, s32 y, u32 a, u32 b, u32 c, u32 d);     // 0x528a70
    u32(__cdecl *f52a970)(u32 a);                                        // 0x52a970
    u32(__cdecl *f52b400)(u8 *a, u8 *b, u32 c);                          // 0x52b400
    void(__cdecl *f52bdd0)(s32 x1, s32 y1, s32 x2, s32 y2);              // 0x52bdd0
    u8(__cdecl *f52c6f0)(u8 *thing, u32 a, u32 b, u32 c);                // 0x52c6f0
    void(__cdecl *f52ccb0)(u8 *thing, u32 a, u32 b, u32 c);              // 0x52ccb0
    void(__cdecl *f52f200)(u32 a, u32 b);                                // 0x52f200
    void(__cdecl *f52f580)(u32 a, u32 b);                                // 0x52f580
    void(__cdecl *f52fc80)(u32 a);                                       // 0x52fc80
    void(__cdecl *f530120)(u8 *thing);                                   // 0x530120
    void(__cdecl *f5327e0)(s32 x1, s32 y1, s32 x2, s32 y2);              // 0x5327e0
    u32(__cdecl *f536100)(u32 code, s32 x, s32 y, s32 *xEnd, s32 *yEnd); // 0x536100
    void(__cdecl *f545450)(u32 a);                                       // 0x545450
    u32(__cdecl *soundPlayAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);  // Sound_PlayAt 0x51bfc0
    u32(__cdecl *applyLine)(u32 arg, s32 x, s32 y, s32 *xEnd, s32 *yEnd);                // Map_ApplyLine 0x53aac0
    u32(__cdecl *pathLines)(s32 x, s32 y);                                               // MapCell_PathLines 0x523230
    void(__cdecl *pathNudge)(s32 *x, s32 *y);                                            // MapCell_PathNudge 0x523070
    void(__cdecl *focusUpdate)(u32 record);                                              // FocusCell_Update 0x5276d0
    u8 *(__cdecl *extendPath)(const s32 *point, s32 *nextX, s32 *nextY);                 // Cell_ExtendPath 0x52a050
    u8 *(__cdecl *entranceLines)(const u8 *thing, s32 *xOut, s32 *yOut, u32 kind);       // Map_ApplyEntranceLines 0x52a1a0
    u32(__cdecl *stepA)(s32 *x, s32 *y, s32 direction);                                  // Position_StepA 0x52a3a0
    u32(__cdecl *stepB)(s32 *x, s32 *y, s32 direction);                                  // Position_StepB 0x52a420
    u32(__cdecl *animateA)(const u8 *thing, u32 start);                                  // TrackRun_AnimateA 0x52a490
    u32(__cdecl *animateB)(const u8 *thing, u32 start);                                  // TrackRun_AnimateB 0x52a700
    void(__cdecl *traceA)(const u8 *thing);                                              // TrackRun_TraceA 0x530830
    void(__cdecl *corners)();                                                            // CellRun_Corners 0x530e80
    s32(__cdecl *checkArea)(s32 x1, s32 y1, s32 x2, s32 y2, u8 charge);                  // HoardingGrid_CheckArea 0x531a50
    void(__cdecl *fill)();                                                               // HoardingGrid_Fill 0x532960
    void(__cdecl *cursorSet)(u32 id);                                                    // GameCursor_Set 0x4a2aa0
    void(__cdecl *setRecorded)(u32 value, u32 force);                                    // Global81d7a4_SetRecorded 0x52f1b0
    u8(__cdecl *advise)(u32 id);  // the advisor told id: the original inlines Advisor_PostId's (0x486b00) message and post
    // thiscall
    void(__fastcall *f4de1f0)(u8 *thing);                                                // 0x4de1f0
    void(__fastcall *f4df390)(u8 *thing);                                                // 0x4df390
    u32(__fastcall *canOpen)(u8 *object);                                                // Object_CanOpen 0x4df290
    u8 *(__fastcall *cell44)(const u16 *handle);                                         // MapHandle_Cell44 0x4d0b10
    u16 *(__fastcall *firstObject)(const u8 *cell, void *, u16 *out);                    // Cell_FirstObject 0x4d99e0
    void(__fastcall *setPatrol)(u8 *staff, void *, u32 bl, u32 tr);                      // Staff_SetPatrolRegion 0x506e50
    u8 *(__fastcall *stepCell)(const u16 *cell, void *, u32 direction, s32 scale);       // Map_StepCell28InDirection 0x50c3e0
    void(__fastcall *addNeighbours)(u8 *cell, void *, u32 bits);                         // MapCell_AddNeighbours 0x522700
    void(__fastcall *setDirection)(u8 *cell, void *, u32 value);                         // MapCell_SetDirection 0x5227e0
    u32(__fastcall *isTypeBD)(const u8 *cell);                                           // MapCell_IsTypeBDOr10Or12Or19 0x53ad30
    u32(__fastcall *isTypeC)(const u8 *cell);                                            // MapCell_IsTypeCOr11 0x53ad90
    u32(__fastcall *isTypeD)(const u8 *cell);                                            // MapCell_IsTypeDOr12ViaParent 0x53adb0
    u16(__fastcall *word24)(const u8 *cell);                                             // MapCell_Word24 0x53aec0
    u32(__fastcall *f539760)(u8 *cell, void *, u32 kind, u32 a, u32 b);                  // 0x539760
    u32(__fastcall *f535670)(u8 *record, void *, u32 a, u32 b);                          // 0x535670
    void(__fastcall *writeBB)(ActionRec *r, void *, u32 code, u32 a, u32 b);             // ActionRec_WriteBB 0x403c20
    void(__fastcall *writeBBW)(ActionRec *r, void *, u32 code, u32 a, u32 b, u32 c);     // ActionRec_WriteBBW 0x403c80
    u32(__fastcall *table5195d0)(u8 *o);                                                 // GameTable7cfb90_Get1da726_5195d0
    void(__fastcall *moneyDebit)(u8 *money, void *, s32 amount);                         // Money_Debit 0x4d01f0
};
// nullptr restores the game's.
void MapClick_SetEnv(const MapClickEnv *e);

extern "C" {
// 0x524960 (cdecl; x, y, flags (the low word read), useStored): see above.
void __cdecl GameMode_MapClick(s32 x, s32 y, u32 flags, u32 useStored);
}
