// The input method editor support (vtable 0x6fef04; "Microsoft IME 98" is singled out at
// 0x478fc9): reimplementation of its constructor 0x478b60. Four vectors (a one-byte allocator,
// then first / last / end) at +8, +0x18, +0x28 and +0x38 (the last of 0x24-byte entries), +0x48
// a flag, +0x4c / +0x50, +0x54 (-1 when none) and +0x5c a handle the destructor closes.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x478b60: the vectors empty, the rest cleared (+0x54 = -1); returns ime. The allocator bytes
// take an uninitialised local, which is the top byte of ime as pushed (sic).
u8 *__fastcall Ime_Ctor(u8 *ime);

// Its u32 vectors' helpers (thiscall, the vector unused), constructing in place (a null slot
// skipped, still counted). 0x47c910: [first, last) to dest; returns dest's end.
u32 *__fastcall ImeVector_Copy(void *vec, void *, const u32 *first, const u32 *last, u32 *dest);
// 0x47c940: n copies of *value at dest (*value read for each).
void __fastcall ImeVector_Fill(void *vec, void *, u32 *dest, u32 n, const u32 *value);
// 0x47db00 (cdecl): *slot = *value unless slot is null.
void __cdecl ImeVector_Construct(u32 *slot, const u32 *value);
// 0x47c720 / 0x47c700: a u32 vector's size (+8) / capacity (+0xc) from its first (+4); 0 when
// that is null. 0x47ae40 / 0x47b300: the same for 16-byte entries.
u32 __fastcall ImeVector_Size(const u8 *vec);
u32 __fastcall ImeVector_Capacity(const u8 *vec);
u32 __fastcall ImeVector16_Size(const u8 *vec);
u32 __fastcall ImeVector16_Capacity(const u8 *vec);

// 0x47a960: the current entry (+0x54) of the 0x24-byte list (first at +0x3c), or null for -1.
u8 *__fastcall Ime_Current(const u8 *ime);
// 0x47a9b0: the current entry cached at +0x160 (a 0x24-byte entry: its dwords, the strings at +0x170 and +0x180): with none, +0x188
// and byte +0x184 cleared; else unless cached (byte +0x184) with the same first dword it is assigned over the cache
// (CountedVector14_Assign, core/buffers, inlined), +0x188 set to +0x180 when 0, and marked cached. Returns the cached strings' count
// (0 when none is cached).
u32 __fastcall Ime_CacheCurrent(u8 *ime);
// 0x47a980: +0x58 = limit, at most the constant at 0x6feee0 (12).
void __fastcall Ime_SetLimit(u8 *ime, void *, u32 limit);
// 0x480e70: the constructor of the global at 0x7b4e38, which holds an IME at +0x1c: that
// constructed, +0x17c, +0x190-+0x198, +0 and +8 zeroed, byte +0xc too, +0x10 = -1, and byte
// +0x18c the top byte of host (an unset local, sic). Returns host.
u8 *__fastcall ImeHost_Ctor(u8 *host);

// 0x47c740 (thiscall; position, pointer to the value, pops 8): inserts one value before the position. With room it is built at the end when the position is the end
// (ImeVector_Copy / ImeVector_Fill), else the last is copied up to the end, the rest shifted up and the value assigned over the position; with none the vector is rebuilt in
// a new block of twice the size (one when it was empty), the old block freed. Returns where the value went (first + the position's index).
u32 *__fastcall ImeVector_InsertOne(u8 *vec, void *, u32 *position, const u32 *value);

// 0x47cf20 (thiscall; position, count, pointer to the value, pops 12): inserts count copies. With room: when fewer elements follow the position than count they are copied
// up past the end, the rest of the new room filled from the value and the position's span assigned; otherwise the last count are copied up, the rest shifted and the count
// values assigned. Without: a new block of the larger of count and the size plus the size. Nothing for a count of 0 (with room).
void __fastcall ImeVector_InsertN(u8 *vec, void *, u32 *position, u32 count, const u32 *value);
}
