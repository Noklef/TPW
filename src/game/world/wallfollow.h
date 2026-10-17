// A walker that follows the edge of what blocks a straight walk between two cells (two of them, one turning each way, in the walk search
// round 0x511470; names unconfirmed): reimplementation of its step 0x511c90.
//
// The walker: +0 the steps recorded (a byte), +4 / +5 the column and row of the cell the walk is headed for, +6 the cells stepped to (words,
// column then row, at most 0xfc), +0x200 its direction (0 up, 1 right, 2 down, 3 left as in the step table 0x7622b0; 0x32 when it has come back
// onto the straight line, 0x33 when it has given up), +0x204 its turn (added to the direction to turn towards the edge, taken off to turn away),
// +0x208 / +0x209 its column and row, +0x20a / +0x20b where it left the straight line. The mode of the steps (MapCell_StepBlocked) is 0x7cf3c0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x511c90 (cdecl; walker): nothing once given up. Blocked ahead: turns away from the edge (direction less turn, low two bits) and records the
// cell. Else steps ahead and, when the turn towards the edge is open, turns (and records the cell); past 0xfb cells recorded it gives up. Then,
// unless the cell is past the target along the turned or the current direction, lies outside the box between where it left the line and the
// target, or is the cell where it left: direction 0x32. The first test passes the x argument's upper bytes from the mode (sic: they count past
// direction 3).
void __cdecl WallFollower_Step(u8 *walker);
}
