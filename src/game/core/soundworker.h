// The streamed-sound worker (reimplementation of 0x6b6050, 0x6b8380, 0x6d3850, 0x6d6f70 and 0x6d75c0; all the names invented): a worker object with its critical section,
// its 30 voices and its thread, the host that makes one, and the refill of a voice's sound buffer from the voice's mixer.
#pragma once

#include "../../../re/types.h"

// What the worker calls outside it, so that it can run outside the game (the benches point it at a recorder).
struct SoundWorkerEnv {
    u32(__cdecl *beginThread)(u32 entry, u32 stack, void *arg);   // 0x67e050 (agent B's, unported)
};

extern "C" {
// The calls go to env (nullptr: the game's).
void SoundWorker_SetEnv(const SoundWorkerEnv *env);

// 0x6d6f70 (thiscall (voice, offset, size), pops 0xc): with a DirectSound buffer at +0: Lock(offset, size, &p1, &n1, &p2, &n2, 0); locked, ChannelVoice_Mix(voice, p1, n1), then
// Unlock(p1, n1, n1, 0) (sic: n1 as the second pointer); when the mix answered 0, [0xfb2724]'s slot 10 (thiscall (&voice, the voice's +0x2c >> [0x782db8])). Returns nothing useful.
void __fastcall SoundBuffer_FillFromVoice(u8 *holder, void *, u8 *voice, u32 offset, u32 size);

// 0x6d3850 (thiscall (position), pops 4): of the slot position % +4: the voice (+0x34's +4, stride 0x8c; null: nothing) has its buffer (+0x28, stride 0x2c) filled under the section +0x38,
// the offset the voice's Obj_Get20Or2400 when position is below the count, else 0, the size that value. A count of 0 divides by 0.
void __fastcall SoundWorker_RefillVoice(u8 *owner, void *, u32 position);

// 0x6d75c0 (thiscall): under the section +0x10, each node of the list (+4 head, +8 tail, +0xc count; node +0 a COM object, +4 next, +8 previous): the object's slots 6 and 2,
// the node unlinked and freed.
void __fastcall ComList_ReleaseAll(u8 *list);

// 0x6b6050 (thiscall (count), pops 4; returns this): the section +4, the zeroed fields, 30 voices at +0x3c, blocks of 0x78 (zeroed, unchecked) at +0x40 and 0x7c at +0x44, +0x4c = the
// count at most 30, byte +0x50 = 1, vtable 0x70a2cc, the thread (0x6b6040, this).
u8 *__fastcall SoundWorker_Ctor(u8 *worker, void *, u32 count);

// 0x6b8380 (thiscall; returns this): +4 to +0x18 0, +0x1c = 12, +0x20 = a new SoundWorker of 12 (null when the allocation fails), +0x24 to +0x30 0, vtable 0x70a588.
u8 *__fastcall SoundWorkerHost_Ctor(u8 *host);
}

#include "gamedata.h"

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[3], SoundWorker_vtable, 0x0070a2cc, 0xc);
GAME_DATA(void *[16], SoundWorkerHost_vtable, 0x0070a588, 0x40);
