// The C++ library's std::locale::_Locimp and the allocation failure's throw: MSVC6's _Nomemory.
#pragma once

#include "../../../re/types.h"
#include "stdstring.h"

struct LocaleFacet {
    void(__fastcall *const *vtable)(LocaleFacet *self, void *edx, u32 flags);   // (slot 0: the deleting destructor)
    u32 refs;
};

struct Locimp {
    LocaleFacet facet;
    LocaleFacet **facets;   // +0x8
    u32 count;              // +0xc
    u32 categories;         // +0x10
    u8 xparse;              // +0x14
    StdString name;         // +0x18
};

struct LocaleHandle {
    Locimp *imp;
};

extern "C" {

// 0x6a6650: bad_alloc's constructor (pops 4). 0x6a66c0: its copy (pops 4). 0x6a6670: its destructor.
u32 *__fastcall BadAlloc_Ctor(u32 *self, void *, const char *text);
u32 *__fastcall BadAlloc_Copy(u32 *self, void *, const u32 *from);
void __fastcall BadAlloc_Dtor(u32 *self);

// 0x6a65f0: _Nomemory: a copy of the one bad_alloc ([0xfb1e70], made once: flag bit 0 of [0xfb1e68]; atexit 0x6a66e0) thrown with info 0x721c88.
void __cdecl Nomemory();

// 0x698ea0: _Locimp::_Addfac (pops 8): the array grown to hold id (at least 0x20, the new slots null), the facet counted in, the one it replaces counted out and deleted at 0.
void __fastcall Locimp_Addfac(Locimp *self, void *, LocaleFacet *facet, u32 id);

// 0x699f70 (pops 4): _Locimp's constructor: count 1, no facets, the byte at +0x14 and the name's allocator from the argument, the name the text at 0x747828 (assign inlined); the vtable 0x709b44, then 0x709bac.
Locimp *__fastcall Locimp_Ctor(Locimp *self, void *, u32 xparse);
// 0x69a080: _Locimp's destructor body: under the lock each facet from the last, a count dropped, and deleted when none are left; the array freed; the name released; the base vtable 0x709b44.
void __fastcall Locimp_Dtor(Locimp *self, void *);
// 0x699ee0 (atexit; name invented, unconfirmed): under the lock the facet at [0xfb1dd0], a count dropped, and deleted when none are left. The global is left as it is.
void __cdecl Locimp_TidyAtExit();

// 0x698c70: _Locimp's copy (pops 4): count 1; the classic one ([0xfb1dcc]) gives all categories' facets by _Makeloc, any other its facets counted in.
Locimp *__fastcall Locimp_Copy(Locimp *self, void *, const Locimp *from);

// 0x693f50: locale::_Addfac (pops 0xc): a shared _Locimp left for a copy, then the facet added. Any category in mask: the name becomes "*". Returns self.
LocaleHandle *__fastcall Locale_Addfac(LocaleHandle *self, void *, LocaleFacet *facet, u32 id, u32 mask);
}
