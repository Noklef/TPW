// The voice-coded sound samples (the names are invented): sound streams (src/game/core/soundstream, src/game/core/soundsample) whose
// data is the LPC speech codec of src/game/core/voicecodec, 0x70ad60 over memory and 0x70adc0 over a sub-stream (src/game/files/substream)
// embedded at +0x40. +0x10 the data (the open's argument), +0x2c the codec's bit reader (0x70adb8 over memory, 8 bytes; 0x70ae28 over the
// sub-stream, 0x200c bytes), +0x30 the samples of the current frame played (0x1b0: all, the next one decoded first), +0x34 bit 0 mono,
// +0x38 the decoder (its first dword the reader's byte pointer; its channels' samples floats from +0x688, 0xd34 bytes apart), +0x3c the
// data's end. Slots: 2 the close, 7 the read, 8 the restart, 9 a query, 20 the open. Reimplementation of 0x6c7bc0-0x6c7dfa, 0x6c80c0 and
// 0x6c8120; the destructors 0x6c8330, 0x6c8000, 0x6c7ba0 and 0x6c7fe0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c7cf0 (thiscall (buffer, bytes, done), pops 0xc): *done = 0; frames of the channels (slot 17's low byte; an unsigned divide of
// bytes / 2) until the bytes are used: a frame's samples all played decodes the next (VoiceCodec_DecodeFrame, +0x30 = 0), 1 when the
// reader's pointer has then reached the end (+0x3c, unsigned); each channel's sample rounded (fistp), 17 bits kept, those in
// 0x8000-0x17fff clamped to 0x7fff / 0x8000 (sic: the rest wrap), stored as 16 bits, *done += 2. Returns 0.
u32 __fastcall VoiceSample_Decode(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *done);
// 0x6c7c20 (thiscall (buffer, bytes, done, flag), pops 0x10; slot 7): with an end time (+0x1c): *done = 0, SoundSample_Drained. Else
// VoiceSample_Decode; its 1 (the data used up) asks slot 19 (flag, *done): its 0 returns 1. Else 0.
u32 __fastcall VoiceSample_Read(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *done, u32 flag);
// 0x6c7bc0 (thiscall (out, record), pops 8; slot 9): slot 20 (+0x10) opens it again; *out = 0. Returns the out.
u32 *__fastcall VoiceSample_Query(u8 *sample, void *, u32 *out, u8 *record);
// 0x6c7ce0 (thiscall; slot 8 of 0x70ad60): slot 20 (+0x10) opens it again.
void __fastcall VoiceSample_Restart(u8 *sample);
// 0x6c7be0 (thiscall; slot 2): the decoder (+0x38) and the reader (+0x2c) freed (Mem_Free) and forgotten, then SoundStream_Close.
void __fastcall VoiceSample_Close(u8 *sample);
// 0x6c7c80 (thiscall (data), pops 4; slot 20 of 0x70ad60): without a reader, a new memory reader (Mem_Alloc(8): 0x70adb8, 0; none: 0);
// the frame header from the data (VoiceCodec_ReadHeader into the decoder, stereo unless +0x34 bit 0); +0x30 = 0x1b0.
void __fastcall VoiceSample_Open(u8 *sample, void *, const u8 *data);
// 0x6c80c0 (thiscall (unused), pops 4; slot 20 of 0x70adc0): without a reader, a new buffered reader over the sub-stream at +0x40
// (Mem_Alloc(0x200c): 0x70ae28, +4 0, +0x2008 the sub-stream; none: 0); VoiceCodec_ReadHeaderBuffered as VoiceSample_Open; +0x30 = 0x1b0.
void __fastcall VoiceStream_Open(u8 *sample, void *, u32 unused);
// 0x6c8120 (thiscall; slot 8 of 0x70adc0): the sub-stream rewound (SubStream_Rewind), then slot 20 (+0x10).
void __fastcall VoiceStream_Restart(u8 *sample);
// 0x6c8330 (thiscall; the destructor of 0x70ad60): a jump to SoundSample_Dtor.
void __fastcall VoiceSample_Dtor(u8 *sample);
// 0x6c8000 (thiscall; the destructor of 0x70adc0): the sub-stream at +0x40's (vtable 0x70ad50; Semaphore_Dtor (+0x14), StreamBase_Dtor
// (+0xc)), then SoundSample_Dtor.
void __fastcall VoiceStream_Dtor(u8 *sample);
// 0x6c7ba0 / 0x6c7fe0 (thiscall (flags), pops 4; slot 0): the destructor; with flags bit 0, the block back to its pool
// (VoiceSamplePool_Delete (0x40) / VoiceStreamPool_Delete (0x5c), core/streampool). Returns the sample.
u8 *__fastcall VoiceSample_DeletingDtor(u8 *sample, void *, u32 flags);
u8 *__fastcall VoiceStream_DeletingDtor(u8 *sample, void *, u32 flags);
// 0x6c7b20 (thiscall (record, source), pops 8): SoundSample_Ctor, +0x2c = +0x34 = 0, a decoder of 0x1a7c bytes (Mem_Alloc) at +0x38,
// the vtable 0x70ad60, the data's end +0x3c = the record's +4 + the data (+0x10); bit 2 of the record's +0x1b sets +0x34 bit 0 (mono).
// Returns the sample.
u8 *__fastcall VoiceSample_Ctor(u8 *sample, void *, const u8 *record, u8 *source);
// 0x6c7f70 (thiscall (stream, record, source), pops 0xc): VoiceSample_Ctor (record, source), SubStream_Ctor (+0x40, stream), its
// vtable 0x70ae18, the sample's 0x70adc0. Returns the sample.
u8 *__fastcall VoiceStream_Ctor(u8 *sample, void *, const u8 *stream, const u8 *record, u8 *source);

// 0x6c8070 (thiscall (buffer, count), pops 8; slot 1 of 0x70ae18, the window at +0x40 of the voice stream sample (0x70adc0); name
// invented): SubStream_Read (buffer, the count or, when fewer, the bytes left: the sample's length (+0x18, this - 0x28) less the position
// (+4)); once the position is at or past the length, the sample's +0x3c (this - 4) = the answer + the buffer's address (sic) and bit 1 of
// its +0x34 (this - 0xc) set. Returns SubStream_Read's answer.
u32 __fastcall VoiceStreamWindow_Read(u8 *window, void *, u8 *buffer, u32 count);

}  // extern "C"
