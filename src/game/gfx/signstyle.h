// A sign's style (SignStyle_Ctor 0x5ebfc0 builds one): reimplementation of what is ported.
#pragma once
#include "../../../re/types.h"

extern "C" {
// 0x5ec3a0 (thiscall on the style; the .sgn file's name; pops 4): the file opened (VFS_Open; AL 0 without a stream) and its version
// read into +0; below 100 (signed) AL 0. Then, each read straight into the style (0x5c4f60): +4, the byte +8 (the background's flag), +0xc
// and +0x10 (the two effects' flags), the first line style (+0x84 0x40 bytes, +0xc4 0x104, +0x40, +0x44, +0x48 0x3c, Eleven_Read at
// +0x358), the second (+0x214, +0x254, +0x1d0, +0x1d4, +0x1d8, Eleven_Read at +0x3c4); each effect whose flag is set (+0x430, +0x444:
// four bytes and four dwords); the two fill strips (Array3D_Read at +0x394 and +0x400); with the background flag, the background at +0x18:
// Array3D_Read for version 100, else Array3D_ReadCompressed through the codec at +0x14. AL 1. The stream is never closed (sic).
u8 __fastcall SignStyle_Load(u8 *style, void *, const char *name);
}
