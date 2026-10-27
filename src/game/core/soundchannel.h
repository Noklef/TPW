// The sound object's channel (its +0x24; vtable 0x70a5d8, made by 0x6b8ed0 / 0x6b9c00; the class name is invented): +0x28 flags
// (0x400: its voices are run through the mixer), +0x2c a float, +0x44 four bytes, +0x48 an array of floats. Settings reach it as stack
// functors (a vtable whose slot 0 is thiscall (out, channel), pops 8, and the functor's data); +0x14 a source's info (+0 and +8 read),
// +0x18 its source object, +0x20 its EAX buffer (soundeax); +8 its position (three floats). Reimplementation of 0x6b7d70, 0x6b8b40, 0x6b5e40,
// 0x6b5e60, 0x6b7910, 0x6b7970, 0x6b79d0, 0x6b7a70, 0x6b7b60, 0x6b7e60, 0x6b7fa0, 0x6b90d0, 0x6b9930, 0x6b9a60 and 0x6b9ab0; the names are unconfirmed. The setters build a
// settings record on the stack (Init_6b9c50, then its fields) and apply it with the functor 0x70a278 (SoundChannel_Apply).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b8160 (thiscall (desc), pops 4; name invented): 0x6c53c0's new resource (the channel's +0x1c the owner, desc, the channel the source).
u8 *__fastcall SoundChannel_NewResource(u8 *channel, void *, const u32 *desc);
// 0x6b9310 (thiscall (index), pops 4; an SEH frame; name invented): the category singleton's (SoundCategory_Get) deque B entry index
// (DequeIter_Plus from its begin; past its count +0x94: none), a bank record: none, 0; its resource +8 when it has one. Else: the
// record's opener (+0x24) slot 4 (&status, &name, &empty; its answer unused), path = the WString +0xc, SoundBank_Open (&status, the
// opener, &path, &name, Obj_Flags28_Bit1IfBit0 of the channel): negative, 0. Then by the flags +0x20: 1 / 2 the opener's slot 3 (&status,
// &stream, the path's text, 0x81000001 / 0x80008001, 0) and, not negative, +8 = SoundChannel_NewResource of {2 / 1, -, &stream, the
// path's text}; 4 a SoundFont: name + path up to its last ".", ".sf2" appended, as {3, the cut length, the cut's text, the text}, and
// when that fails the flag-1 open. The descriptor's +4 is uninitialised on flags 1 and 2 (0 here, sic; the loads ignore it).
u8 *__fastcall SoundChannel_LoadBank(u8 *channel, void *, u32 index);
// 0x6b9160 (thiscall (out, on), pops 8; name invented): without flag 1 (+0x28) *out = -1. Else, when on (nonzero) differs from flag 2:
// flag 2 set or cleared to match, the channel's slot 4 (out: on's own stack slot, sic; a stack functor: vtable 0x70a5b0, its data
// unset) and SoundCategory_ReleaseBankResources; *out = 0. Returns out.
u32 *__fastcall SoundChannel_SetFlag2(u8 *channel, void *, u32 *out, u32 on);
// 0x6b7840 (thiscall; name invented): the EAX buffer (+0x20), when there is one, released (EaxBuffer_Release), freed (GameHeap_Free) and
// cleared.
void __fastcall SoundChannel_FreeEax(u8 *channel);
// 0x6b7750 (thiscall (out), pops 4; an SEH frame; name invented): the EAX buffer freed (SoundChannel_FreeEax); the source's (+0x18) sound
// (its slot 3, into a stack dword the slot must set); with one that has hardware 3D streaming buffers (a SoundCaps holder of it,
// SoundCaps_Max3DStreaming) +0x20 = a new zeroed 12-byte EAX buffer (null when the heap fails) opened (EaxBuffer_Open: out, the sound, the
// source's rate +0xc), else *out = -1. Returns out.
u32 *__fastcall SoundChannel_OpenEax(u8 *channel, void *, u32 *out);
// 0x6b7870 (thiscall (out), pops 4; an SEH frame for the new; name invented): +0x1c = a new synthesiser holder (8 bytes, Init_6c47b0;
// null when the heap fails) and SfHost_OpenOut on it (on null too, sic). Returns out.
u32 *__fastcall SoundChannel_OpenSynth(u8 *channel, void *, u32 *out);
// 0x6b78e0 (thiscall; name invented): the synthesiser holder (+0x1c), when there is one, destroyed (SfHost_Dtor), freed and cleared.
void __fastcall SoundChannel_FreeSynth(u8 *channel);

// 0x6b8b40 (thiscall (out, functor), pops 8): the functor's slot 0 (out, channel). Returns out.
u32 *__fastcall SoundChannel_Apply(u8 *channel, void *, u32 *out, u8 *functor);

// 0x6b5e40 (the stack functor 0x70a260's slot 0: thiscall (out, channel), pops 8; the functor is not read): the channel's source
// (+0x18)'s slot 1; *out = 0. Returns out.
u32 *__fastcall SoundChannelFn70a260_Run(const u8 *functor, void *, u32 *out, u8 *channel);

// 0x6b5e60 (the stack functor 0x70a278's slot 0: thiscall (out, channel), pops 8; the functor's +4 a settings record, as
// SoundChannel_GetSettings fills, reread for each test): with +4 set the source's slot 5 (a local out, +4); then nothing more without
// flags (+0x4c). Bit 7: the voices' count (ChannelVoices_SetCount, the channel's +0x14). Bit 2: SoundChannel_SetPosition (a local out).
// Bit 3: a copy of the vector at +0x54 scaled by 1 / its length (Vec3_Scale; the x87's sqrt) to the source's slot 0x23 (out, the copy,
// +0x58), returning out as it is (sic: nothing more, *out not set). Else bit 4: the source's slot 0x19 (a local out, +0x5c); bit 9: the
// dword at 0xfb2708 = +0x68 (0x6baa10) and the source's slot 0x10 (a local out, +0x68); bit 5 / bit 6: SoundChannel_SetEaxProperty0b /
// SoundChannel_SetEaxAll (a local out). *out = 0. Returns out.
u32 *__fastcall SoundChannelFn70a278_Run(const u8 *functor, void *, u32 *out, u8 *channel);

// 0x6b79d0 (thiscall (out, settings), pops 8): the source's slot 0x22 (out: the settings argument's own slot, sic; the channel's position
// (+8), the settings' +0x50 vector pointer, 0, 1 when the settings' +4 bit 6 is clear); when the dword it returns a pointer to is negative,
// *out = -1 and nothing more. Else the vector (when not null) becomes the position; with +4 bit 11, SoundChannel_UpdateOcclusion (1 with
// an EAX buffer (+0x20); else 0 when the source's slot 0x13 answers nonzero, not at all when it answers 0). *out = 0. Returns out.
u32 *__fastcall SoundChannel_SetPosition(u8 *channel, void *, u32 *out, u8 *settings);

// 0x6b7b60 (thiscall (eax, a byte), pops 4): for each of the voices in use (channelvoice; the first even with none in use, sic; none
// without an array) that has a sound and +0x14 bit 5 but not bit 0: its position from the source's slot 0x1e (a local out, the voice's id
// (+0x2c shifted right by the byte at 0x782db8), the position, zeroed first); the occluders' test (soundoccluders, 0xfb2038) from the
// channel's position to it. Occluded: when the occlusion differs from the voice's +0x28 (neither equal nor NaN) it is stored there, the
// EAX 2.0 obstruction and occlusion (fistp of it times -2000 and -1000) set and flag 1 set (ChannelVoice_SetFlag). Not occluded: with flag
// 1 and +0x28 neither 0 nor NaN, +0x28 = 0, both levels 0 and flag 1 cleared. Then, if either changed: with eax the source's slot 0x12
// (the id, twelve dwords of EAX 2.0 buffer properties: 0 but for the levels (the 6th and 8th) and 0.25, 0.5 and 1.0 (9th, 10th, 12th));
// else its slot 0x15 (a local out, the id, 1 - the occlusion, 0.5 at least: 0.5 when above it, NaN kept).
void __fastcall SoundChannel_UpdateOcclusion(u8 *channel, void *, u8 eax);
// 0x6b7fa0 (thiscall (id, voice, record), pops 0xc): one voice's position and occlusion. Nothing without a source (+0x18) or the record's
// position (+0x30). The source's slot 0x1d (a local out, the id, the position, the record's +0xc bit 5); then only with +0xc bit 11, an
// EAX buffer (+0x20) or the source's slot 0x13 answering (AL), and bit 5: the occlusion of the path from the channel's position (+8) to
// the record's (SoundOccluders_Occlusion). Occluded: unless it equals the voice's +0x28 (or either is NaN), stored there, the EAX 2.0
// levels (fistp of it times -2000 and -1000) set and the voice's +0x10 bit 0 set. Not occluded: with that bit and +0x28 neither 0 nor
// NaN, +0x28 = 0 and the bit cleared. Else nothing more. Then with the EAX buffer the source's slot 0x12 (the id, the twelve dwords of
// SoundChannel_UpdateOcclusion), else its slot 0x15 (a local out, the id, 1 - the occlusion, 0.5 at least).
void __fastcall SoundChannel_UpdateVoiceOcclusion(u8 *channel, void *, u32 id, u8 *voice, const u8 *record);
// 0x6b7a70 (thiscall (eax byte), pops 4): each voice with a sound and flag 1 (+0x10 bit 0): the flag cleared; with +0x28 neither 0 nor
// NaN, +0x28 = 0 and with eax the source's slot 0x12 (the id, SoundChannel_UpdateOcclusion's twelve dwords with both levels 0), else its
// slot 0x15 (a local out, the id, 1.0).
void __fastcall SoundChannel_ClearOcclusion(u8 *channel, void *, u8 eax);

// 0x6b90d0 (thiscall (out, a, b, c), pops 0x10): flags (+0x28) bit 3 set when a is nonzero, else cleared; a settings record whose +4
// gets bit 2 (and the flags bit 8) when b is nonzero, bit 3 when c is; applied (so the source's slot 5 gets +4 when nonzero). Returns out.
u32 *__fastcall SoundChannel_SetMode(u8 *channel, void *, u32 *out, u32 a, u32 b, u32 c);
// 0x6b9930 (thiscall (out, position, direction, value), pops 0x10): +0x60 = the position (a vector pointer); a settings record with
// the position (+0x50, bit 2) and the direction and value (+0x54 / +0x58, bit 3: Obj_Set54And58Flag8), +4 bit 11 when the flags (+0x28)
// have bit 7 and one of bits 0, 1, 6; applied (SoundChannel_SetPosition, the occlusion with bit 11, the direction scaled to a unit
// vector to the source's slot 0x23). Returns out.
u32 *__fastcall SoundChannel_SetPlacement(u8 *channel, void *, u32 *out, u32 position, u32 direction, u32 value);

// 0x6b7d70 (thiscall (out, settings), pops 8): fills a settings record (made by Init_6b9c50: +4 and +8 the asked-for flags, +0x4c the
// flags of what was filled) from the channel: +4 bit 8 cleared with an EAX buffer, or with a source whose slot 0x13 answers 0; +4 bit 9
// cleared without an EAX buffer; for +8 bit 8 / bit 7 (with info at +0x14) +0x44 / +0x40 = the info's +0 / +8 and the flag; for bit 0
// the source's slot 3 (settings; no null test), the flag when +0 is then set; for bit 6 (with an EAX buffer) EaxBuffer_QueryAll (out:
// the settings argument's own slot) and the flag when it succeeds; for bit 10 +0x6c = [0xfb2058] and the flag. *out = 0. Returns out.
u32 *__fastcall SoundChannel_GetSettings(u8 *channel, void *, u32 *out, u8 *settings);

// 0x6b7910 / 0x6b7970 (thiscall (out, settings), pops 8): with an EAX buffer (+0x20), its property 0xb set (soundeax: out, the settings'
// +0x60) / all its properties set (out, settings + 0xc); else with a source (+0x18) whose slot 0x13 answers nonzero, its slot 0x14 (out, settings +
// 0xc), both; else *out = -1. Returns out.
u32 *__fastcall SoundChannel_SetEaxProperty0b(u8 *channel, void *, u32 *out, u8 *settings);
u32 *__fastcall SoundChannel_SetEaxAll(u8 *channel, void *, u32 *out, u8 *settings);

// 0x6b7e60 (thiscall (out, voice, value), pops 0xc): with a voice (channelvoice), the source's slot 10 (out: the voice argument's own
// stack slot, sic; the voice's id +0x2c shifted right by the byte at 0x782db8); when the dword it returns a pointer to is not negative,
// the voice released (value). *out = -1 either way (sic). Returns out.
u32 *__fastcall SoundChannel_ReleaseVoice(u8 *channel, void *, u32 *out, u8 *voice, u32 value);
// 0x6b7eb0 (thiscall; name invented): with a voice array (the pool +0x14's +4), SoundChannel_ReleaseVoice (a local out, the voice, 0)
// for each voice up to the count (+8, read once) of them, the first one even when the count is 0 (sic: the test comes after).
void __fastcall SoundChannel_ReleaseVoices(u8 *channel);
// The sound commands' stack functors' runs on a channel (slot 1 of each, thiscall (out, channel), pops 8; slot 0 the functor's type;
// the names are invented). By a voice's handle (the functor's +4: ChannelVoices_Holds on the channel's pool +0x14, its index out the
// channel argument's own stack slot, sic): 0x6b8d60 (0x70a2d0): the held voice released (SoundChannel_ReleaseVoice (out, the voice, 0)),
// else *out = 0; 0x6b8df0 (0x70a5b8) / 0x6b8e60 (0x70a5c0): the source's (+0x18) slot 11 / 12 (out, the index), else *out = -1.
// 0x6b8dd0 (0x70a5b0): SoundChannel_ReleaseVoices, *out = 0. 0x6b8e40 (0x70a268) / 0x6b8eb0 (0x70a270): the source's slot 14 / 15 (out).
// Each returns out.
u32 *__fastcall SoundChannelFn70a2d0_Run(const u8 *functor, void *, u32 *out, u8 *channel);
u32 *__fastcall SoundChannelFn70a5b8_Run(const u8 *functor, void *, u32 *out, u8 *channel);
u32 *__fastcall SoundChannelFn70a5c0_Run(const u8 *functor, void *, u32 *out, u8 *channel);
u32 *__fastcall SoundChannelFn70a5b0_Run(const u8 *functor, void *, u32 *out, u8 *channel);
u32 *__fastcall SoundChannelFn70a268_Run(const u8 *functor, void *, u32 *out, u8 *channel);
u32 *__fastcall SoundChannelFn70a270_Run(const u8 *functor, void *, u32 *out, u8 *channel);
// 0x6b8f60 (slot 13 of 0x70a5d8, thiscall (out, record, value), pops 0xc, the channel unused; name invented): a settings record's +0x40
// = value (SetThenFlag40_4c_6b9cf0), its +0x68 = 0 (SetThenFlag68_4c_6b9d30), its +4 |= 0xc; *out = 0. Returns out.
u32 *__fastcall SoundChannel_SetRecordValue40(u8 *channel, void *, u32 *out, u8 *record, u32 value);
// 0x6b5fc0 (slot 14 of 0x70a5d8, thiscall (out, channel), pops 8, this unused; name invented): with a voice array (the channel's pool
// +0x14, its +4), for each voice up to the count (+8, read once) of them, the first one even when the count is 0 (sic), the channel's
// source (+0x18, re-read each time; its vtable read before the rate) slot 32 (a local out, the voice's id, ChannelVoice_ScaledRate);
// *out = 0. Returns out.
u32 *__fastcall SoundChannel_ApplyVoiceRates(u8 *self, void *, u32 *out, u8 *channel);

// 0x6b9a60 (thiscall (a, &picked, record), pops 0xc; name invented): the channel's slot 6 (a, record) picks a voice, kept in *picked; none
// and the record's flags (+0x14, read after) with bit 13: 0. Else the channel's slot 5 (the voice, or a without one; record), its result returned.
u32 __fastcall SoundChannel_PickVoice(u8 *channel, void *, u32 a, u32 *picked, u8 *record);

// 0x6b9ab0 (thiscall (record, &voice), pops 8; name invented): with the channel's flags (+0x28) bits 3 and 8 both set, the record's +0xc
// gets bit 1; SoundChannel_PickVoice (0, voice, record): 0 when it answers 0. Else *voice (when 0) = its result, and the channel's slot 4
// (out: the record argument's own stack slot, sic, so the record's address unless written; a stack functor {0x70a618, *voice}): 0 when the
// dword it returns a pointer to is negative, else PickVoice's result.
u32 __fastcall SoundChannel_PlayRecord(u8 *channel, void *, u8 *record, u32 *voice);

// 0x6b8180 (thiscall (out, on, radius), pops 0xc; name invented): the occluders' (0xfb2038) switch, the byte +0x18 (0xfb2050), and
// their smallest radius that counts, the float +0x14 (0xfb204c): nothing when the switch is as it was and the radius compares equal or
// unordered (fcomp: C3); else both stored (the radius as its dword) and, with an EAX buffer (+0x20) or else the source's slot 0x13
// answering nonzero (a byte), the switch turned off: SoundChannel_ClearOcclusion, turned on: SoundChannel_UpdateOcclusion (eax: the EAX
// buffer is there). *out = 0; returns out.
u32 *__fastcall SoundChannel_SetOcclusion(u8 *channel, void *, u32 *out, u8 on, u32 radius);
// 0x6b8230 (thiscall (x, y, z, radius), pops 0x10): with an EAX buffer (+0x20) or the source's slot 0x13 answering nonzero (a byte), the
// occluder {x, y, z, radius} copied into the occluders' list (0xfb2038) by its radius (PtrList_InsertCopyByKey); with a node made and the
// occlusion switched on (0xfb2050), SoundChannel_UpdateOcclusion (eax: the buffer's). Returns the node, 0 without one or without EAX.
u8 *__fastcall SoundChannel_AddOccluder(u8 *channel, void *, u32 x, u32 y, u32 z, u32 radius);
// 0x6b99c0 (thiscall (out, on, radius), pops 0xc; name invented): with `on` (a dword) and the flags (+0x28) & 0x42, their bit 7 set and
// SoundChannel_SetOcclusion (out, 1, radius); else bit 7 cleared and (out, 0, radius). (The byte passed sits in a dword whose upper bytes
// are the channel's address's: the callee reads the byte only.) Returns out.
u32 *__fastcall SoundChannel_SwitchOcclusion(u8 *channel, void *, u32 *out, u32 on, u32 radius);
// 0x6b9a10 (thiscall (out, x, y, z, radius, &node), pops 0x18; name invented): *node = 0; with the flags (+0x28) & 0x41,
// *node = SoundChannel_AddOccluder (x, y, z, radius) and *out = 0 when it made one; else (or without the flags) *out = -1. Returns out.
u32 *__fastcall SoundChannel_TryAddOccluder(u8 *channel, void *, u32 *out, u32 x, u32 y, u32 z, u32 radius, u8 **node);
// 0x6b82c0 (thiscall (out, &node), pops 8): with a node (*node) and EAX (+0x20: 1, else the source's slot 0x13's byte, nonzero),
// SoundOccluders_Remove on the occluders' list (0xfb2038; a local out) and, with the occlusion switched on (0xfb2050),
// SoundChannel_UpdateOcclusion (that byte); *out its status. Else *out = 0. Returns out.
u32 *__fastcall SoundChannel_RemoveOccluder(u8 *channel, void *, u32 *out, u8 **node);

// 0x6b9230 (thiscall; slot 0 of 0x70a5d8 and 0x70a630; name invented): the channel's slot 4 (an unset out, a stack functor of vtable
// 0x70a5b0 whose second dword is unset), then Class703770_Release; returns its count.
s32 __fastcall SoundChannel_Release(u8 *channel);
// 0x6c5c50 (thiscall (out, record, value), pops 0xc; slot 7 of 0x70a630; name invented): SoundChannel_SetRecordValue40 with the value's
// own stack slot as its out; not negative: SetThenFlag60_4c_6b9cb0 (record, 1) and *out = 0, else *out = -1. Returns out.
u32 *__fastcall SoundChannel_SetRecordValue40Flagged(u8 *channel, void *, u32 *out, u8 *record, u32 value);
// 0x6b98d0 (thiscall (out, index, modifier), pops 0xc; name invented): the rate modifier +0x48 + 4 * the index's low byte set (fld /
// fstp), then SoundChannel_Apply (out, a stack functor {0x70a610, the modifier}). Returns out.
u32 *__fastcall SoundChannel_SetRateModifier(u8 *channel, void *, u32 *out, u32 index, u32 modifier);
// 0x6b9cd0 (thiscall (block), pops 4; name invented): a settings record's +0xc-+0x3f from the block's 13 dwords (rep movsd), its flags
// +0x4c |= 0x40. Returns the record.
u8 *__fastcall SoundChannel_SetRecordBlock(u8 *record, void *, const u32 *block);

// 0x6b92f0 (thiscall (hide), pops 4; name invented): with the byte +0x64 not the argument's low byte, that byte stored and
// SoundCategory_HideFlag40Samples (the argument) on SoundCategory_Get's singleton.
void __fastcall SoundChannel_SetHidden(u8 *channel, void *, u32 hide);

}  // extern "C"
