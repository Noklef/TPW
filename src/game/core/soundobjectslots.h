// The sound object's small vtable slots and two update functors, each handing on to a port. Reimplementation of 0x6b7f20, 0x6b7f80, 0x6b8360, 0x6b9ba0, 0x6b9bb0, 0x6ba1f0,
// 0x6bb4f0 and 0x6c5630; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct SoundObjectSlotsEnv {
    u32 *(__fastcall *apply)(const u8 *update, void *, u32 *out, u8 *channel, u32 id);    // SoundUpdate_Apply
    u32(__fastcall *holds)(u8 *pool, void *, const u32 *handle, u32 *out);                // ChannelVoices_Holds
    u32(__fastcall *play)(u8 *pool, void *, u32 handle, u8 *record, u32 *out);            // ChannelVoices_PlayRecord
    u32(__fastcall *notify)(u8 *pool, void *, u32 handle, u8 *record);                    // ChannelVoices_PlayNotify
    u32(__fastcall *mix)(u8 *voice, void *, u8 *dest, u32 size);                          // ChannelVoice_Mix
};

void SoundObjectSlots_SetEnv(const SoundObjectSlotsEnv *env);

// 0x6b7f20 (thiscall (handle, record), pops 8; stored at 0x70a55c, 0x70a5ec and 0x70a644): ChannelVoices_PlayRecord on the pool +0x14, its out the record argument's own
// stack slot (sic: unwritten, the record is what the functor gets). When that answers not 0, the object's slot 4 (out: the handle's slot; a stack functor: vtable 0x70a580, the
// answer, the record, the out). Returns the answer.
u32 __fastcall SoundObject_PlayRecord(u8 *object, void *, u32 handle, u8 *record);

// 0x6b7f80 (thiscall (handle, record), pops 8; stored at 0x70a560, 0x70a5f0 and 0x70a648): ChannelVoices_PlayNotify on the pool +0x14; its answer.
u32 __fastcall SoundObject_PlayNotify(u8 *object, void *, u32 handle, u8 *record);

// 0x6b8360 (thiscall (out, channel), pops 8; stored at 0x70a584): SoundUpdate_Apply on the voice id +0xc. Returns out.
u32 *__fastcall SoundUpdateById_Apply(const u8 *update, void *, u32 *out, u8 *channel);

// 0x6c5630 (thiscall (out, channel), pops 8; stored at 0x70a854): with a handle +4 that the channel's pool +0x14 holds (ChannelVoices_Holds, the low byte of its answer; its
// out the channel argument's own stack slot, sic: the id passed on is the channel's address when it writes nothing), SoundUpdate_Apply on that id. Otherwise *out = -1. Returns out.
u32 *__fastcall SoundUpdateByHandle_Apply(const u8 *update, void *, u32 *out, u8 *channel);

// 0x6b9ba0 (thiscall (unused), pops 4; stored at 0x70a600): 0.
float __fastcall SoundObject_SlotZero(void *object, void *, u32 unused);

// 0x6ba1f0 (thiscall (unused), pops 4; stored at 0x70a658): 1.
float __fastcall SoundObject_SlotOne(void *object, void *, u32 unused);

// 0x6b9bb0 (thiscall; stored at 0x70a604 and 0x70a65c): 0.2 with bit 6 of +0x28, else 0.8.
float __fastcall SoundObject_SlotByFlag(const u8 *object);

// 0x6bb4f0 (stdcall (dest, size, voice), pops 0xc; the callback the QSound source hands over at 0x6d240b): ChannelVoice_Mix on the voice; its answer.
u32 __stdcall SoundVoice_MixCallback(u8 *dest, u32 size, u8 *voice);
