// The sound play commands: a stack command (vtable 0x700b90; +4 the category, +8 the sound, +0xc/+0x10/+0x14 the position as floats,
// +0x18 the range) posted to the sound manager [0x802bcc] (its vtable +8: (owner, &command), whose result is returned) when sound is on
// (0x802bc8) and started (0x802bd4). The interface plays its sounds in the category gSndCatUi [0x803a2c] at the origin. Reimplementation
// of 0x51bfc0, 0x51c130, 0x485aa0 and 0x486ae0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x51bfc0 (cdecl (owner, category, sound, x, y, z)): 0 when sound is off or not started or the sound is 0; else the command (the
// position fild / fstp as floats, the range 0) posted, the manager's result returned.
u32 __cdecl Sound_PlayAt(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);

// 0x51c130 (cdecl (owner, category, sound, x, y, z, range)): Sound_PlayAt with the range (an int too) as a float at +0x18.
u32 __cdecl Sound_PlayAtRange(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z, s32 range);

// 0x51e730 (cdecl; called by 0x54ec9f; name invented): the music [0x803aa0] = Sound_PlayAt([0x803aa0] as the owner, cat_music
// [0x803a44], 2, 0, 0, 0), then [0x803aa0] = Sound_SetParam(it, 4, 0). EAX not kept.
void __cdecl Sound_StartMusic();

// 0x51e770 (cdecl; called by 0x55002b; name invented): the music [0x803aa0], when there is one, stopped (Sound_Stop; [0x803aa0] kept).
void __cdecl Sound_StopMusic();

// 0x51e790 (cdecl (value); called by 0x54f870; name invented): the music [0x803aa0] = Sound_SetParam(it, 4, value).
void __cdecl Sound_SetMusicLevel(u32 value);

// 0x51e7b0 (cdecl (level); name invented): the kids' level [0x803ab0] = the level, 100 at most (signed); with the kids' sounds on
// ([0x803aa8]): a level stored not 0 plays the kids' sound [0x803aa4] (Sound_PlayAt(0, gSndCatKids [0x803a24], 0x5b, 0, 0, 0)) when
// there is none, then [0x803aa4] = Sound_SetParam(it, 7, the level as passed, sic); a level 0 stops the sound (Sound_Stop) when there
// is one and clears it.
void __cdecl Sound_SetKidsLevel(s32 level);

// 0x51e830 (cdecl (level); name invented): as Sound_SetKidsLevel for the ambient sound [0x803aac] (Sound_PlayAt(0, gSndCatAmbient
// [0x803a20], 0x21, 0, 0, 0)), the level clamped to 0-100 (signed) and stored nowhere, its parameter 8 (the clamped level); stopping
// it keeps [0x803aac] (sic).
void __cdecl Sound_SetAmbientLevel(s32 level);

// 0x485aa0 (cdecl (sound); name unconfirmed): Sound_PlayAt(0, [0x803a2c], sound, 0, 0, 0).
u32 __cdecl SoundUi_Play(u32 sound);

// 0x486ae0 (cdecl; name unconfirmed): Sound_PlayAt(0, [0x803a2c], 0x1d, 0, 0, 0).
u32 __cdecl SoundUi_Play1d();

// 0x551560 (cdecl (level, x, y, z); name unconfirmed): a kids' sound for a level played at a position in the category gSndCatKids
// [0x803a24]: 0 for level 0 (nothing played), else sound 0x69 for level 1, 0x6a below 4, 0x6c below 8 (signed: so 0x6a for a
// negative level), 0x6d from 8 on; Sound_PlayAt's result.
u32 __cdecl KidSound_Play(s32 level, s32 x, s32 y, s32 z);

// 0x551320 (cdecl (level, a, b, x, y, z); name unconfirmed): as KidSound_Play, the sound graded by (a + b) / 50 (truncated, at most 3)
// over a base of 0x4b for level 1, 0x4f below 4, 0x53 below 8, 0x57 from 8 on: 0 (nothing played) for level 0 or a negative grade.
u32 __cdecl KidSound_PlayGraded(s32 level, s32 a, s32 b, s32 x, s32 y, s32 z);

// 0x51eeb0 (cdecl (id, owner, category, index, x, y, z); name unconfirmed): the sound of the ride record the record of id links to, its
// element index (Rides_GetLinkedElement), played with Sound_PlayAt(owner, category, it, x, y, z); 0 (nothing played) for none.
u32 __cdecl Sound_PlayRideElement(s32 id, u32 owner, u32 category, s32 index, s32 x, s32 y, s32 z);

}
