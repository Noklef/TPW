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

// 0x6419b0 (cdecl (p, n)): the n bytes at p as a big-endian number.
u32 __cdecl ReadBigEndian(const u8 *p, u32 n);

// 0x67850c (cdecl (src, dst, decode); the video library's own copy, in hand-written assembly; name unconfirmed): src decoded into
// dst when src is not null and decode is nonzero. It returns nothing: it reads the header's decoded size into EAX but its POPAD then
// restores the caller's EAX (and the one caller, 0x677914, ignores it).
void __cdecl RefPack_DecodeIf(const u8 *src, u8 *dst, s32 decode);

// 0x6419e0 (cdecl (dst, value, n)): value's low n bytes at dst, the most significant first (the shift is the CPU's: n above 4 wraps).
void __cdecl WriteBigEndian(u8 *dst, u32 value, u32 n);
// 0x641a20 (cdecl (a, b, max)): how many leading bytes of a and b are equal, at most max.
u32 __cdecl RefPack_MatchLength(const u8 *a, const u8 *b, u32 max);

// 0x641a50 (thiscall on the codec (dst, src, len, workspace), pops 16; vtable 0x703f00 slot 1, no caller: the game's codec only
// decodes): src encoded into dst with the plain 0x10FB header and the 3-byte size; greedy LZ over a 128 KB window with CRC-16
// (0x779c60) hash chains in the workspace (0x10000 heads, then 0x20000 links by position & 0x1ffff: 0xc0000 bytes). No workspace:
// one of the codec's slot 3 size from Mem_Alloc, freed after (none got: 0). Returns the encoded size + 5 (sic). The hash reads two
// bytes past the input near its end and the candidate test a byte past the longest match (sic).
u32 __fastcall RefPack_Encode(u8 *codec, void *, u8 *dst, const u8 *src, u32 len, s32 *workspace);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32[256], gCrc16Table, 0x00779c60, 0x400);
