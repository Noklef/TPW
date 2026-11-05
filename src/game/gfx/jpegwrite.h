// The game's JPEG writer (reimplementation of 0x64bf90, 0x64c202-0x64c5e9, 0x64dc00-0x64dd5c): an image described by a Windows bitmap-like header, compressed with the statically linked IJG
// libjpeg 6a (src/game/lib/jpeg6a) through a destination manager of the game's that writes to a guarded file. All the names are invented.
#pragma once

#include "../../../re/types.h"

struct AddrCall;
void JpegWrite_SetCalls(const AddrCall *table, u32 count);   // (the bench's recorders)

extern "C" {

// 0x64c202 (stdcall (out, file, unused, unused), pops 16): a zeroed 0x18-byte block from the guarded heap, its +0x10 the file, *out the block; 1. No block: 0, *out as it was.
u32 __stdcall JpegWrite_Open(u8 **out, u32 file, u32 unused1, u32 unused2);

// 0x64c258 (stdcall (handle, info, pixels, pitch), pops 16): the image compressed to handle's file (+0x10): info +0x10 width, +0x14 height, +0x18 bits a pixel, +0x24 colours, +0x28 the palette
// (4 bytes each), +0x428 flags, +0x430 quality. Flag bit 0: four components of unknown colour space, with the marker 0xed of the 9 bytes at 0x77b2d4 after the start; else one of grey when an 8-bit
// image's palette is all grey or every row of any other (as 4 bytes a pixel, whatever its bits, sic) is, three of RGB otherwise. 1 once its rows are written; 0 when the row buffer cannot be had, with
// the compressor finished and destroyed either way.
u32 __stdcall JpegWrite_Image(const u8 *handle, const u8 *info, const u8 *pixels, s32 pitch);

// 0x64bf90 (cdecl): count pixels of one row to the compressor's components. 0x20 bits a pixel (info +0x18), else 8 through the palette at info +0x28: mode 2 RGB (bytes 2, 1, 0), mode 1 the grey of
// byte 1, mode 0 four (3, 2, 1, 0). Another mode: nothing.
void __cdecl JpegWrite_ConvertRow(const u8 *from, u8 *to, s32 count, s32 mode, const u8 *info);

// 0x64c552 (cdecl): 1 when the bytes 0, 1 and 2 of each of count 4-byte entries agree.
u32 __cdecl JpegWrite_IsGrey(const u8 *entries, s32 count);

// 0x64c5c4 (stdcall (handle), pops 4): the handle freed (GuardHeap_Free's answer), 0 without one.
u32 __stdcall JpegWrite_Close(u8 *handle);

// 0x64dc00 (cdecl): the compress struct's destination (+0x14, the 0x1c bytes from its memory manager's alloc_small when it has none) set to the methods 0x64dc5a, 0x64dc9c and 0x64dd00 and the file.
void __cdecl JpegWrite_SetDest(u8 *cinfo, u32 file);

// 0x64dc5a (cdecl), init_destination: the 0x1000-byte buffer (+0x18) from alloc_small (image pool), the next byte and the room left.
void __cdecl JpegDest_Init(u8 *cinfo);

// 0x64dc9c (cdecl), empty_output_buffer: the buffer written; short, ERREXIT 0x24. The buffer empty again; 1.
u32 __cdecl JpegDest_Empty(u8 *cinfo);

// 0x64dd00 (cdecl), term_destination: what the buffer holds written; short, ERREXIT 0x24.
void __cdecl JpegDest_Term(u8 *cinfo);
}
