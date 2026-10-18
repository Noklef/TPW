// The game's command line: reimplementation of its parser (0x40f000).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x40f000 (cdecl (argc, argv); name unconfirmed): each argument after the first in turn. A switch ('/' or '-' then, any case,
// "v" or "version") stops it with 0; "quickload", "nodebug", "noload" and "SAVEDEBUG" are passed over, "flmouse" sets 0x78826c
// and "bwcursor" 0x788270 (to 1). Anything else (other switches too) is the park file: the first one whose end matches the
// extension at [0x7888c8] (any case) is copied to 0x788164 and 0x788160 set; a second, or one without the extension, is an error
// (GameError_Set(0x785de8, 2, 0, it)) that ends it. Returns 1 when there is no error (0x785de8 is 0), else 0.
u32 __cdecl CommandLine_Parse(s32 argc, char **argv);
}
