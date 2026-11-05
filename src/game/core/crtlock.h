// The C library's locks (MSVC6's own names): the table of critical sections at 0x77ece0, filled on first use, and the streams' locks.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x682720: _lock: section `which` entered; a missing one first made (malloc 0x18, or _amsg_exit(0x11), which does not return) under lock 0x11. One made meanwhile by another thread wins: ours is freed.
void __cdecl CRT_lock(s32 which);

// 0x6827a0: _unlock: section `which` left. EAX stays the import's (a caller, 0x6893d7, reads it).
s32 __cdecl CRT_unlock(s32 which);

// 0x6827c0, 0x682830: _lock_file, _unlock_file: a stream of the table __iob (0x77e800-0x77ea60) by its lock 0x1c + index, any other by its own section at +0x20.
void __cdecl CRT_lock_file(void *file);
void __cdecl CRT_unlock_file(void *file);

// 0x682800, 0x682870: _lock_file2, _unlock_file2: stream `index` of the table by its lock 0x1c + index when below 0x14, otherwise by its section.
void __cdecl CRT_lock_file2(s32 index, void *file);
void __cdecl CRT_unlock_file2(s32 index, void *file);

// 0x6826f0: _mtinitlocks: InitializeCriticalSection of locks 0x11, 0xd, 9 and 1.
void __cdecl CRT_mtinitlocks();

}
