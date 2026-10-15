// A model's bounds put into its root's frame: reimplementation of 0x46ed80.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x46ed80: copies source's two corners (+0x80 and +0x8c, 3 floats each; the game passes the
// model itself) to the model's, then puts each through the matrix at its root (+0x78) + 0x10,
// applied as rows: x' = (m8 z + m4 y) + x m0 + m12, y' = (m9 z + m5 y) + m1 x + m13 and z' =
// (m10 z + m6 y) + m2 x + m14, the matrix read again for the second corner.
void __cdecl Model_TransformBounds(const u8 *source, u8 *model);
}
