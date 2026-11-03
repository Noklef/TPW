// The MSVC6 C++ library's std::ios_base, its standard exceptions' throws and the classic locale's start.
// All in the game's CRT range; names are the library's own.
#pragma once

#include "../../../re/types.h"

struct IosBaseEnv {
    void *(__fastcall *lockit)(void *lock);                        // 0x691de0: std::_Lockit's constructor (thiscall)
    void(__fastcall *unlockit)(void *lock);                        // 0x691ea0
    void(__cdecl *memFree)(void *block);                           // Mem_Free 0x67acb0
    void(__stdcall *cxxThrow)(void *object, const void *info);     // _CxxThrowException 0x67d3a0
    void *(__cdecl *fopen)(const char *name, const char *mode);    // 0x67b8a0
    s32(__cdecl *fseek)(void *file, s32 offset, s32 origin);       // 0x67c100
    s32(__cdecl *fclose)(void *file);                              // 0x67b620
    void *(__cdecl *memAlloc)(u32 size);                           // Mem_Alloc 0x67acc0
    void *(__fastcall *locimpCtor)(void *self, void *edx, u32 refs);   // 0x699f70: locale::_Locimp's constructor
    s32(__cdecl *atexit)(void(__cdecl *fn)());                     // 0x67a7d0
};

void IosBase_SetEnv(const IosBaseEnv *env);

extern "C" {

// 0x6918f0 / 0x691ba0: _Xlen, _Xran: length_error("string too long") (vtable 0x709884, throw info 0x720468), out_of_range("invalid string position") (vtable 0x7098b4,
// info 0x71f298) thrown. The thrower's string stays allocated if a handler unwinds through here (the original's frame frees it).
void __cdecl Std_Xlen();
void __cdecl Std_Xran();

// 0x693340 (thiscall (state, reraise), pops 8): ios_base::clear: state & 7 kept at +4; bits the exception mask +8 also has: a rethrow when reraise's low byte is set, else
// ios_base::failure (vtable 0x7099fc, info 0x720720) naming the worst (bad, fail, eof).
void __fastcall IosBase_Clear(u8 *self, void *edx, u32 state, u32 reraise);

// 0x6938e0 (thiscall (event), pops 4): _Callfns: each registered callback of the list +0x1c: fn +8 (event, self, its index +4).
void __fastcall IosBase_Callfns(u8 *self, void *edx, u32 event);

// 0x693910 (thiscall): _Addstd: the first of the eight standard stream slots (0xfb1d60) from index +0x24 that is free or this stream, the index stored there; past them the
// ninth slot (sic); this stored and its open count (0xfb1d88) raised, under the lock.
void __fastcall IosBase_Addstd(u8 *self);

// 0x693960 (thiscall): _Init: the locale reference +0x20 (LocaleRef_Ctor), the mask +8 cleared, flags +0xc 0x201, precision +0x10 6, +0x14, +0x18 and +0x1c cleared, then clear(0, 0).
void __fastcall IosBase_Init(u8 *self);

// 0x6939a0 (thiscall): _Tidy: _Callfns(0), then the two lists +0x18 and +0x1c freed and cleared.
void __fastcall IosBase_Tidy(u8 *self);

// 0x693830 (thiscall): ~ios_base: the vtable 0x709964; a standard stream's open count (index +0x24, unsigned) lowered: _Tidy unless more remain (signed); then the locale
// reference +0x20 given up (its count +4, unless 0 or -1, lowered under the lock), its deleting destructor (slot 0, 1) at a count of 0.
void __fastcall IosBase_Dtor(u8 *self);

// 0x6939f0: __Fiopen: the mode less ate (4) looked up in the table 0x709a50 (zero ended; missing: 0); fopen with the matching text of 0x782590; with ate, a seek to the end,
// closing the file and answering 0 if it fails.
void *__cdecl Std_FiopenTable(const char *name, u32 mode);

// 0x693a70: the same.
void *__cdecl Std_Fiopen(const char *name, u32 mode);

// 0x699da0: std::locale::_Init: under the lock, on the first call, the classic locale's _Locimp ([0xfb1dd0]) made, its destructor 0x699ee0 given to atexit, its categories +0x10
// 0x3f, its name +0x18 "C"; held for [0xfb1dcc] (a use more, under the lock) and [0xfb1dc8]. Returns it.
void *__cdecl Std_LocaleInit();

}  // extern "C"
