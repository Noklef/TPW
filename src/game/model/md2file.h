// The game's .md2 model files (see tools/md2.py for the layout) read and made ready: reimplementation of 0x46d6d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x46d6d0 (cdecl (name, flags)): the file read whole (VFS_Open, its size from the stream's slot 0x14 and a read through slot 8) into a
// zeroed GlobalMem_Alloc block of size + 4, the model at the block rounded up to 8 (its +0xb0 the block). It is freed again (and 0
// returned; no name or no stream: 0) unless the magic is 0x1cd15d46, the version (+4) 0xdd (or below with flags bit 0), and the motion
// block (+0x98) there without flags bit 1 or absent with it. Then each file offset is made a pointer (0 left as 0; +0x9c the model):
// the header's +0x4c..+0x78, +0x98, +0x7c, +0xac and the 0x14-byte records at +0x7c (+0xc, +0x10; u16 +0x48 of them). With geometry
// (+0x30 bit 0): the materials (+0x54, 0x10 bytes, u16 +0x36: +4 cleared, +0xc; flags bit 2: the word +0xa 1 and bit 0 of +0 cleared),
// the +0x6c block's +0x28 and +0x2c (made pointers even when 0, sic), the 0x10-byte records at +0xac (+8, +0xc; u16 +0x40), the
// geometry nodes (+0x70, 0xa0 bytes, u16 +0x44: +4, +8, +0xc, +0x60, +0x64, +0x68, +0x70, +0x94, the groups +0x6c (0x10 bytes, u16
// +0x5a: +0), +0x54) and the other nodes (+0x74, 0x58 bytes, u16 +0x42 - u16 +0x44 of them: +4, +8, +0xc, +0x54). A motion block is
// then dropped (+0x98 cleared, and +0x30 cleared below sub-version (+8) 0xcb or its bit 5 above), else (sub-version 0xcb) made ready:
// its records at +0x20 (0x10 bytes, u16 +0xc: +8, +0xc), +0x24, +0x28, +0x2c, +0x38, +0x3c, the 8-byte records at +0x30 (u16 +0x18:
// +4, even when 0, sic) and the 0x40-byte tracks at +0x2c (u16 +0x12): +0x18, +0x20, +0x1c, +0x24 (and its +0xc), +0x28 (with +4 bit
// 14: its +0 and +8; else its +0xc's 0x14-byte keys (u16 +2: +4, +8, +0xc) and +0x10), +0x2c (its +4, +0xc, +0x10), +0x30, +0x34.
// Returns the model.
u8 *__cdecl MD2_Load(const char *name, u32 flags);
}
