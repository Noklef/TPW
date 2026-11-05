// The game's libjpeg data source (jpeg_stdio_src's shape over the game's stream): reimplementation of 0x631140, 0x6311b0, 0x6311c0, 0x631230. The source manager:
// libjpeg's five methods and pointers, +0x1c the stream, +0x20 the 0x1000-byte buffer, +0x24 start of file. Names invented; the shape is jdatasrc.c's.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x631140 (cdecl): the source manager allocated when the decompressor has none (its memory manager's alloc_small, twice), filled for the stream, and empty.
void __cdecl JpegSource_Set(u8 *cinfo, void *stream);

// 0x6311b0 (cdecl): init_source: start of file.
void __cdecl JpegSource_Init(u8 *cinfo);

// 0x6311c0 (cdecl): fill_input_buffer: 0x1000 bytes from the stream's slot 8. None: at the start of the file ERREXIT 0x2a (the code after it runs when error_exit returns);
// WARNMS 0x78 and a fake EOI. Returns 1.
u8 __cdecl JpegSource_Fill(u8 *cinfo);

// 0x631230 (cdecl): skip_input_data: fills past the data skipped.
void __cdecl JpegSource_Skip(u8 *cinfo, s32 count);

}  // extern "C"
