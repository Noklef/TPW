// The build's authorization stamp, as the start-up's debug line prints it (0x54dd3a). Reimplementation of 0x5d55a0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d55a0 (cdecl; name unconfirmed): the stamp. When the phrase after the first byte at 0x763f3c ("The Quick Brown Fox...") is still
// the one at 0x774bd8, the text 0x774bc0 ("No Authorization Stamp"). Else the bytes of the phrase less those of the key at 0x774b78
// are written to 0xf827f8 on, from the first, until a phrase byte equals its key byte (none written when the first two do; no
// terminator), and 0xf827f8 is returned.
u8 *__cdecl AuthStamp_Text();

}  // extern "C"
