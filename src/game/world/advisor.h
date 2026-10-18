// The advisor's responses: reimplementation of the lookup 0x5997b0 in the response table at 0x768fb8 (entries of 0x20 bytes, ended by
// the id 0x270f), which Advisor_Say (0x599050, "Advisor says ResponseID %d") also walks.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5997b0 (cdecl; id): the table of 0x20-byte entries at 0x768fb8, ended by an id of 0x270f, searched
// for the id; the dword at +0x1c of the entry found, or 0x17f for none.
u32 __cdecl Table768fb8_Lookup(u32 id);
}
