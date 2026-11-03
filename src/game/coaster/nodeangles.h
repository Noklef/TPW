// A place on a coaster's nodes: the turn to each node near it. Names invented (unconfirmed).
#pragma once

#include "../../../re/types.h"

// 0x438ab0 (cdecl; path, car, out1, out2, out3, unused, position, count): position's whole part is the node (path +0x9c, 0x38 bytes
// each, +0xa4 of them), its fraction t the way to the next; the place between the two by their +4 (+0x3c is the next's) and the heading
// by Angle_Lerp of their +0x10. Behind wanted = __ftol(car params' +0x28 / path +0xac) + 1, ahead wanted the same of +0x24; both as
// unsigned against what the node list gives. out1, out2, out3 get a row for each: zeros for the wanted rows behind that are not there,
// the rows behind, the node's own (0, 0, weight), the rows ahead, zeros for the wanted rows ahead that are not there. A row is the acos
// of two dots of the car's normalised +0x10 vector with the way to the node (flat, and turned by the node's heading), each clamped to
// 0..1 (a NaN to 1), each negated by its cross product's sign (the rows ahead negate out2 on a negative or NaN cross product, the rest
// on a positive one), and the node's weight: path params' +0x5c or +0x58 by bit 8 of this node and of the next, mixed by 1 - t and t.
// The vector's x, z and second y are negated for the rows behind. *count = behind wanted.
void __cdecl Coaster_NodeAngles(const u8 *path, const u8 *car, float *out1, float *out2, float *out3, u32 unused, float position, u32 *count);
