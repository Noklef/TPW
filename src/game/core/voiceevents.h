// The channel voices' events (vtable 0x70ab50; the class name is invented): what a sound channel's service thread waits on. +4 the
// voices (channelvoice.h's, 0x8c bytes each), +0xc their count, +8 the handles' count (one more: the last, at index count, the stop
// event), +0x10 the handles (Mem_Alloc), +0x14 flags (bit 0 stopping, bit 1 running). Reimplementation of 0x6c6120, 0x6c6090, 0x6c6100
// and 0x6c6160.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c6120 (thiscall (voices, count), pops 8): +4 the voices, +0xc the count, +8 the count + 1, +0x10 that many handles' block
// (Mem_Alloc; not filled), +0x14 0, the vtable. Returns the object.
u8 *__fastcall VoiceEvents_Ctor(u8 *self, void *, u8 *voices, u32 count);
// 0x6c6090 (thiscall; an SEH frame): the vtable; each of the handles (+8, read again each time) closed (CloseHandle); the block freed.
void __fastcall VoiceEvents_Dtor(u8 *self);
// 0x6c6100 (thiscall (flags), pops 4; slot 0): VoiceEvents_Dtor, the object freed (Mem_Free) for flags bit 0. Returns the object.
u8 *__fastcall VoiceEvents_DeletingDtor(u8 *self, void *, u32 flags);
// 0x6c6160 (thiscall): flag 0 (stopping) set; the stop event (the handle at index count) set (SetEvent). Returns its answer.
u32 __fastcall VoiceEvents_Stop(u8 *self);

}  // extern "C"
