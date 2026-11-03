// The DirectSound source (vtable 0x711b08; core/soundsource.h has its constructor and more): +4 the channels, +8 bit 6 the worker runs, bit 7 it is waiting, +0x18 the DirectSound object (CoCreated), +0x1c its listener, +0x20 and +0x24 two more of
// its interfaces, +0x28 the slots (0x2c bytes each, SoundSlot; the count at -4), +0x2c the +0x30 events, +0x38 the critical section around the slots. Reimplementation of 0x6d2d60, 0x6d2de0, 0x6d30b0 and 0x6d3190-0x6d3a20; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct DSoundSourceEnv {
    u32(__cdecl *beginthread)(u32 proc, u32 stack, void *arg);                               // CRT_beginthread
    void(__stdcall *vectorCtor)(u8 *array, u32 size, u32 count, u32 ctor, u32 dtor);         // CRT_VectorCtorIterator
    void(__stdcall *vectorDtor)(u8 *array, u32 size, u32 count, u32 dtor);                   // CRT_VectorDtorIterator
    void(__fastcall *refill)(u8 *source, void *, u32 position);                              // SoundWorker_RefillVoice
    void(__fastcall *waitLoop)(u8 *source);                                                  // SoundSource_WaitLoop
};
void DSoundSource_SetEnv(const DSoundSourceEnv *env);

extern "C" {

// 0x6d2d60 (cdecl (source)): SoundSource_WaitLoop: the worker's thread.
void __cdecl SoundSource711b08_Thread(u8 *source);

// 0x6d2de0 (slot 6, (out, info)): ComInit_Get; CoCreateInstance into +0x18; its slot 15 (0, 0xc2, 0); SoundSource711b08_HasCaps; its slot 19 (the window info's +0x48 with info's +0x4c bit 1, else the import 0x6fd370's, 1); slot 23 (1); slot 16 (0x40), (2), each 0 a failure.
// Its QueryInterface 0x711ab8 into +0x20: with it, that one's slot 35 into +0x24 (with it, +0x24's slot 14 (0, 0), slot 12 (0.8, 0.8), and +0x20's slot 36 (+0x24)), +0x20's slot 3 (0x40), (2), slot 12 (10.0), slot 10 (1.0). QueryInterface 0x711ad8 into +0x1c, whose slot 3 (0, 0, 0).
// The slots: info's +0x44 of them in a new block (null when the heap fails, nothing constructed) at +0x28. Obj_Pick40Or44, the events, bit 6 set, the thread at SetThreadPriority (15). *out = 0; a negative answer anywhere: bit 6 off and *out = -1. Nothing undone.
u32 *__fastcall SoundSource711b08_Open(u8 *source, void *, u32 *out, const u8 *info);
// 0x6d30b0 (slot 7): with bit 6: the last event set, and a wait until bit 7 is off. The release list emptied; the four interfaces released and nulled (+0x1c, +0x20, +0x24, +0x18); with events each closed, the array freed and nulled; with slots
// each destroyed and the block freed (+0x28 keeps its value, sic). Bit 6 off.
void __fastcall SoundSource711b08_Close(u8 *source);
// 0x6d3190 (slot 24, (out, index, vector)): no +0x18: *out = -1. Else under the section SoundSlot_SetDistances. Returns out.
u32 *__fastcall SoundSource711b08_SetDistances(u8 *source, void *, u32 *out, u32 index, const u32 *vector);
// 0x6d3230 ((index)): under the section SoundSlot_IsSet's byte.
u32 __fastcall SoundSource711b08_IsSet(u8 *source, void *, u32 index);
// 0x6d32a0 ((out, index)): under the section SoundSlot_Free. Returns out.
u32 *__fastcall SoundSource711b08_FreeSlot(u8 *source, void *, u32 *out, u32 index);
// 0x6d3310 ((out)): no +0x18: *out = -1. Else under the section each channel's slot freed, +0x18's slot 13; *out = 0. Returns out.
u32 *__fastcall SoundSource711b08_FreeAll(u8 *source, void *, u32 *out);
// 0x6d33c0: with +0x18, under the section the release list emptied and +0x18's slot 13.
void __fastcall SoundSource711b08_Flush(u8 *source);
// 0x6d3430 (slot 9, (out, info)): index = info's +0x2c >> [0x782db8]. Under the section: +0x18's slot 17 (7 for info's +0x14 bit 0, else 6, a stack out), negative a failure. SoundSlot_Set the answer; SoundSlot_Call7 the format of the record at info's +0x3c;
// size = Obj_Get20Or2400 (info): SoundSlot_Call5 (size * 2), SoundSlot_Call59 (0, event index), (size, event index + channels), each negative: SoundSlot_Free, a failure. The refill (index + channels), SoundSlot_Restart (out). A failure: *out = -1. Returns out.
u32 *__fastcall SoundSource711b08_PlayWave(u8 *source, void *, u32 *out, u8 *info);
// 0x6d3600 (slot 29, (out, index, vector, unused)): under the section SoundSlot_SetPosition. Returns out.
u32 *__fastcall SoundSource711b08_SetPosition(u8 *source, void *, u32 *out, u32 index, const u32 *vector, u32);
// 0x6d3680 (slot 36, (out, unused, index, steps)): under the section SoundSlot_SetPitch. Returns out.
u32 *__fastcall SoundSource711b08_SetPitch(u8 *source, void *, u32 *out, u32, u32 index, s32 steps);
// 0x6d3900 (slot 32, (out, index, gain)): under the section SoundSlot_SetGain. Returns out.
u32 *__fastcall SoundSource711b08_SetGain(u8 *source, void *, u32 *out, u32 index, u32 gain);
// 0x6d3980 ((out, index, value)): under the section SoundSlot_Set48. Returns out.
u32 *__fastcall SoundSource711b08_Set48(u8 *source, void *, u32 *out, u32 index, u32 value);
// 0x6d3a00 (slot 0, (flags)): SoundSource711b08_Dtor, the source freed for flags bit 0. Returns the source.
u8 *__fastcall SoundSource711b08_DeletingDtor(u8 *source, void *, u32 flags);
// 0x6d3a20: vtable 0x711b08; the section deleted; vtable 0x7119b8; SoundHelper_Free.
void __fastcall SoundSource711b08_Dtor(u8 *source);
}
