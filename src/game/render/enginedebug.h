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
// 0x5645e0 (name invented): the same, to the file named at 0x766334, its count 0x879a28.
void __cdecl EngineDebug_LogFallback(const char *text);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[4], gText_Enginedebug_764ff0, 0x00764ff0, 0x4);
GAME_DATA(char[16], gText_EngineDebug_LogFallback_766334, 0x00766334, 0x10);
