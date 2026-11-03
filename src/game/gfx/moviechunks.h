// The movie's chunks read from its file (the settings' +0xa4, mmio): the header and tag of each, then the video chunks kept in the frames' ring (+0x1c, +0x24), the sound chunks decoded into the sound's ring (+0x20, +0x28).
// Reimplementation of 0x66e410, 0x66e840, 0x66e920, 0x66e9a0 and 0x66ed60; the names are invented, unconfirmed. The answers: -1 at the file's end (+0x39c set, except for the sound header), -2 on an error or an unknown tag, else the chunk's kind.
#pragma once

#include "../../../re/types.h"

// The decoders (the ports by default; the bench's recorders: their own benches have them).
struct MovieChunksEnv {
    s32 *(__cdecl *mono)(s32 count, s32 *position, s32 *last, s32 *before, const u8 *data, s16 **out);                                        // XaAdpcm_DecodeMono
    s32(__cdecl *stereo)(s32 count, s32 *position, s32 *lastLeft, s32 *beforeLeft, s32 *lastRight, s32 *beforeRight, const u8 *data, s16 **out);   // XaAdpcm_DecodeStereo
    void(__cdecl *ima)(const u8 *data, s16 *out, s32 count, u32 *samples, u32 *indices);                                                      // ImaAdpcm_DecodeStereo
};
void MovieChunks_SetEnv(const MovieChunksEnv *env);

extern "C" {

// 0x66e410 (thiscall on the player): the next chunk: its 8 bytes (a tag, its length with these 8) read; by the tag, the rest read or skipped: a kind 1-7 of video (ReadFrame), the start (ReadStart), the sound header (ReadHeader), a sound chunk,
// skipped (seek; 4), the end (-1); +0x3b4 cleared by all but the sound's and the end. A sound chunk: -2 without a sound (settings' +0x84); else +0x3b4 counted, ReadSound at the settings' +0x7c, else skipped: 2.
s32 __fastcall MovieChunks_Next(u8 *player);

// 0x66e840 (fastcall (player, size), the kind on the stack, popped): the chunk read into the frames' ring (from its start when it would pass +0x124f80); its place, kind and clock (+0x38c) kept in the entry +0x380 % 24 (at +0x1e8, 16 bytes each);
// the ring moved on; +0x380, +0x384 and +0x38c counted. 1.
s32 __fastcall MovieChunks_ReadFrame(u8 *player, u32 size, u32 kind);

// 0x66e920 (fastcall (player, size)): the chunk read and dropped, the sound 16 bits stereo 22050 of the kind 10. 0.
s32 __fastcall MovieChunks_ReadStart(u8 *player, u32 size);

// 0x66e9a0 (fastcall (player, size)): the first chunk's file place less 8 into +0x3b8, when asked (settings' +4 bit 0x8000, +0x390 0); the chunk read into a 0xc000-byte stack buffer (not checked, sic); by the kind +0x64: 0 PCM (a dword count, then
// stereo dwords or mono words doubled), 7 XA (four words of state, then blocks by the channels +0x58), 10 IMA stereo; into the sound's ring (from its start when it would pass +0x493e0); entry +0x378 % 24 (at +0x68): its place, length, the clock
// (+0x38c, counted first with the settings' +0x88), +0x390; the ring and +0x3a0 moved on; +0x378, +0x388 and +0x390 counted. 2. Another kind's length is a stack dword nothing sets in the original (sic; here 0, never seen).
s32 __fastcall MovieChunks_ReadSound(u8 *player, u32 size);

// 0x66ed60 (fastcall (player, size)): the sound's header read (a 0xc000-byte stack buffer) with the defaults 16 bits mono 22050 of the kind 0: from its fifth byte the numbers (MovieTags_ReadNumber) up to the byte 0xfd skipped; then to the byte 0x8a, the tags 0x81 bits
// (+0x60), 0x82 channels (+0x58), 0x83 kind (+0x64), 0x84 rate (+0x5c), others skipped. 0. A read of nothing: -1 (+0x39c not set).
s32 __fastcall MovieChunks_ReadHeader(u8 *player, u32 size);
}
