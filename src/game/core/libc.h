// The game's __stdcall wrappers around the C library: reimplementation of 0x5f76c0-0x5f78c0.
//
// strlen, strcpy and strcat are inline in the originals (REPNE SCASB, then REP MOVSD and
// REP MOVSB: the whole string with its NUL, a dword at a time, then the last bytes); the rest
// call the game's MSVC6 CRT, which they still do here (LibcEnv), so that formatting and
// conversions behave as they did.
#pragma once

#include "../../../re/types.h"

extern "C" {
u32 __stdcall Lib_StrLen(const char *s);                              // 0x5f76c0
char *__stdcall Lib_StrCpy(char *dst, const char *src);               // 0x5f76e0
char *__stdcall Lib_StrNCpy(char *dst, const char *src, u32 count);   // 0x5f7710
char *__stdcall Lib_StrCat(char *dst, const char *src);               // 0x5f7730
char *__stdcall Lib_StrNCat(char *dst, const char *src, u32 count);   // 0x5f7770
s32 __stdcall Lib_AtoI(const char *s);                                // 0x5f7790
// 0x5f77a0: iswctype(c sign-extended to 16 bits, _DIGIT); the original leaves the upper
// half of the argument as it found EAX, which iswctype ignores.
u32 __stdcall Lib_IsDigit(char c);
s32 __stdcall Lib_VSPrintf(char *dst, const char *format, void *args);             // 0x5f77c0
s32 __stdcall Lib_VSNPrintf(char *dst, u32 size, const char *format, void *args);  // 0x5f77e0
u32 __stdcall Lib_WcsLen(const u16 *s);                               // 0x5f7800
u16 *__stdcall Lib_WcsCpy(u16 *dst, const u16 *src);                  // 0x5f7810
u16 *__stdcall Lib_WcsNCpy(u16 *dst, const u16 *src, u32 count);      // 0x5f7830
u16 *__stdcall Lib_WcsCat(u16 *dst, const u16 *src);                  // 0x5f7850
u16 *__stdcall Lib_WcsNCat(u16 *dst, const u16 *src, u32 count);      // 0x5f7870
s32 __stdcall Lib_WtoI(const u16 *s);                                 // 0x5f7890
u32 __stdcall Lib_IsDigitW(u32 c);                                    // 0x5f78a0: iswctype(c, _DIGIT)
s32 __stdcall Lib_VSWPrintf(u16 *dst, const u16 *format, void *args); // 0x5f78c0
}

// The game's CRT functions the wrappers call (all __cdecl).
struct LibcEnv {
    char *(__cdecl *strncpy)(char *, const char *, u32);             // 0x67ae00
    char *(__cdecl *strncat)(char *, const char *, u32);             // 0x67b8c0
    s32(__cdecl *atoi)(const char *);                                // 0x67a520
    u32(__cdecl *iswctype)(u32 c, u32 mask);                         // 0x67f330
    s32(__cdecl *vsprintf)(char *, const char *, void *);            // 0x67c1e0
    s32(__cdecl *vsnprintf)(char *, u32, const char *, void *);      // 0x67f3a0
    u32(__cdecl *wcslen)(const u16 *);                               // 0x67acd0
    u16 *(__cdecl *wcscpy)(u16 *, const u16 *);                      // 0x67bf10
    u16 *(__cdecl *wcsncpy)(u16 *, const u16 *, u32);                // 0x67b0d0
    u16 *(__cdecl *wcscat)(u16 *, const u16 *);                      // 0x67bed0
    u16 *(__cdecl *wcsncat)(u16 *, const u16 *, u32);                // 0x67f450
    s32(__cdecl *wtoi)(const u16 *);                                 // 0x67f4a0
    s32(__cdecl *vswprintf)(u16 *, const u16 *, void *);             // 0x67f4d0
};
// The wrappers' calls go to env (nullptr: the game's).
void Libc_SetEnv(const LibcEnv *env);
