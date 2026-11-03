// The engine's debug log, enginedebug.txt: each text written by opening, writing and closing the file, made anew on the first write of a run. Reimplementation of 0x564650.
#pragma once

#include "../../../re/types.h"

// The game's C library file calls (the driver's: recorders, so no bench writes a file).
struct EngineDebugEnv {
    void *(__cdecl *fopen)(const char *name, const char *mode);   // 0x67b8a0
    s32(__cdecl *fprintf)(void *file, const char *format, ...);   // 0x67dad0
    s32(__cdecl *fflush)(void *file);                             // 0x67d070
    s32(__cdecl *fclose)(void *file);                             // 0x67b620
};
// nullptr restores the game's.
void EngineDebug_SetEnv(const EngineDebugEnv *e);

extern "C" {

// 0x564650 (cdecl (text); name unconfirmed): enginedebug.txt opened "a", or "w+b" while the count 0x879a2c is 0 (which then goes up, opened or not); opened, "%s\n" of text
// written and flushed and the file closed.
void __cdecl EngineDebug_Log(const char *text);
}
