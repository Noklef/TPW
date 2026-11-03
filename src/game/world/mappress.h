// The game mode's map press (0x524790; name invented, unconfirmed; cdecl (x, y, flags, useStored)): what the button going down on the map at (x, y) does,
// the twin of GameMode_MapClick (src/game/world/mapclick). [0x8186d8] = 1 (that click's clears it). Unless sound is off ([0x785a2c] 0), [0x816d50] = 0
// and with no [0x8187f8] the press goes to 0x5234d0 (x, y, flags) as well. With useStored, (x, y) is the kept point ([0x8186b8], [0x8186b4]). With no
// [0x816d48] it is recorded on the action record 0x785a08: 0xf (x, y) with flags' low word 0, else 0x28 (x, y, flags). The rest only in mode 0x16 with no
// [0x816d48] (the original also tests modes 0x38 and 0): FocusCell_Update (0); with [0x818c54] the pair table's pair (x, y), the lines 0x536100 of 0x87,
// 0x15, 0x83 and 0x81 each from where the last ended, the end's cell +4 (0x28-byte records from 0x6cde of the table [0x7cf83c]) = CoasterEdit_PlaceSelected
// of that end, [0x7639a0] = -1 and [0x818c54] = 0; without it CoasterEdit_Click (1).
#pragma once

#include "../../../re/types.h"

struct ActionRec;

extern "C" void __cdecl GameMode_MapPress(s32 x, s32 y, u32 flags, u32 useStored);

// What it calls (the fncmp driver's recorders; the game's by default, the ports).
struct MapPressEnv {
    void(__cdecl *preview)(s32 x, s32 y, u32 flags);                     // GameMode_MapPreview 0x5234d0
    void(__fastcall *writeBB)(ActionRec *r, void *, u32 code, u32 a, u32 b);            // ActionRec_WriteBB 0x403c20
    void(__fastcall *writeBBW)(ActionRec *r, void *, u32 code, u32 a, u32 b, u32 c);    // ActionRec_WriteBBW 0x403c80
    void(__cdecl *focusUpdate)(u32 record);                              // FocusCell_Update 0x5276d0
    u32(__cdecl *pairAdd)(u32 a, u32 b);                                 // PairTable_Add 0x52fbf0
    u32(__cdecl *sendLine)(u32 message, s32 x0, s32 y0, const s32 *x1, const s32 *y1);  // Map_SendLine 0x536100
    u32(__cdecl *placeSelected)(s32 x, s32 y);                           // CoasterEdit_PlaceSelected 0x445a70
    u32(__cdecl *click)(u32 press);                                      // CoasterEdit_Click 0x446300
};
void MapPress_SetEnv(const MapPressEnv *env);   // nullptr: the game's
