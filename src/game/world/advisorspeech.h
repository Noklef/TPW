// The advisor's speech from frame to frame.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

// Its calls go by the game's addresses; a table of replacements (tests/fncmp/advisorupdatespeech.py gives them recorders).
void AdvisorSpeech_SetCalls(const AddrCall *table, u32 count);

struct Stream;

extern "C" {

// 0x599880 (cdecl): time without the game's pauses; with no speech (0xf7966c) the lip times are freed. A voice ended (or none) past the end time ends the speech. The sample starts at its time, the end's particles, sound and dialog update at theirs. The next queued animation plays once the head's layer is idle (0xc).
// The lip times switch the mouth open and shut, a random shape each 100 while open, drawn by flag 0x10 of the shapes' faces.
void __cdecl Advisor_UpdateSpeech();
// 0x598b20 (cdecl; the sound's length; returns the animations' length; name unconfirmed): random gestures fill the queue 0xf79718 (at most 0x13): first 0xe, then a dice of ten
// (rand) that fits, else the first of the lengths 0xf796c8.. that holds the rest (the tenth if none), each counted off by its length; 0xf and the ending length 0xf79700 last, 0 after.
u32 __cdecl Advisor_QueueRandom(s32 length);
// 0x598bf0 (cdecl; the table's index, the sound's length; name unconfirmed): the queue from the 0x50-byte row of 0x76dc18: a step of flag 0x400000 is a time (the next dword) at the
// length so far, stored to 0xf79778 and left out; a first step that is no 0xe clears 0xf79694; a row of no length at all and a sound queues at random.
u32 __cdecl Advisor_QueueAnim(s32 index, s32 length);
// 0x599050 (cdecl; a response's id; returns the time it takes, or 0 when none is said): 0x768fb8's row of the id (0x20 bytes, 0x270f ends it) says its sound, its lip file, its animations and its place. The lip
// times are read from "Data\Global" or the base folder + the name (malloc when over 1000 bytes or the static 0xf797d0 is taken), the sound played, the first animation called.
u32 __cdecl Advisor_Say(s32 id);
// 0x599d00 (cdecl; a stream): "ADV_", the block 0xf79660 of the size given, the lip times; the old times freed. A short read leaves the words as they were.
void __cdecl Advisor_Load(Stream *stream);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[28], gText_Advisor_Say_76e0d8, 0x0076e0d8, 0x1c);
GAME_DATA(char[12], gText_Advisor_Say_76e0cc, 0x0076e0cc, 0xc);

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x4], gText_Advisorspeech_74cfb8, 0x0074cfb8, 0x4);
GAME_DATA(u8[0x4c60], gAdvisorResponses, 0x00768fb8, 0x4c60);
GAME_DATA(u8[0x4b4], gAdvisor_QueueAnim_76dc18, 0x0076dc18, 0x4b4);
