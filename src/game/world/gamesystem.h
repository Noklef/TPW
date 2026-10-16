// The game system module's save and load (GSYS, 0x550520 / 0x5506e0, cdecl; stream): five globals as they are, then three of them as offsets from the
// one at 0x8786bc.
//
// The originals' quirks are kept: the stream's write and read always answer >= 0 (see Stream_Write), so neither ever fails; the save writes a sixth dword
// between the globals and the offsets that no variable holds (stack garbage in the original, 0 here), which the load reads and drops.
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {
// Writes 0x878c74, 0x87879c, 0x878a1c, 0x877d34 and 0x8786c0 (4 bytes each), the unset dword, then 0x878c74, 0x87879c and 0x878a1c each minus 0x8786bc.
// Returns 1.
u32 __cdecl GameSystem_Save(Stream *stream);
// Reads them back: the five globals, the dword it drops, then the three offsets added to 0x8786bc (the value read for it is not stored). Returns 1.
u32 __cdecl GameSystem_Load(Stream *stream);
}
