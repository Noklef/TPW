// The video library's DirectDraw error names (for its OutputDebugString reports): the static table 0x705ee8-0x7061c7 of (code, name)
// pairs, DDERR_ names. Reimplementation of 0x672e30. The name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x672e30 (fastcall (code); name unconfirmed): the name paired with code in the table, searched from its start, else 0x77e528 (the
// "Unknown E..." string).
const char *__fastcall DdErrors_Name(u32 code);
}
