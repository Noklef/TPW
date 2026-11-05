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

// 0x67e570: fputc: under the file's lock the byte put, or _flsbuf when the count has run out.
s32 __cdecl CRT_fputc(s32 c, CrtFile *file);
// 0x68b490: _input's _inc: the next byte, by _filbuf at the buffer's end. 0x68b4c0: _un_inc: back by _ungetc_lk, unless -1.
s32 __cdecl CRT_input_inc(CrtFile *file);
s32 __cdecl CRT_input_un_inc(s32 c, CrtFile *file);
// 0x67dad0: fprintf: under the file's lock _output between _stbuf and _ftbuf.
s32 __cdecl CRT_fprintf(CrtFile *file, const char *format, ...);
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

// 0x67e210 / 0x67e1d0: _fread_lk / fread: from the buffer, whole chunks directly through _read, a part through _filbuf; end of file sets flag 0x10, a read's failure 0x20. Returns the whole items.
u32 __cdecl CRT_fread_lk(char *data, u32 size, u32 count, CrtFile *file);
u32 __cdecl CRT_fread(char *data, u32 size, u32 count, CrtFile *file);

// 0x67e380 / 0x67e350: _ftell_lk / ftell: the handle's place corrected by what is in the buffer, a text file's line ends counted twice. A negative count is first set to 0.
s32 __cdecl CRT_ftell_lk(CrtFile *file);
s32 __cdecl CRT_ftell(CrtFile *file);

// 0x67c140: _fseek_lk: the flags have no read, write or both, or the origin is not 0, 1 or 2: errno 0x16, -1. Origin 1 goes by _ftell_lk. The buffer flushed; a both-ways file forgets its direction,
// a read file with its own buffer (not 0x400) has a buffer size of 0x200 again. -1 if _lseek fails, else 0. 0x67c100: fseek: the same under the file's lock.
s32 __cdecl CRT_fseek_lk(CrtFile *file, s32 offset, s32 origin);
s32 __cdecl CRT_fseek(CrtFile *file, s32 offset, s32 origin);

// 0x688610: _stbuf: stdout (0x77e820) or stderr (0x77e840) on a device gets a temporary buffer: its slot at 0xfb1540's block of 0x1000 from malloc, kept there, or 2 bytes in the file itself. 1 if given. [0xfb1350] counts the tries.
s32 __cdecl CRT_stbuf(CrtFile *file);

// 0x6886d0: _ftbuf: a flag and a file with a temporary buffer (0x1000): flushed and given up.
void __cdecl CRT_ftbuf(s32 flag, CrtFile *file);

// 0x6839f0: _woutput's write_char: _fputwc_lk; *count -1 if it could not go, else one more.
void __cdecl CRT_woutput_write_char(u32 c, CrtFile *file, s32 *count);
// 0x683a20, 0x683a60: write_char n times, or over a text; stopped by a count of -1.
void __cdecl CRT_woutput_write_multi(u32 c, s32 n, CrtFile *file, s32 *count);
void __cdecl CRT_woutput_write_string(const u16 *text, s32 n, CrtFile *file, s32 *count);
// 0x68b4e0: _whiteout: the next character that is no space; the counter goes up by one for each read.
s32 __cdecl CRT_whiteout(s32 *counter, CrtFile *file);
s32 __cdecl CRT_printf(const char *format, ...);  // 0x67e6f0

}  // extern "C"
