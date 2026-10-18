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

// 0x485aa0 (cdecl (sound); name unconfirmed): Sound_PlayAt(0, [0x803a2c], sound, 0, 0, 0).
u32 __cdecl SoundUi_Play(u32 sound);

// 0x486ae0 (cdecl; name unconfirmed): Sound_PlayAt(0, [0x803a2c], 0x1d, 0, 0, 0).
u32 __cdecl SoundUi_Play1d();

}
