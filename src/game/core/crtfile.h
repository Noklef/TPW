// The C library's low-level files and the stream's write side (MSVC6's own names): the handle table ([0xfb3ba0], 0x20 records of 0x24 bytes in each block, the count [0xfb3ca0]: +0 the Win32 handle,
// +4 the flags, +8 its lock made, +0xc the lock), _write, _lseek, _flsbuf, fwrite.
#pragma once

#include <cstdint>

#include "../../../re/types.h"

struct CrtFile {
    char *ptr;
    s32 count;
    char *base;
    u32 flags;
    s32 fd;
    u32 charBuffer;
    s32 size;
    void *name;
};

inline u8 *crt_handle(s32 fd) { return reinterpret_cast<u8 **>((uintptr_t)0x00fb3ba0)[fd >> 5] + (fd & 0x1f) * 0x24; }

// The record, or the dummy record 0x7813b0 for no file.
inline u8 *crt_handle_or_dummy(s32 fd) { return fd == -1 ? reinterpret_cast<u8 *>((uintptr_t)0x007813b0) : crt_handle(fd); }

inline bool crt_handle_open(u32 fd) { return fd < *reinterpret_cast<u32 *>((uintptr_t)0x00fb3ca0) && crt_handle(static_cast<s32>(fd))[4] & 1; }

// A bad handle's errno 9, __doserrno 0: -1.
s32 crt_bad_handle();

extern "C" {

// 0x686a10: __doserrno: the thread's data + 0xc.
u32 *__cdecl CRT_doserrno();

// 0x686980: _dosmaperr: __doserrno the error; errno from the table 0x7811e0, or by its range (0x13-0x24 0xd, 0xbc-0xca 8, others 0x16).
void __cdecl CRT_dosmaperr(u32 error);

// 0x68ead0: _get_osfhandle: the record's handle; a bad or closed one: errno 9, __doserrno 0, -1.
u32 __cdecl CRT_get_osfhandle(u32 fd);

// 0x68eb20 / 0x68eb90: _lock_fhandle (its lock made first, under lock 0x11) / _unlock_fhandle.
void __cdecl CRT_lock_fhandle(s32 fd);
void __cdecl CRT_unlock_fhandle(s32 fd);

// 0x686aa0: _lseek_lk: SetFilePointer on the handle, the end-of-file flag 2 cleared; -1 with errno when it fails.
s32 __cdecl CRT_lseek_lk(u32 fd, s32 offset, u32 origin);

// 0x686a20: _lseek: the same under the handle's lock; a bad handle: errno 9, __doserrno 0, -1.
s32 __cdecl CRT_lseek(u32 fd, s32 offset, u32 origin);

// 0x68c320: _isatty: flag 0x40, 0 for a bad handle.
s32 __cdecl CRT_isatty(u32 fd);

// 0x6864c0: _write_lk: nothing for no bytes. Flag 0x20: to the end first. Flag 0x80 (text): the bytes with \r before \n through a buffer, up to 0x400 at a time, until a short or failed WriteFile; else one
// WriteFile. Returns the bytes written, less the \r's; none: -1 with errno (0x1c, or 9 and __doserrno 5, or mapped), or 0 for a device (flag 0x40) and text starting 0x1a.
s32 __cdecl CRT_write_lk(s32 fd, const char *data, u32 count);

// 0x686440: _write: the same under the handle's lock; a bad handle: errno 9, __doserrno 0, -1.
s32 __cdecl CRT_write(u32 fd, const char *data, u32 count);

// 0x687530: _getbuf: a buffer of 0x1000 from malloc (flag 8), or else the file's own two bytes (flag 4).
void __cdecl CRT_getbuf(CrtFile *file);

// 0x67d0f0: _flush: a written buffer given to _write: flag 0x20 and -1 if short; a file open both ways (0x80) leaves write mode, and all done. The pointer back at the buffer, no count. 0 unless short.
s32 __cdecl CRT_flush(CrtFile *file);

// 0x680ad0: _flsbuf: the byte and the buffer written (made if there is none, but not for a terminal's stdout or stderr). Unwritable or short: flag 0x20, -1. Else the byte as unsigned.
s32 __cdecl CRT_flsbuf(s32 c, CrtFile *file);

// 0x686ff0: _flswbuf: the same for a word: 0xffff, or the word.
u32 __cdecl CRT_flswbuf(u32 c, CrtFile *file);

// 0x67b710: _fwrite_lk: through the buffer, past it in whole buffers, or by _flsbuf. Returns the whole items written.
u32 __cdecl CRT_fwrite_lk(const void *data, u32 size, u32 items, CrtFile *file);

// 0x67b6d0: fwrite: the same under the file's lock.
u32 __cdecl CRT_fwrite(const void *data, u32 size, u32 items, CrtFile *file);

}  // extern "C"
