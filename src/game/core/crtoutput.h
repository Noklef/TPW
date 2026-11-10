// The C library's formatted output (MSVC6's output.c) and sprintf.
#pragma once

#include "../../../re/types.h"
#include "crtfile.h"

typedef unsigned long long Crt64;

extern "C" {

// 0x681590: write_char: one character to the file, _flsbuf when its count has run out; *written counted, or -1 when that failed. Returns written, or -1.
s32 *__cdecl CRT_output_write_char(s32 c, CrtFile *file, s32 *written);

// 0x6815e0: write_multi_char: the character, count times, stopping when *written is -1. Returns the last *written, or count when not positive.
s32 __cdecl CRT_output_write_multi_char(s32 c, s32 count, CrtFile *file, s32 *written);

// 0x681620: write_string: the same, for count characters of text.
s32 __cdecl CRT_output_write_string(const char *text, s32 count, CrtFile *file, s32 *written);

// 0x681660, 0x681680, 0x683aa0: the next argument: a dword, the low word of one, a 64 bit one.
u32 __cdecl CRT_output_get_int_arg(char **args);
u16 __cdecl CRT_output_get_short_arg(char **args);
Crt64 __cdecl CRT_output_get_int64_arg(char **args);

// 0x680c00: _output: the format to the file. Returns the characters written, or -1. No limit on a precision (sic): over 0x1ff, the digits go past the start of the buffer.
s32 __cdecl CRT_output(CrtFile *file, const char *format, char *args);

// 0x683090: _woutput: _output for a wide format. %c a word, %C (and %hc) a byte through mbtowc; %s wide text, %S (and %hs) bytes counted by lead bytes; no 'l' size (it does
// nothing, sic). Bytes (numbers, floats, narrow text) written as wide characters through mbtowc, stopped by a failed one. Returns the characters written, or -1.
s32 __cdecl CRT_woutput(CrtFile *file, const u16 *format, char *args);

// 0x67a650: sprintf.
s32 __cdecl CRT_sprintf(char *to, const char *format, ...);

// 0x68c350, 0x68c3c0: _aulldiv, _aullrem: the unsigned 64 bit division.
Crt64 __stdcall CRT_aulldiv(Crt64 value, Crt64 divisor);
Crt64 __stdcall CRT_aullrem(Crt64 value, Crt64 divisor);
}
