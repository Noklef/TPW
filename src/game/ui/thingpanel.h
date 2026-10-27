// The info panel a thing is shown in (0x486920; names invented): which of the panels' Show (slot 10 of their vtables) a thing goes to.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x486920 (cdecl (thing)): nothing for none. A thing of kind (+2) 3 by its definition's (Object_GetDef) kind +0x4ac: 0 the panel
// 0x7cc3e0 when the definition's +0x288 is 2, else with a linked thing (the word +0x30, Fn_4e0df0) the panel 0x7cc218 for that thing of
// the table 0x7cfb90 (Debug_Printf when the table is empty, left out), else the panel 0x7cc218 for this one when the definition's word
// +0x284 (Object_DefWord284) is 0; 1 the panel 0x7cc258; 2 0x7cc298; 3 by the word +0x32: bit 0 0x7ca178, bit 1 0x7cc328, else 0x7ca228;
// other kinds nothing. Kinds 4-8 the panel 0x7cc3b0, kind 1 0x7cc408, others nothing.
void __cdecl ThingPanel_Show(u8 *thing);
}
