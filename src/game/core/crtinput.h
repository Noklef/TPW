// The C library's formatted input (MSVC6's input.c and sscanf.c).
#pragma once

#include "crtfile.h"

extern "C" {

// 0x68a710: _input: the format's directives against the file's characters (by _inc, _un_inc and _whiteout); %[ sets in a 256-bit table, %s and %c by the sets at
// 0x781710 and 0x781718, integers 32 or 64 bit (I64), floats through _fassign ([0x77eca8]) from a text of at most 0x15d characters. Returns the fields assigned,
// -1 at the end of the input before any field matched.
s32 __cdecl CRT_input(CrtFile *file, const u8 *format, char *args);
// 0x67f620: sscanf: _input over a string file (flags 0x49) on the text.
s32 __cdecl CRT_sscanf(const char *text, const char *format, ...);
}
