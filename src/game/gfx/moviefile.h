// The movie's file opened, its headers counted, its buffers made and first chunks read, and all that let go again (the settings' +0xa4, mmio).
// Reimplementation of 0x66e080, 0x66e250 and 0x66ef50; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The game's calloc and the next chunk (the bench's recorder and stand-in: MovieChunks_Next has its own bench, and its counts need more reads than a log has room for).
struct MovieFileEnv {
    void *(__cdecl *calloc)(u32 count, u32 size);   // 0x67fcb0
    s32(__fastcall *next)(u8 *player);              // MovieChunks_Next
};
void MovieFile_SetEnv(const MovieFileEnv *env);

extern "C" {

// 0x66e080 (fastcall (file, player)): the clock's +0x3a4 5. Without the settings' flag 0x80000 the file opened into their +0xa4; null, or its buffer not set: 1. The headers
// counted; the settings' +0x7c 1 unless flag 0x4000 and it is one of the counted. Both tables' places cleared, the rings made and cleared (their calloc failing: 2), the counts
// cleared. Chunks read: with a sound (+0x84) until 21 of it (the file's end does not stop this, sic), and the clock 0x8000 / (sound bytes / blocks); without one until 20
// frames or the end. The settings' +0x88 set when no frame counted. 0.
u32 __fastcall MovieFile_Open(const char *file, u8 *player);

// 0x66e250 (thiscall on the player): the chunks SEAD, SCHl, 1SNh at the file's start stepped over, counted in the settings' +0x80, +0x84 set when there are any; the file put
// back. The +0x84. A read that fails ends it.
u32 __fastcall MovieFile_CountHeaders(u8 *player);

// 0x66ef50 (thiscall on the player): the file closed (unless the settings' flag 0x80000, or null), both rings freed, the places cleared in the frames' table and, with a sound,
// in the sound's (the settings' +0x84). 0.
u32 __fastcall MovieFile_Close(u8 *player);
}
