// The wavelet codec's bit reader and a record flag lookup: reimplementation of 0x597090 and 0x59c610. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x597090 (cdecl; count): the next `count` bits, least significant first, from the stream of 16-bit words at [0xbe6800] + [0xe74a40]: when fewer bits than count are held
// ([0xaa67f8]) the next word (the position at [0xbe6800] advanced by 2) goes above them in the accumulator at [0xf74a48] and the held count grows by 16. Shift counts use
// their low 5 bits.
u32 __cdecl Wavelet_ReadBits(u32 count);

// 0x59c610 (stdcall; id): the 0x38-byte records from 0x76e300 hold their id at +4; false for the ids 0x15f and 0x160 and for an id found nowhere (the table is searched up to
// 0x772fcc; a printed message is left out); otherwise whether the dword at +8 of the record at id * 0x38 is 1 (the record is that one even when the search found the id
// elsewhere, sic).
u8 __stdcall Record_IsActive(u32 id);

}  // extern "C"
