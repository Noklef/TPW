// The sound object's channel (its +0x24; vtable 0x70a5d8, made by 0x6b8ed0 / 0x6b9c00; the class name is invented): +0x28 flags
// (0x400: its voices are run through the mixer), +0x2c a float, +0x44 four bytes, +0x48 an array of floats. Settings reach it as stack
// functors (a vtable whose slot 0 is thiscall (out, channel), pops 8, and the functor's data); +0x14 a source's info (+0 and +8 read),
// +0x18 its source object, +0x20 its EAX buffer (soundeax). Reimplementation of 0x6b7d70, 0x6b8b40, 0x6b5e40, 0x6b7910 and 0x6b7970; the names
// are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b8b40 (thiscall (out, functor), pops 8): the functor's slot 0 (out, channel). Returns out.
u32 *__fastcall SoundChannel_Apply(u8 *channel, void *, u32 *out, u8 *functor);

// 0x6b5e40 (the stack functor 0x70a260's slot 0: thiscall (out, channel), pops 8; the functor is not read): the channel's source
// (+0x18)'s slot 1; *out = 0. Returns out.
u32 *__fastcall SoundChannelFn70a260_Run(const u8 *functor, void *, u32 *out, u8 *channel);

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

}  // extern "C"
