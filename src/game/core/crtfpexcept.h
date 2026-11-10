// The C library's floating point exception handling (MSVC6's _87except and the helpers under it): an error record of the math
// functions (type, name, arg1, arg2, retval: doubles at +8, +0x10 and +0x18) handled as the caller's control word says.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x686050: _statfp: the x87 status word, sign extended.
s32 __cdecl CRT_statfp();

// 0x686060: _clrfp: the x87 status word (sign extended), then the exception flags cleared.
s32 __cdecl CRT_clrfp();

// 0x6860b0: _set_statfp: raises (flags) the x87 exception flags by doing an operation that flags each: 1 invalid, 8 overflow, 0x10
// underflow, 4 zero divide, 0x20 precision (the low byte only).
void __cdecl CRT_set_statfp(u32 flags);

// 0x685a20: _handle_exc: the exceptions in flags (8 invalid, 4 zero divide, 1 overflow, 2 underflow, 0x10 precision: one of the first
// four, then precision) that cw masks are flagged on the x87, and an overflow's or underflow's *value replaced by the masked result
// (an overflow by cw's rounding: the C library's infinity 0x781198 or DBL_MAX 0x7811a8; an underflow's *value, scaled by 2^1536,
// denormalised). 1 if no exception is left unmasked.
s32 __cdecl CRT_handle_exc(u32 flags, double *value, u32 cw);

// 0x6856e0: _raise_exc: fills the 0x58-byte _FPIEEE_RECORD at rec from flags, *cw, the x87 status, opcode, *arg1 and *result, raises
// the matching exception code (none in flags: the record's address, sic) through RaiseException with the record's address, then
// takes the masks, rounding and (sic: precision 0 and 1 set the precision field, 2 clears the rounding) back into *cw and *result.
void __cdecl CRT_raise_exc(u32 *rec, u32 *cw, u32 flags, u32 opcode, const double *arg1, double *result);

// 0x68e6f0: _87except: the error record exc (type 1 to 8: domain, singularity, overflow, underflow, total loss, partial loss (no
// exception), 7 (made domain), inexact) through _handle_exc and, unmasked, _raise_exc; then *cw back into the x87, _matherr (when
// 0x781edc is 0 and the type is not 8) and errno.
void __cdecl CRT_87except(s32 opcode, s32 *exc, const u16 *cw);

}
