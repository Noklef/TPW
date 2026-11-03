// The park entrance's hoardings and path laid when the game starts placing them: reimplementation of 0x5323f0.
#pragma once

#include "../../../re/types.h"

// What ParkEntrance_Lay calls (the ports by default; the bench runs them as recorders).
struct ParkEntranceEnv {
    void(__cdecl *setMode)(u32 mode, u32 record);                                // GameMode_Set (0x52f200)
    void(__cdecl *focusUpdate)(u32 record);                                      // FocusCell_Update (0x5276d0)
    void(__cdecl *mapClick)(s32 x, s32 y, u32 flags, u32 useStored);             // GameMode_MapClick (0x524960)
    void(__cdecl *mark)(u32 set);                                                // CellRun_Mark (0x531040)
    void(__cdecl *prune)();                                                      // TrackLine_Prune (0x5311c0)
    u32(__cdecl *lay)(s32 x, s32 y, s32 endX, s32 endY, u32 direction, u32 setDirection);  // CellRun_Lay (0x52ba50)
    void(__fastcall *pathModel)(u8 *cell);                                       // MapCell_ApplyPathModel (0x5365d0)
    void(__cdecl *shade)();                                                      // Terrain_ShadeNormals (0x450d10)
    void(__cdecl *cursor)(u32 id);                                               // GameCursor_Set (0x4a2aa0)
};
// The calls go to env (nullptr: the ports).
void ParkEntrance_SetEnv(const ParkEntranceEnv *env);

extern "C" {

// 0x5323f0 (cdecl, no arguments; name invented, unconfirmed). 0x81b4cc is set for its length. The rectangle comes from the two entrance places (Map_EntrancePos, a cell id less 1): 0x7639f8 the first's column less 4,
// 0x7639fc its row, 0x7639f0 the second's column plus 3, 0x7639f4 its row. 0x818c50 = 4, game mode 0x19, the focus updated. Then map clicks: with [0x79d86c]'s 0x400 at six fixed places around (0x7639f0, 0x7639f4),
// else at the offsets of the hoardings table from 0x784e3c (pairs of x, y; x 0x200 ends it, its 0x40 pairs all taken without one). The run marked, the track line pruned, the run unmarked; CellRun_Lay with direction 4,
// game mode 0. Each column from 0x7639f8 + 2 up to 0x7639f0 less 1 clears flag 0x40 and applies the path model of its cell in 0x7639f4's row and the next. 0x81b4cc is cleared and the terrain shaded.
void __cdecl ParkEntrance_Lay();

// 0x5327e0 (cdecl (x1, y1, x2, y2); name invented, unconfirmed). 0x81b4cc is set for its length. Game mode 0x19 (not recorded), map clicks round the rectangle from its lowest corner (five, back to the first), game
// mode 0, cursor 1; [0x7639c0] = CellRun_Lay of the four dwords at 0x7639f0 with direction 4; the run marked, the track line pruned, the run unmarked; the terrain shaded.
void __cdecl ParkEntrance_LayOutline(s32 x1, s32 y1, s32 x2, s32 y2);
}
