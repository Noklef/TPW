// EA RefPack (QFS, magic 0x10FB) decompression, used for .wad entries of type 4.
// See tools/wadtool.py for the Python version.
//
// In the game both functions are virtual methods of a codec object (vtable 0x703f00, slots
// 2 and 5) that never read `this`, and they pop their own arguments (ret 0xc / ret 4), so
// __stdcall reproduces the ABI for virtual and direct callers alike.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x641f10: decodes src into dst and returns the decoded size from the header. Like the
// original it trusts the stream: no bounds checks, and dst must hold the full size.
// Flag 0x01 in the first byte means a 3-byte compressed size precedes the decoded size;
// sizes are always 3 bytes (the 0x80 "4-byte sizes" flag is not honoured).
// The third argument is ignored (the one direct caller, 0x638be0, passes 0).
s32 __stdcall RefPack_Decode(u8 *dst, const u8 *src, s32 unused);

// 0x642110: the decoded size, read from after the 2-byte magic when it is exactly 0x10FB,
// else after 5 bytes.
u32 __stdcall RefPack_DecodedSize(const u8 *src);
}

// 0x641a50 (vtable slot 1), not hooked: nothing in the game constructs a codec that encodes,
// so this is for tools. Same output as the original byte for byte (it reproduces every
// compressed entry of the shipped .wad files), always with the plain 0x10FB header.
// dst must hold RefPack_EncodeBound(len) bytes. Returns the encoded size; note the
// original returned that plus 5.
u32 RefPack_Encode(u8 *dst, const u8 *src, u32 len);
// Literal runs cost one byte per 112, every match saves at least one, plus header and end.
inline u32 RefPack_EncodeBound(u32 len) { return len + len / 112 + 8; }
