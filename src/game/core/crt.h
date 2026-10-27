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
    char *(__cdecl *strncpy)(char *to, const char *from, u32 n);   // 0x67ae00
    u32(__cdecl *iswspace)(u32 c);                                 // 0x67a730: iswctype(c, _SPACE), c taken as 16 bits
    u16 *(__cdecl *wcscpy)(u16 *to, const u16 *from);             // 0x67bf10
    // The classifiers index _pctype (0x77ea90) with c as given: callers pass sign-extended chars, so bytes from 0x80 read
    // the words before the table (sic).
    s32(__cdecl *isalpha)(s32 c);                                  // 0x67a530: _pctype[c] & 0x103
    s32(__cdecl *isdigit)(s32 c);                                  // 0x67a560: _pctype[c] & 4
    s32(__cdecl *isspace)(s32 c);                                  // 0x67a590: _pctype[c] & 8
    s32(__cdecl *isalnum)(s32 c);                                  // 0x67a5c0: _pctype[c] & 0x107
    double(__cdecl *atof)(const char *s);                          // 0x67a6c0
    s32(__cdecl *swprintf)(u16 *to, const u16 *format, ...);       // 0x67af00: %s takes a wide string
    u16 *(__cdecl *wcsncpy)(u16 *to, const u16 *from, u32 n);      // 0x67b0d0: n chars, zeros after the end
    s32(__cdecl *wcsicmp)(const u16 *a, const u16 *b);             // 0x67b140: A-Z folded down, the difference of the last units
    u16 *(__cdecl *wcscat)(u16 *to, const u16 *from);              // 0x67bed0
    s32(__cdecl *wcscmp)(const u16 *a, const u16 *b);              // 0x67c290: -1, 0 or 1 (the units compared unsigned)
    s32(__cdecl *strcmp)(const char *a, const char *b);            // 0x680060: -1, 0 or 1 (the bytes compared unsigned)
    // The clock (the driver's stand-in: a fixed time, the same in the emulator's hook, fncmp._time).
    s32(__cdecl *time)(s32 *t);                                    // 0x67d3f0: the seconds since 1970 (UTC), also through t unless null
    // The generator (the driver's stand-in: a fixed answer, the same in the emulator's hook, fncmp._rand).
    s32(__cdecl *rand)();                                          // 0x67b5c0: the thread's seed * 214013 + 2531011, its bits 16-30
};

// The CRT ports call (the game's unless swapped).
const CrtEnv &Crt();

// Swaps it (nullptr: the game's again).
void Crt_SetEnv(const CrtEnv *env);

extern "C" {

// 0x67acf0 (stdcall; array, size, count, ctor, dtor, pops 20): ctor (thiscall on the element, no other arguments) for each of the count (signed) elements of
// size bytes in turn; the dtor is only for the unwinding.
void __stdcall CRT_VectorCtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *), void *dtor);

// 0x401000 (stdcall; array, size, count, ctor, pops 16): the iterator without unwinding: ctor for each element, count - 1
// tested for a sign, so 0 and below do nothing but 0x80000000 runs 0x80000000 times (sic).
void __stdcall CRT_VectorCtorIteratorNoEh(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *));

// 0x67d830 (stdcall; array, size, count, ctor, dtor, pops 20): the iterator for a class with a virtual base: ctor (thiscall on the element, 1
// for most derived; pops 4) for each of the count (signed) elements of size bytes in turn; the dtor is only for the unwinding.
void __stdcall CRT_VectorVbaseCtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *, void *, u32), void *dtor);

// 0x67afa0 (stdcall; array, size, count, dtor, pops 16): dtor (thiscall) for each of the count elements, the last first.
void __stdcall CRT_VectorDtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *dtor)(void *));

}  // extern "C"
