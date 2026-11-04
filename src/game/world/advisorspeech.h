// The advisor's speech from frame to frame.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

// Its calls go by the game's addresses; a table of replacements (tests/fncmp/advisorupdatespeech.py gives them recorders).
void AdvisorSpeech_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x599880 (cdecl): time without the game's pauses; with no speech (0xf7966c) the lip times are freed. A voice ended (or none) past the end time ends the speech. The sample starts at its time, the end's particles, sound and dialog update at theirs. The next queued animation plays once the head's layer is idle (0xc).
// The lip times switch the mouth open and shut, a random shape each 100 while open, drawn by flag 0x10 of the shapes' faces.
void __cdecl Advisor_UpdateSpeech();
}
