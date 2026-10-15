// The game's MSVC6 C library (all __cdecl, used in the "C" locale), for ports to call.
//
// Ports call it through Crt(), which is the game's own CRT, so formatting and comparisons
// behave as they did. tools/fncmp's driver swaps in stand-ins (Crt_SetEnv), which
// tests/fncmp/crt.py checks against these originals.
#pragma once

#include "../../../re/types.h"

struct CrtEnv {
    s32(__cdecl *stricmp)(const char *a, const char *b);           // 0x67df40: -1, 0 or 1
    char *(__cdecl *strrchr)(const char *s, s32 c);                // 0x67b5f0
    char *(__cdecl *strchr)(const char *s, s32 c);                 // 0x67bfc0
    s32(__cdecl *strncmp)(const char *a, const char *b, u32 n);    // 0x67c250: -1, 0 or 1
    char *(__cdecl *strstr)(const char *s, const char *find);      // 0x67c080
    s32(__cdecl *toupper)(s32 c);                                  // 0x67f1a0
    s32(__cdecl *tolower)(s32 c);                                  // 0x67fa60
    s32(__cdecl *atoi)(const char *s);                             // 0x67a520
    s32(__cdecl *sprintf)(char *to, const char *format, ...);      // 0x67a650
    u32(__cdecl *wcslen)(const u16 *s);                            // 0x67acd0
};

// The CRT ports call (the game's unless swapped).
const CrtEnv &Crt();

// Swaps it (nullptr: the game's again).
void Crt_SetEnv(const CrtEnv *env);
