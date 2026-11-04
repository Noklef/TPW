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
    // The generator (the driver's stand-in: the same on a seed at fncmp.RAND_SEED, as the emulator's hook, fncmp._rand).
    s32(__cdecl *rand)();                                          // 0x67b5c0: the thread's seed (_getptd +0x14) * 214013 + 2531011, bits 16-30
    // The heap (the driver's stand-in: fncmp's heap, as the emulator's hook a bench gives, fncmp._free).
    void(__cdecl *free)(void *block);                              // 0x67ad90: free (the CRT heap's, under its lock)
    // 0x67c8d0: _wcsnicmp: up to n units, A-Z folded down (the "C" locale's path); the difference of the last units compared (0 for n 0).
    s32(__cdecl *wcsnicmp)(const u16 *a, const u16 *b, u32 n);
};

// The CRT ports call (the game's unless swapped).
const CrtEnv &Crt();

// Swaps it (nullptr: the game's again).
void Crt_SetEnv(const CrtEnv *env);

// What the ported CRT routines below call: the rest of the game's CRT, by address (tools/fncmp's driver gives recorders).
struct CrtInnerEnv {
    u8 *(__cdecl *getptd)();                                     // 0x682460: the thread's data (+0x14 the rand seed)
    void(__cdecl *lockExit)();                                   // 0x67fa20: lock 0xd
    void(__cdecl *unlockExit)();                                 // 0x67fa30
    u32(__cdecl *msize)(void *block);                            // 0x6818f0
    void *(__cdecl *realloc)(void *block, u32 size);             // 0x681740
    void(__cdecl *lock)(s32 which);                              // 0x682720
    void(__cdecl *unlock)(s32 which);                            // 0x6827a0
    void *(__cdecl *sbhFindBlock)(void *block, void **region, void **page);     // 0x682b40
    void(__cdecl *sbhFreeBlock)(void *region, void *page, void *entry);         // 0x682ba0
    void(__cdecl *lockFile)(void *file);                         // 0x6827c0
    void(__cdecl *unlockFile)(void *file);                       // 0x682830
    s32(__cdecl *fcloseLocked)(void *file);                      // 0x67b660
    void(__cdecl *memFree)(void *block);                         // 0x67acb0
    void *(__cdecl *memAlloc)(u32 size);                         // 0x67acc0
    void *(__cdecl *nhMalloc)(u32 size, u32 newMode);            // 0x67dcf0
    u16(__cdecl *towlowerLocked)(u32 unit);                      // 0x67e670: only the low 16 bits count
    s32(__cdecl *woutput)(void *file, const u16 *format, void *args);   // 0x683090
    s32(__cdecl *flsbuf)(s32 c, void *file);                     // 0x680ad0
    s32(__cdecl *mbrtowc)(u32 *wc, const u32 *text, u32 n, u32 *state, void *conversion);   // 0x693bc0
    void *(__cdecl *calloc)(u32 count, u32 size);                // 0x67fcb0
    void(__cdecl *initptd)(void *data);                          // 0x682440
    void(__cdecl *amsgExit)(s32 code);                           // 0x67efc0
    void(__cdecl *dosmaperr)(u32 error);                         // 0x686980
    void(__cdecl *tzset)();                                      // 0x687d80
};

void CrtInner_SetEnv(const CrtInnerEnv *env);

const CrtInnerEnv &CrtInner();

extern "C" {

// 0x67b410: _isnan, of the double's bits: all exponent bits set and any mantissa bit.
s32 __cdecl CRT_isnan(u32 low, u32 high);

// 0x67c320 (__alldiv; stdcall, pops 16): the signed 64-bit quotient (a zero divisor faults).
long long __stdcall CRT_alldiv(u32 aLow, u32 aHigh, u32 bLow, u32 bHigh);

// 0x67b5c0: rand: the thread's seed (getptd +0x14) * 214013 + 2531011 stored, bits 16-30 of it.
s32 __cdecl CRT_rand();

// 0x67a740: _onexit, under the exit lock: fn appended to the table [0xfb3cb4] up to its end [0xfb3cb0] (the block grown by 0x10 bytes when msize has
// no room for one more). Returns fn; 0 when the growth fails.
void *__cdecl CRT_onexit(void *fn);

// 0x67a7d0: atexit: 0 when _onexit took fn, else -1.
s32 __cdecl CRT_atexit(void *fn);

// 0x67ad90: free: not null: under lock 9 the small block heap's block freed, or else (past the lock) HeapFree on the CRT's heap [0xfb3cac].
void __cdecl CRT_free(void *block);

// 0x67b620: fclose: a string stream's (flag 0x40 in +0xc) flags cleared and -1; else _fclose_lk under the file's lock.
s32 __cdecl CRT_fclose(void *file);

// 0x67d320 (thiscall): exception's destructor: the vtable 0x7089c4 set, the text +4 freed if it is owned (+8).
void __fastcall CRT_exception_Dtor(u32 *self);

// 0x67d250 (thiscall (&text), pops 4): exception(const char *const &): the vtable, a copy of the text +4 (0 if the allocation fails), owned +8. Returns self.
u32 *__fastcall CRT_exception_Ctor(u32 *self, void *edx, const char *const *text);

// 0x67d2b0 (thiscall (from), pops 4): the copy constructor: an owned text copied (0 if the allocation fails), an unowned one shared. Returns self.
u32 *__fastcall CRT_exception_Copy(u32 *self, void *edx, const u32 *from);

// 0x67dcd0: malloc: _nh_malloc with the new mode [0xfb1548].
void *__cdecl CRT_malloc(u32 size);

// 0x682460: _getptd: the thread's data in its TLS slot [0x77ecd0], or a block of 0x74 made, set and _initptd'd (+0 the thread's id, +4 -1); no block or
// no slot: _amsg_exit(0x10), and past it (when it returns) the block as it is. GetLastError's value is kept across it.
void *__cdecl CRT_getptd();

// 0x686a00: _errno: the thread's data + 8.
s32 *__cdecl CRT_errno();

// 0x67e050: _beginthread: a block of 0x74 _initptd'd (+0x48 proc, +0x4c arg, +4 the handle) for a suspended thread at 0x67e0d0, resumed. Returns the handle. No block,
// no thread or no resume: the block freed (a null too), the Win32 error mapped when there is one, -1.
u32 __cdecl CRT_beginthread(void *proc, u32 stack, void *arg);

// 0x67af00: swprintf: _woutput on a string file (0x7fffffff bytes), then two zero bytes put (_flsbuf where the count has run out). Returns _woutput's answer.
s32 __cdecl CRT_swprintf(u16 *to, const u16 *format, ...);

// 0x67c830: _snwprintf: the same in count * 2 bytes.
s32 __cdecl CRT_snwprintf(u16 *to, u32 count, const u16 *format, ...);

// 0x67b140: _wcsicmp: with no locale handle [0xfb1440] A-Z folded down; else _towlower_lk's fold, inside the locale's lock (the reader count 0xfb3ca8, or lock 0x13 while
// the locale is being set, [0xfb3ca4]). The difference of the last units.
s32 __cdecl CRT_wcsicmp(const u16 *a, const u16 *b);

// 0x67c8d0: _wcsnicmp: the same up to n units (0 for n 0).
s32 __cdecl CRT_wcsnicmp(const u16 *a, const u16 *b, u32 n);

// 0x693dc0: btowc: EOF: -1 as it is (the original only sets AX). Else _Mbrtowc on the argument's low byte: its unit, 0xffff if it answers below 0.
u32 __cdecl CRT_btowc(u32 c);

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

// 0x6d76f0 (cdecl; called from 0x4901ff): every unit of the wide string before its terminator set to the unit. Returns the string.
u16 *__cdecl CRT_wcsset(u16 *text, u16 unit);

// The C++ library's conversions under the C library's locale (names are the library's own). A conversion block is { locale id, code page }; none: the current ones (0xfb1440, 0xfb1450).

// 0x69a560: _Getcvt: the current block, by value in EDX:EAX, under the locale's lock.
unsigned long long __cdecl CRT_Getcvt();

// 0x699bd0 (cdecl (&result)): _Getctype: { the LC_CTYPE id (0xfb143c), the code page, a copy of the table _pctype (0x200 bytes from malloc), 1 } or, with no block, the table itself and 0. Returns &result.
u32 *__cdecl CRT_Getctype(u32 *out);

// 0x69a4e0 (cdecl (to, unit, unused, conversion)): the unit as bytes. Id 0: units to 0xff as one byte; else WideCharToMultiByte for _mb_cur_max bytes. Fails (a loss of data, or a unit too big) with errno 0x2a and -1.
s32 __cdecl CRT_WcrtombLocked(char *to, u32 unit, void *unused, const u32 *conversion);

// 0x69a460 (cdecl (to, unit, state, conversion)): _Wcrtomb: the same under the locale's lock (the state is not used).
s32 __cdecl CRT_Wcrtomb(char *to, u32 unit, void *state, const u32 *conversion);

// 0x693c40 (cdecl (&unit, text, n, state, conversion)): one character of up to n bytes as a unit: 0 for no text, no n or a zero byte (*unit 0), 1 under id 0, a lead byte kept in the state with too few bytes (-2);
// a kept lead byte joined with this byte, else _mb_cur_max bytes for a lead byte (of the table _pctype), else one: MultiByteToWideChar's answer, _mb_cur_max or 1. Fails with errno 0x2a and -1.
s32 __cdecl CRT_MbrtowcLocked(u16 *unit, const char *text, u32 n, u8 *state, const u32 *conversion);

// 0x6871b0 / 0x687140: _wctomb_lk / wctomb: the word as the locale's bytes (no locale: as itself, up to 0xff). No buffer: 0. Not representable: errno 0x2a, -1.
s32 __cdecl CRT_wctomb_lk(char *out, u32 c);
s32 __cdecl CRT_wctomb(char *out, u32 c);

// 0x687710 / 0x687690: _mbtowc_lk / mbtowc: one character of the text as a word, returning its bytes: 0 for no text or a null, -1 and errno 0x2a if bad. A lead byte in a bad conversion still counts as the
// whole character when another byte follows.
s32 __cdecl CRT_mbtowc_lk(u16 *out, const char *text, u32 size);
s32 __cdecl CRT_mbtowc(u16 *out, const char *text, u32 size);

// 0x693bc0: _Mbrtowc: the same under the locale's lock.
s32 __cdecl CRT_Mbrtowc(u16 *unit, const char *text, u32 n, u8 *state, const u32 *conversion);

// 0x67b5f0: strrchr: the last c (its low byte; 0 finds the terminator), else null.
char *__cdecl CRT_strrchr(const char *text, s32 c);

// 0x67d7f0: strpbrk: the first byte of the text in the set, else null.
char *__cdecl CRT_strpbrk(const char *text, const char *set);

// 0x67d3a0 (stdcall, pops 8): _CxxThrowException: RaiseException with the record at 0x7089e8 (3 arguments: its magic, the object, the type's info).
void __stdcall CRT_CxxThrowException(void *object, const void *info);

// 0x689180: strncnt: the bytes before a terminator within n; n when there is none (the byte at n is read too).
u32 __cdecl CRT_strncnt(const char *text, u32 n);

// 0x68a5d0: __crtGetStringTypeA: which call works is found once (0xfb1890: 1 GetStringTypeW, 2 GetStringTypeA; neither: 0). 2: GetStringTypeA under the id (or the current 0xfb1440). 1: the text
// through MultiByteToWideChar (code page or the current 0xfb1450; a block of calloc(2, n)) to GetStringTypeW; 0 if a step fails. Any other value there is returned (sic).
s32 __cdecl CRT_GetStringTypeA(u32 info, const char *text, s32 count, u16 *types, u32 page, u32 id, s32 strict);

// 0x688f50: __crtLCMapStringA: the same choice (0xfb1554, LCMapStringW or A). 2: LCMapStringA. 1: the text widened, mapped, and with flag 0x400 (a sort key) the bytes given as they are, else
// the units narrowed. The size written or needed; 0 if a step fails. Any other value there is returned (sic).
s32 __cdecl CRT_LCMapStringA(u32 id, u32 flags, const char *from, s32 count, char *to, s32 size, u32 page, s32 strict);

// 0x680430: _isctype: -1 to 0xff: the table _pctype; others a lead and a trail byte (one byte if the lead is no lead byte) through __crtGetStringTypeA, 0 if that fails. Anded with the mask.
s32 __cdecl CRT_isctype(s32 c, s32 mask);

// 0x67f230: _toupper_lk: no locale (0xfb1440): a-z only. Below 0x100 and not lower case (signed; negatives index before the table, sic): as it is. Else __crtLCMapStringA (LCMAP_UPPERCASE):
// the one or two bytes it gives, or c when it fails.
s32 __cdecl CRT_toupperLocked(s32 c);

// 0x67f1a0: toupper: no locale: a-z only. Else the same under the locale's lock.
s32 __cdecl CRT_toupper(s32 c);

// 0x67d540: strtoxl: white space, a sign, the base (0: by the prefix; 0x prefix skipped for 16), digits. Flags: 1 unsigned; a number too big: errno 0x22, the limit. A bad base (below 0, 1, above 36):
// 0 and *end the text. No digits: 0 and *end the text.
u32 __cdecl CRT_strtoxl(const char *text, const char **end, s32 base, u32 flags);

// 0x67d520 / 0x67d7d0: strtol / strtoul.
s32 __cdecl CRT_strtol(const char *text, const char **end, s32 base);
u32 __cdecl CRT_strtoul(const char *text, const char **end, s32 base);

// 0x67faf0: _tolower_lk: the same as _toupper_lk for the upper case.
s32 __cdecl CRT_tolowerLocked(s32 c);

// 0x67df40: _stricmp: -1, 0, 1. No locale: through A-Z. Else through _tolower_lk under the locale's count (not the Interlocked imports: its own LOCK INC) or lock 0x13; two ends at once answer the last character's _tolower_lk.
s32 __cdecl CRT_stricmp(const char *a, const char *b);

// 0x699ab0 / 0x69a340: _Tolower / _Toupper of the C++ library: the character by the given (id, code page) or, for null, the current locale. A call of __crtLCMapStringA with no eighth argument: it reads the caller's EDI, as
// `strict` here, through the shims.
s32 __cdecl CRT_Tolower(s32 c, const u32 *locale);
s32 __cdecl CRT_Toupper(s32 c, const u32 *locale);

// 0x67bb90: memmove: returns dest.
void *__cdecl CRT_memmove(void *dest, const void *from, u32 size);

// 0x67b8c0: strncat: at most `size` characters and an end; size 0: not even the end. Returns dest.
char *__cdecl CRT_strncat(char *dest, const char *from, u32 size);

// 0x67c4e0: strtok: the thread's data + 0x18 keeps the place. An empty token: null.
char *__cdecl CRT_strtok(char *text, const char *separators);

// 0x688770: xtoa: the digits (a-z above 9) with a minus for is_negative, which also negates; the end after them.
void __cdecl CRT_xtoa(u32 value, char *out, u32 radix, s32 is_negative);

// 0x67ff70: strlen. 0x67fff0: memset. 0x680060: strcmp (-1, 0 or 1, the bytes unsigned). 0x67f570: memchr. 0x67bfc0: strchr (the end too for 0). 0x67c080: strstr (the text itself for an empty one).
u32 __cdecl CRT_strlen(const char *text);
void *__cdecl CRT_memset(void *dest, s32 value, u32 size);
s32 __cdecl CRT_strcmp(const char *a, const char *b);
void *__cdecl CRT_memchr(const void *block, s32 value, u32 size);
char *__cdecl CRT_strchr(const char *text, s32 value);
char *__cdecl CRT_strstr(const char *text, const char *wanted);

// 0x67a480: atol: white space, a sign, digits, with no check for overflow. 0x688730: _itoa: a minus only in radix 10.
s32 __cdecl CRT_atol(const char *text);
char *__cdecl CRT_itoa(s32 value, char *out, s32 radix);

// 0x67b0d0: wcsncpy. 0x67bf10: wcscpy. 0x67bed0: wcscat. 0x67f450: wcsncat (always an end). 0x67c290: wcscmp (-1, 0 or 1). 0x67f410: wcsncmp (the difference of the last units). 0x67c2f0: wcschr (the end too for 0).
u16 *__cdecl CRT_wcsncpy(u16 *dest, const u16 *from, u32 count);
u16 *__cdecl CRT_wcscpy(u16 *dest, const u16 *from);
u16 *__cdecl CRT_wcscat(u16 *dest, const u16 *from);
u16 *__cdecl CRT_wcsncat(u16 *dest, const u16 *from, u32 count);
s32 __cdecl CRT_wcscmp(const u16 *a, const u16 *b);
s32 __cdecl CRT_wcsncmp(const u16 *a, const u16 *b, u32 count);
u16 *__cdecl CRT_wcschr(const u16 *text, u32 value);

// 0x67c3d0: wcsstr: unlike strstr, an empty text gives null even for an empty one wanted.
u16 *__cdecl CRT_wcsstr(const u16 *text, const u16 *wanted);

// 0x67c430: wcstok: the thread's data + 0x1c keeps the place. An empty token: null.
u16 *__cdecl CRT_wcstok(u16 *text, const u16 *separators);

}  // extern "C"
