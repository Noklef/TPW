// The C library's opening, closing and flushing of files (MSVC6's own names): fopen and its helpers, the low-level open, the handle table's slots, setvbuf, fflush.
#pragma once

#include "crtfile.h"

extern "C" {

// 0x6868a0: _getstream: under lock 2 the first stream of the table [0xfb3cbc] (of [0xfb4cc0]) not in use, locked (one that a thread took meanwhile is passed by); an empty
// slot gets a new one (0x38 bytes; its section at +0x20 made and entered). Its fields cleared, fd -1. None: 0.
CrtFile *__cdecl CRT_getstream();

// 0x6866d0: _openfile: mode: r, w or a, then + b t c n S R T D, the first that repeats a kind ends them; any other first letter: 0. _sopen with permission 0x1a4; its failure: 0. Counts in
// [0xfb1350]. The stream's flags from [0xfb1894] and the mode; its fields cleared, its fd.
CrtFile *__cdecl CRT_openfile(const char *name, const char *mode, u32 share, CrtFile *file);

// 0x67b860, 0x67b8a0: _fsopen, fopen (share 0x40): a stream of _getstream opened by _openfile and unlocked. None free: 0.
CrtFile *__cdecl CRT_fsopen(const char *name, const char *mode, u32 share);
CrtFile *__cdecl CRT_fopen(const char *name, const char *mode);

// 0x68b640: _sopen: CreateFileA by the flags (bad ones: errno 0x16, __doserrno 0; no slot: errno 0x18, __doserrno 0); its slot left unlocked. Read-write text: a last byte 0x1a cut off, the position back at 0.
s32 __cdecl CRT_sopen(const char *path, u32 oflag, u32 shflag, u32 pmode);

// 0x68e810: _alloc_osfhnd: under lock 0x12 the first slot not open, its handle -1, its lock held (the slot's section made under lock 0x11 when missing); a slot that opened meanwhile is passed by. An empty
// block of the table 0xfb3ba0 is made (0x480 bytes; [0xfb3ca0] + 0x20). None: -1.
s32 __cdecl CRT_alloc_osfhnd();

// 0x68e980, 0x68ea30: _set_osfhnd, _free_osfhnd: the slot's handle (free: -1), for 0-2 also SetStdHandle when [0x77ea84] is 1. A bad slot: errno 9, __doserrno 0, -1.
s32 __cdecl CRT_set_osfhnd(u32 fd, u32 handle);
s32 __cdecl CRT_free_osfhnd(u32 fd);

// 0x686300, 0x686370: _close (under the slot's lock), _close_lk: CloseHandle, not for 1 or 2 when they share a handle; the slot freed and its flags 0; a failure by _dosmaperr.
s32 __cdecl CRT_close(u32 fd);
s32 __cdecl CRT_close_lk(u32 fd);

// 0x68f630: _chsize_lk: the file made `size` long: zeros written in binary mode, or SetEndOfFile; the position kept. A failure: errno 0xd.
s32 __cdecl CRT_chsize_lk(s32 fd, s32 size);

// 0x691610: _setmode_lk: flag 0x80 cleared (0x8000) or set (0x4000), the old mode returned; any other: errno 0x16, -1.
s32 __cdecl CRT_setmode_lk(s32 fd, u32 mode);

// 0x686400: _freebuf: a stream in use with its own buffer (flag 8): freed, its fields cleared.
void __cdecl CRT_freebuf(CrtFile *file);

// 0x67cf90: setvbuf: type 4, or 0 and 0x40 with a size 2 - 0x7fffffff (made even), else -1. Under the stream's lock _flush, _freebuf, then its own two bytes (4), the buffer given, or a block made (none: -1, counted in [0xfb1350]).
s32 __cdecl CRT_setvbuf(CrtFile *file, char *buffer, u32 type, u32 size);

// 0x67d0b0, 0x67d070: _fflush_lk: _flush (failed: -1), then with flag 0x4000 _commit, 0 or -1. fflush: under the stream's lock; no stream: flsall(0).
s32 __cdecl CRT_fflush_lk(CrtFile *file);
s32 __cdecl CRT_fflush(CrtFile *file);

// 0x67e830: _dtoxmode: a file's mode from its attributes and path: a path of a root slash, an empty one past any drive, or attribute 0x10: a directory (0x4040), else 0x8000; read 0x100, write 0x80 unless attribute 1; execute 0x40 for .exe, .cmd, .bat, .com by _mbsicmp; each right copied down to group and other.
u32 __cdecl CRT_dtoxmode(u32 attributes, const char *path);
// 0x67ec40: IsRootUNCName: at least 5 bytes, two slashes, then from the fourth byte a name, a slash, a second name that is not empty, and at most one slash that ends the text.
u32 __cdecl CRT_IsRootUNCName(const char *path);

// 0x67b660: _fclose_lk: a stream in use: _flush (its answer the result), _freebuf, _close, then the temporary file's name freed; a failed close -1, that name kept. The flags cleared in every case; not in use: -1.
s32 __cdecl CRT_fclose_lk(CrtFile *file);
// 0x6891b0: _fcloseall: under lock 2, from stream 3 up: those in use closed by fclose and counted when that is not -1; from slot 20 up the slot's section deleted, its block freed and the slot emptied. Returns the count.
s32 __cdecl CRT_fcloseall();

// 0x67d170, 0x67d160: flsall, _flushall (mode 1): under lock 2 every stream in use, locked. Mode 1: all flushed, the count of them that did; mode 0: those being written, -1 if one failed; any other: none, 0.
s32 __cdecl CRT_flsall(s32 mode);
s32 __cdecl CRT_flushall();

// 0x687b20: _commit: FlushFileBuffers under the slot's lock; its failure in __doserrno. A failure or bad slot: errno 9, -1.
s32 __cdecl CRT_commit(u32 fd);

// 0x67e530, 0x67e560: remove, _unlink: DeleteFileA; 0x67ecc0: rename: MoveFileA. A failure with a Win32 error: _dosmaperr, -1; otherwise 0 (a failure with error 0 too).
s32 __cdecl CRT_remove(const char *path);
s32 __cdecl CRT_unlink(const char *path);
s32 __cdecl CRT_rename(const char *from, const char *to);

// 0x687230: _ioinit: the first block of the handle table (0x480 bytes, 0x20 slots: handle -1, flags 0, 0xa at +5; none: _amsg_exit(0x1b), and on past it, sic); the handles
// GetStartupInfoA passes (a count up to 0x800, its flags, its handles; more blocks while they are made) for those open (flag 1) and not -1 (and, without flag 8, of a type);
// then slots 0-2 still -1 from GetStdHandle (flags 0x81, the type's 0x40 for 2, 8 for 3; none: 0x40), the others 0x80 added. SetHandleCount of the table's count.
void __cdecl CRT_ioinit();

// 0x67e750: _initstdio: [0xfb4cc0] streams (0: 0x200, under 0x14: 0x14), the table [0xfb3cbc] from calloc (none: 0x14 again, then _amsg_exit(0x1a)); its first 20 the
// _iob entries 0x77e800-0x77ea80; stdin, stdout, stderr get fd -1 where their handle is -1 or 0.
void __cdecl CRT_initstdio();

// 0x67e810: _endstdio: _flushall, then with [0xfb1398] set _fcloseall.
void __cdecl CRT_endstdio();

// 0x68b9f0: _rmtmp: under lock 2 each stream, locked, in use with a temporary file (+0x1c) by _fclose_lk; the count of them.
s32 __cdecl CRT_rmtmp();

// 0x67bb50: _validdrive: 1 for drive 0 or a root ("X:\") GetDriveTypeA knows (not 0 or 1), else 0.
s32 __cdecl CRT_validdrive(u32 drive);

// 0x67ba20: _getdcwd_lk: drive 0 by GetCurrentDirectoryA, else (an invalid drive: __doserrno 0xf, errno 0xd, 0) by GetFullPathNameA of "X:."; none or over 0x103: 0. No buffer:
// malloc of the larger of the length (with its 0) and size (errno 0xc on a failure); a buffer with a smaller size: errno 0x22, 0. The path copied, the buffer returned.
char *__cdecl CRT_getdcwd_lk(u32 drive, char *buffer, s32 size);

// 0x67b9f0: _getcwd: _getdcwd_lk(0, ...) under lock 0xc.
char *__cdecl CRT_getcwd(char *buffer, s32 size);

}
