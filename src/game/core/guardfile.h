// The guarded write file, the game's "gwopen" and "gwrite" (from their messages; the rest of the names invented, unconfirmed). Reimplementation of 0x5d4d10, 0x5d4fd0 and 0x5d5350.
// A file of 0x2424 from the guard heap: +0 its FILE, +4 the position, +8 the length, +0xc the file's position of the buffer, +0x10 the buffer's cursor, +0x14 the buffer's end, +0x18 writable, +0x1c the buffer changed, +0x20 the mode ([0x7748f0] at the
// open), +0x24 the name (0x200), +0x224 the name it is written under (0x200), +0x424 the buffer (0x2000). Modes 2 and 3 write to "<name>.tm1" and at the close put that in the name's place, mode 2 keeping the old file as "<name>.bak". Messages go through the
// function pointer at 0x7748f4.
#pragma once

#include "../../../re/types.h"

// The CRT's calls (tests/fncmp/guardfile.py gives recorders).
struct GuardFileEnv {
    s32(__cdecl *stat)(const char *name, void *out);                         // 0x67e900
    void *(__cdecl *fopen)(const char *name, const char *mode);              // CRT_fopen
    s32(__cdecl *fseek)(void *file, s32 offset, s32 origin);                 // CRT_fseek
    s32(__cdecl *fwrite)(const void *data, u32 size, s32 count, void *file);  // CRT_fwrite
    s32(__cdecl *fread)(void *data, u32 size, s32 count, void *file);        // CRT_fread
    s32(__cdecl *fflush)(void *file);                                        // CRT_fflush
    s32(__cdecl *fclose)(void *file);                                        // CRT_fclose
    s32(__cdecl *remove)(const char *name);                                  // 0x67e530
    s32(__cdecl *rename)(const char *from, const char *to);                  // 0x67ecc0
};
void GuardFile_SetEnv(const GuardFileEnv *env);  // nullptr: the game's

extern "C" {
// 0x5d4d10 (stdcall (name), pops 4): "con:": the FILE at 0x77e820 and mode 0. A read-only file or a directory: a message and 0. Opened "w+b", else "wb" with a message. No file: 0. No block: two messages and 0. In the original the working name of "con:" is
// its stack's garbage; here empty.
u8 *__stdcall GuardFile_Open(const char *name);

// 0x5d4fd0 (stdcall (file), pops 4): none: 1. The buffer flushed; with a mode, the FILE closed and the names changed over; the block freed. Answers whether all that worked.
u32 __stdcall GuardFile_Close(u8 *file);

// 0x5d5350 (stdcall (file, data, count), pops 0xc): none: 0; not writable: a message and 0. What fits goes in the buffer. With the buffer's end reached and more to write: the buffer flushed, the part of the file at the last 0x2000 boundary read back in, all
// up to that boundary written straight to the file (short: a message and the count so far), the rest put in the buffer. Answers the count.
s32 __stdcall GuardFile_Write(u8 *file, const char *data, s32 count);
}
