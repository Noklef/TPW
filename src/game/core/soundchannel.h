// The sound object's channel (its +0x24; vtable 0x70a5d8, made by 0x6b8ed0 / 0x6b9c00; the class name is invented): +0x28 flags
// (0x400: its voices are run through the mixer), +0x2c a float, +0x44 four bytes, +0x48 an array of floats. Settings reach it as stack
// functors (a vtable whose slot 0 is thiscall (out, channel), pops 8, and the functor's data); +0x14 a source's info (+0 and +8 read),
// +0x18 its source object, +0x20 its EAX buffer (soundeax); +8 its position (three floats). Reimplementation of 0x6b7d70, 0x6b8b40, 0x6b5e40,
// 0x6b5e60, 0x6b7910, 0x6b7970, 0x6b79d0, 0x6b7a70, 0x6b7b60, 0x6b7e60, 0x6b7fa0, 0x6b90d0 and 0x6b9930; the names are unconfirmed. The setters build a
// settings record on the stack (Init_6b9c50, then its fields) and apply it with the functor 0x70a278 (SoundChannel_Apply).
#pragma once

#include "../../../re/types.h"

extern "C" {

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

}  // extern "C"
