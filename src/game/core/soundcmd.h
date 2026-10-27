// The sound commands' runs: each command class (constructed on the poster's stack: SoundCmd_Ctor 0x51bb40 sets the base vtable
// 0x700b58 and the value at +4, the poster then its own one-slot vtable) is posted to the sound manager [0x802bcc], whose engine
// calls the slot as thiscall (out, target), pops 8, and the run applies the value to the target (a sound object: +0x20 and +0x24 its
// parts) and returns out. Reimplementation of the runs whose callees are ported: 0x6b5640, 0x6b5670, 0x6b56a0, 0x6b5770, 0x6b5790,
// 0x6b5840, 0x6b58b0, 0x6b5990, 0x6b59f0, 0x6b5a50, 0x6b57b0 and 0x6b5810, and of the runs on the sound object (soundobject) and its
// mixer: 0x6b5600, 0x6b5620, 0x6b56c0, 0x6b58d0, 0x6b5930, 0x6b5a90 (the second value at +8), 0x6b55c0 and 0x6b5720. The names are the vtables' (the commands' meanings are unconfirmed).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b5770 (vtable 0x700b2c, posted by SoundCategory_Release 0x51b620): target's slot 8 (out, the value).
u32 *__fastcall SoundCmd700b2c_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b56a0 (vtable 0x700b34): the value as a float (fld / fstp: a signalling NaN comes out quiet) stored at +0x2c of target's +0x24
// (Set2c_6b9220); *out = 0.
u32 *__fastcall SoundCmd700b34_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5790 (vtable 0x700b48): target's slot 7 (out, the value).
u32 *__fastcall SoundCmd700b48_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5840 (vtable 0x700b50): slot 9 of target's +0x24 (out, the value).
u32 *__fastcall SoundCmd700b50_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5640 / 0x6b5670 (vtables 0x700b64 / 0x700b68, posted by 0x51bcf0 / 0x51bd30): slot 4 of target's +0x24 (out, a stack
// functor: its vtable 0x70a268 / 0x70a270 and a second dword left unset). The command's value is not read.
u32 *__fastcall SoundCmd700b64_Run(const u8 *command, void *, u32 *out, u8 *target);
u32 *__fastcall SoundCmd700b68_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b59f0 / 0x6b5a50 / 0x6b5990 (vtables 0x700b70 / 0x700b74 / 0x700b78, posted by the sound update 0x51bd70 with 0x803ad8 /
// 0x803adc / 0x803ae0): Bytes44_Set(target's +0x24, out, 3 / 1 / 2, the value).
u32 *__fastcall SoundCmd700b70_Run(const u8 *command, void *, u32 *out, u8 *target);
u32 *__fastcall SoundCmd700b74_Run(const u8 *command, void *, u32 *out, u8 *target);
u32 *__fastcall SoundCmd700b78_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b58f0 / 0x6b5a10 / 0x6b5950 / 0x6b59b0 (vtables 0x700b80 / 0x700b88 / 0x700b7c / 0x700b84, posted by the sound update 0x51bd70
// with [0x803ab8] / [0x803ac0] / [0x803ac4] / [0x803abc]): SoundChannel_SetRateModifier on target's +0x24 (out, the index 0 / 1 / 2 / 3, the value
// times 0.01 ([0x70a280]; fild qword of the zero-extended value, fmul float, fstp float)). Returns out.
u32 *__fastcall SoundCmd700b80_Run(const u8 *command, void *, u32 *out, u8 *target);
u32 *__fastcall SoundCmd700b88_Run(const u8 *command, void *, u32 *out, u8 *target);
u32 *__fastcall SoundCmd700b7c_Run(const u8 *command, void *, u32 *out, u8 *target);
u32 *__fastcall SoundCmd700b84_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b58b0 (vtable 0x700ba8): target's slot 6 (out, the value).
u32 *__fastcall SoundCmd700ba8_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5600 (vtable 0x700b5c): SoundObject_Broadcast (out) on the target. The command's values are not read.
u32 *__fastcall SoundCmd700b5c_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b55c0 (vtable 0x700b6c): SoundChannel_Apply on target's +0x24 with the stack functor 0x70a260 (its run SoundChannelFn70a260_Run)
// and the target argument's own stack slot as the out (sic: it is zeroed, the target already read), then SoundMixer_Update on its +0x20.
// *out = 0. The command is not read.
u32 *__fastcall SoundCmd700b6c_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5620 (vtable 0x700b60): SoundObject_BroadcastValue (out, the value).
u32 *__fastcall SoundCmd700b60_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b56c0 (vtable 0x700bbc): *out = -1 for an id (+4) of 0; else SoundMixer_SetVoiceParams on target's +0x20 (out, the id, a stack
// copy of the 16 bytes at +8).
u32 *__fastcall SoundCmd700bbc_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b58d0 (vtable 0x700ba4): SoundMixer_SetVoicePending on target's +0x20 (out, the id at +8, the value at +4).
u32 *__fastcall SoundCmd700ba4_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5930 (vtable 0x700bac): SoundObject_SendVoice (out, the id at +4, the value at +8).
u32 *__fastcall SoundCmd700bac_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5a90 (vtable 0x700bb8): SoundObject_VoiceInfo (out, the id at +8, the result pointer at +4).
u32 *__fastcall SoundCmd700bb8_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b57b0 (vtable 0x700b4c): the target's channel (+0x24) given a settings record with the value at +0x68 (bit 9: the dword at 0xfb2708
// and the source's slot 0x10) with the functor 0x70a278 (SoundChannel_Apply).
u32 *__fastcall SoundCmd700b4c_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5720 (vtable 0x700b3c): the sound object's voice count set (soundobject, 0x6b84f0: out, the value).
u32 *__fastcall SoundCmd700b3c_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5810 (vtable 0x700b54; three values at +4, +8, +0xc): the channel's mode set (soundchannel, 0x6b90d0) on the target's channel
// (+0x24) (out, the values).
u32 *__fastcall SoundCmd700b54_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5c90 (vtable 0x700b30; the index at +4): the sound object's EAX record applied (soundobject, 0x6ba1b0: out, the index).
u32 *__fastcall SoundCmd700b30_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5cb0 (vtable 0x700bc0; five values at +4-+0x14): the sound object's occluder add (soundobject, 0x6b8c70: out, the values).
u32 *__fastcall SoundCmd700bc0_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5ce0 (vtable 0x700bc4; the node pointer at +4): the sound object's occluder remove (soundobject, 0x6b8cb0: out, the node pointer).
u32 *__fastcall SoundCmd700bc4_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5890 (vtable 0x700bc8; two values at +4, +8): the occlusion switch (soundchannel, 0x6b99c0) on the target's channel (+0x24).
u32 *__fastcall SoundCmd700bc8_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5860 (vtable 0x700b44; a flag at +4): the target's channel (+0x24) hidden or shown (SoundChannel_SetHidden: whether the flag is
// not 0); *out = 0. Returns out.
u32 *__fastcall SoundCmd700b44_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5c70 (vtable 0x700ba0; the id at +4): the sound object's functor 3 send (soundobject, 0x6b8a70: out, the id).
u32 *__fastcall SoundCmd700ba0_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5c10 (vtable 0x700bb0; the stream at +4): the sound object's save (soundobject, 0x6ba0c0: out, the stream).
u32 *__fastcall SoundCmd700bb0_Run(const u8 *command, void *, u32 *out, u8 *target);
// 0x6b5c30 (vtable 0x700bb4; the stream at +4, the restore byte at +8): the sound object's load (soundobject, 0x6ba150: out, the stream,
// the byte; the dword's upper bytes the caller's EAX's, which nothing reads).
u32 *__fastcall SoundCmd700bb4_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5c50 (vtable 0x700b9c; the id at +4): the sound object's functor 2 send (soundobject, 0x6b8a10: out, the id).
u32 *__fastcall SoundCmd700b9c_Run(const u8 *command, void *, u32 *out, u8 *target);

}  // extern "C"
