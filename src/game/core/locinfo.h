// The C++ library's lock and locale information: MSVC6's std::_Lockit and std::_Locinfo.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x691de0: _Lockit's constructor: the critical section [0xfb1950], made once ([0xfb1968]: 0 none, 1 being made, 2 made; atexit 0x691e70), entered.
void *__fastcall Lockit_Ctor(void *self);

// 0x691ea0: _Lockit's destructor: the critical section left when made.
void __fastcall Lockit_Dtor(void *self);

// 0x691e70: the exit routine the constructor registers: [0xfb1968] set to 3; the critical section deleted when it was 2.
void __cdecl Lockit_Term();

// 0x698fc0: _Locinfo's constructor (pops 4): four strings (+0, +0x10, +0x20, +0x30). +0x20 the C library's locale, which setlocale changes to name; +0x30 the new one, or "*" when name is null or no such locale. The strings' allocators take the low byte of name (sic).
void *__fastcall Locinfo_Ctor(void *self, void *, const char *name);

// 0x699100: _Locinfo's destructor: back to the locale at +0x20, unless that is empty; the strings released.
void __fastcall Locinfo_Dtor(void *self);
}
