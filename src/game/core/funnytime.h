// The game's calendar clock ("funny time": game seconds run at mFunnySecsPerRealSec to the real one; saved with the world globals by
// WorldGlobals_Serialize 0x4d7a70): reimplementation of its serializer 0x4f7f30. It holds mFunnyTimeStart (a FILETIME at +0, 2000-01-01),
// mSessionStart (8 bytes at +8), mMonthAtLastUpdate (+0x10), mDayAtLastUpdate (+0x14) and mFunnySecsPerRealSec (+0x1c).
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {
// 0x4f7f30 (thiscall; stream, save, version, unused): 0 for a version outside 0..1, else the five fields saved or loaded as a serializer does
// (Save_Field; 1 at the first that fails). The load reads the last through Save_ReadU32, which takes the same steps.
u32 __fastcall GameClock_Serialize(u8 *clock, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
