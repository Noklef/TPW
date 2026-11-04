// The overlays made from a named folder's .md2 files: reimplementation of 0x4891d0 and 0x4893c0. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/string.h"
#include "overlayload.h"

// Their calls (the game's by default; the bench's recorders).
struct OverlayLoadAllEnv {
    u32(__cdecl *loadRide)(const char *name, const char *folder, const char *c);              // Quickload_LoadRide
    u32(__fastcall *find)(u8 **ref, void *, const u8 *key, u8 **out);                         // NamedList_Find
    u32(__fastcall *first)(u8 *folder, void *, const String *pattern, OverlayFile *out);      // 0x41d570 (B)
    u32(__fastcall *next)(u8 *folder, void *, OverlayFile *out);                              // Class6fd9d8_Find
};
void OverlayLoadAll_SetEnv(const OverlayLoadAllEnv *env);   // nullptr: the game's

extern "C" {

// 0x4891d0 (cdecl): Quickload_LoadRide(0x74fe28, 0x747c7c, 0); the folder named by the String 0x7b9b80 in the registry [0x786b9c] (not found: 0); its first file by the String
// 0x7b9760 (none: 0); then for each file the three loaders in turn with the folder's +0x34 and the String at its +0x18, the first overlay ending the turn. Returns how many overlays.
u32 __cdecl OverlayLoad_All();

// 0x4893c0 (cdecl): the folder named by the String 0x7c2458 (not found: 0); its files by the language's folder name (Language_Buffer), a backslash and the String 0x7b9760. On the
// first file (Quickload_LoadRide(its name, the path, 0x74f89c)) the path is the String at the folder's +0x18, a backslash, the language's name; then a backslash. Each file:
// OverlayLoad_Plain with the folder's +0x34 and a copy of the path, the overlay's +0x80 = 1 (no overlay: that write faults, sic). Returns 0.
u32 __cdecl OverlayLoad_Language();

}  // extern "C"
