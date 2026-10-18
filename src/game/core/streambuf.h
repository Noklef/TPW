// Stream buffers (MSVC6 streambuf and wstreambuf): reading and writing through the get and put areas, with the underflow and overflow slots when they run out. Reimplementation of 0x4f0700, 0x4f07c0, 0x4f6740, 0x5ceaf0, 0x5ceba0, 0x5eb1a0, and the wide streambuf's pointer setup, setg and setp (0x4f6020, 0x4f6820, 0x4f6850). Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f0700 (thiscall on the streambuf; destination, count, pops 8): copies up to count wide characters from the get area (the pointer behind the pointer at +0x1c, the
// remaining count (31 bits, sign extended) behind the pointer at +0x2c; both advanced) and, when the area is empty, takes one through slot 5 (offset 0x14) until it returns
// 0xffff. Returns the number copied.
u32 __fastcall Wstreambuf_Read(u8 *object, void *, u16 *destination, s32 count);

// 0x4f07c0 (thiscall; source, count, pops 8): the same into the put area (pointer behind +0x20, count behind +0x30), the character of an empty area given to slot 1 (offset 4)
// with the high half of the put pointer above it (sic), until that returns 0xffff. Returns the number written.
u32 __fastcall Wstreambuf_Write(u8 *object, void *, const u16 *source, s32 count);

// 0x4f6740 (thiscall; n, pops 4): a new block of n * 4 bytes (none below 0) that takes the dwords from the pointer at +0x10 up to (not including) the one past the pointer at
// +0x20, at n rounded down to a multiple of 4 bytes into it; the old block at +0x24 is freed, +0x24 is the new block and +0x28 is n. Returns where the dwords were put.
u32 *__fastcall Streambuf_Regrow(u8 *object, void *, u32 n);

// 0x5ceaf0 / 0x5ceba0: the same for the narrow streambuf (bytes; the room is the signed dword behind the pointer, an underflow / overflow result of -1 ends, and the
// overflow slot is given the byte alone).
u32 __fastcall Streambuf_Read(u8 *object, void *, u8 *destination, s32 count);

u32 __fastcall Streambuf_Write(u8 *object, void *, const u8 *source, s32 count);

// 0x5eb1a0 (thiscall; stream, pops 4): the eleven dwords at +0 .. +0x28 read from the stream (Object_CallSlot8Product with 4, 1). Returns true.
u8 __fastcall Eleven_Read(u8 *object, void *, u8 *stream);

// 0x4f6020 (thiscall): the wide streambuf's pointer slots pointed at its own fields: +0xc -> +4, +0x10 ->
// +8, +0x1c -> +0x14, +0x20 -> +0x18, +0x2c -> +0x24, +0x30 -> +0x28; the six fields they name are
// zeroed. Returns the address of the last (+0x24).
u32 __fastcall Wstreambuf_InitPointers(u8 *object);

// 0x4f6820 / 0x4f6850 (thiscall; the std::basic_string streambuf's setg and setp over wide characters,
// pops 12 / 8): the pointers behind +0xc, +0x1c, +0x2c (first, next, count) / +0x10, +0x20, +0x30
// get first, next and (last - next) / 2 (setg: first, next, last; setp: first, last, with next =
// first). Returns the count pointer.
u32 *__fastcall Wstreambuf_Setg(u8 *object, void *, u32 first, u32 next, u32 last);
u32 *__fastcall Wstreambuf_Setp(u8 *object, void *, u32 first, u32 last);
}  // extern "C"
