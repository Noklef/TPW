// Stepping a position by a direction (0, 90, 180 or 270 degrees): reimplementation of 0x52a3a0 and 0x52a420. The positions are in
// units of a fraction of a cell (a cell is 0x80 wide in the code around these).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x52a3a0 (cdecl; x, y, direction): moves *x and *y by a step for the direction: 0 by (+4, +1) giving 4, 90 by (+1, -5) giving 1,
// 180 by (-5, -2) giving 0x40 and 270 by (-2, +4) giving 0x10. Any other direction leaves them and returns the address of x (sic: the original
// reloads its first argument instead of the direction).
u32 __cdecl Position_StepA(s32 *x, s32 *y, s32 direction);

// 0x52a420 (cdecl; x, y, direction): the same with the steps 0 (-2, +1), 90 (+1, +1), 180 (+1, -2) and 270 (-2, -2) and the
// codes 4, 1, 0x40 and 0x10 (for any other direction the address of x, sic as above); returns Nibble_Scale of the code, in its low byte.
u32 __cdecl Position_StepB(s32 *x, s32 *y, s32 direction);

}  // extern "C"
