// The C library's stream reading and its wide characters (MSVC6's own names): _read, _filbuf, fgetc, ungetc, and fgetwc, fputwc, ungetwc over a text file's multibyte characters or a binary file's words.
#pragma once

#include "crtfile.h"

extern "C" {

// 0x688ad0: _read_lk: at end of file (flag 2) nothing. Flags 0x48 (a device or pipe): the byte kept at +5 first. Flag 0x80 (text): flag 4 after a \n first, \r\n to \n in place, 0x1a the end
// (of file unless a device); a \r last of all reads one byte more to decide. Returns the bytes in the buffer, -1 with errno when the read fails (a broken pipe 0x6d: 0).
s32 __cdecl CRT_read_lk(s32 fd, char *data, u32 count);

// 0x688a50: _read: the same under the handle's lock; a bad handle: errno 9, __doserrno 0, -1.
s32 __cdecl CRT_read(u32 fd, char *data, u32 count);

// 0x687440 / 0x687590: _filbuf / _filwbuf: the buffer read again, and its first byte or word. Failing: flag 0x10 at the end of the file, 0x20 for a read error; -1 / 0xffff.
s32 __cdecl CRT_filbuf(CrtFile *file);
u32 __cdecl CRT_filwbuf(CrtFile *file);

// 0x67cb90: fgetc under the file's lock.
s32 __cdecl CRT_fgetc(CrtFile *file);

// 0x67cc10 / 0x67cbe0: _ungetc_lk / ungetc: the byte back in the buffer, or checked against it for a string (flag 0x40). Returns it as unsigned, -1 if it cannot go back.
s32 __cdecl CRT_ungetc_lk(s32 c, CrtFile *file);
s32 __cdecl CRT_ungetc(s32 c, CrtFile *file);

// 0x67ce20 / 0x67cdf0: _getwc_lk / fgetwc: a text file's one or two bytes through mbtowc, or a binary file's word; 0xffff at the end. A read lost after a lead byte puts that back.
u16 __cdecl CRT_fgetwc_lk(CrtFile *file);
u16 __cdecl CRT_fgetwc(CrtFile *file);

// 0x67ca20 / 0x67c9f0: _fputwc_lk / fputwc: a text file's bytes from wctomb, a binary file's word. Returns the character, 0xffff if it cannot go.
u16 __cdecl CRT_fputwc_lk(u32 c, CrtFile *file);
u16 __cdecl CRT_fputwc(u32 c, CrtFile *file);

// 0x67ccd0 / 0x67cca0: _ungetwc_lk / ungetwc: as ungetc, a text file's bytes from wctomb, a binary file's word.
u16 __cdecl CRT_ungetwc_lk(u32 c, CrtFile *file);
u16 __cdecl CRT_ungetwc(u32 c, CrtFile *file);

}  // extern "C"
