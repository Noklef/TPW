// A saved game's stream, read and written: the action recorder, the level, then the game's parts in turn, each but those two behind a four-letter tag (the saver's are the dwords at 0x6fd8b8-0x6fd8f8).
// Reimplementation of 0x415270 and 0x4164c0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"
#include "../files/file.h"

// The parts' calls by address (the bench's recorders).
void GameFile_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x415270 (thiscall, this unused (stream, mode), pops 8): ActionRec_Serialize (stream, 0, 1, 0); mode 1 stops there with 1. World_Load, then by tag WRLD SpriteScripts_Load, SPSC Particles_Load and
// Particles_EndAll, PART MessageCentre_Load, MESS Clock_Load, CLOK VanillaTime_Load, VANT GameSystem_Load, GSYS RideSystem_Load, RSYS TrackRides_Load, TRAK FlyingRides_Load, FLYR RSSE_Load,
// RSSE Camera_Load, KAME Coasters_Load, COAS Advisor_Load, ADVS Sound_Load, SOUN CheatSystem_Load, CHTS AdvisorScoring_Load, ADSC UI_Load. Returns 0 at the first wrong tag or failed part (Serialize,
// WRLD, SPSC, PART, TRAK and FLYR fail on not 0, the others on 0; those of COAS, ADVS and ADSC can not fail), else 1. The tag goes into the mode's slot: a short read leaves the last one there (sic).
// The stream's position is asked for twice after each part (for the original's prints, left out). The cheats and the advisor's scoring are made at their first use, a destructor for each registered.
u32 __fastcall GameFile_Load(void *, void *, Stream *stream, u32 mode);

// 0x4164c0 (thiscall, this unused (stream, mode), pops 8): the same in writing: ActionRec_Serialize (stream, 1, 1, 0), mode 1, World_Save, then each tag written and the part's Save (with Particles_Save's
// second argument 0). The same parts fail in the same way, but for SpriteScripts_Save, which can not. A stream's write status is 0 or 1, never the negative the original tests.
u32 __fastcall GameFile_Save(void *, void *, Stream *stream, u32 mode);
}
