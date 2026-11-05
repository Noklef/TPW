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
    void(__cdecl *makeloc)(void *info, u32 categories, void *imp, void *loc);   // 0x694120: locale::_Locimp::_Makeloc
    void *(__fastcall *ctypeCtor)(void *self, void *edx, u32 table, u32 owned, u32 refs);   // 0x5cfb60: ctype<char>'s constructor
    void(__cdecl *makexloc)(void *info, u32 categories, void *imp, void *loc);   // 0x6a67f0: _Locimp::_Makexloc
    void(__cdecl *makewloc)(void *info, u32 categories, void *imp, void *loc);   // 0x69a7a0: _Locimp::_Makewloc
    void *(__fastcall *numpunctCtor)(void *self, void *edx, u32 refs);    // 0x695910: numpunct<char>'s constructor
    void *(__fastcall *wnumpunctCtor)(void *self, void *edx, u32 refs);   // 0x69c680: numpunct<wchar_t>'s constructor
};

void IosBase_SetEnv(const IosBaseEnv *env);
const IosBaseEnv &IosBase_Env();

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

// 0x4ef840 (thiscall (flags), pops 4): ios_base's deleting destructor: the destructor, the block freed for flags bit 0. Returns self.
u8 *__fastcall IosBase_DeletingDtor(u8 *self, void *edx, u32 flags);

// 0x4f0100 (thiscall): basic_ios<wchar_t>'s destructor: its vtable 0x7099e4, then ios_base's.
void __fastcall BasicIosW_Dtor(u8 *self);
// 0x4f1210 (thiscall (flags), pops 4): BasicIosW_Dtor, the object freed for flags bit 0. Returns it.
u8 *__fastcall BasicIosW_DeletingDtor(u8 *self, void *, u32 flags);
// 0x4f12b0, 0x4f1320 (thiscall on the stream): basic_ostream<wchar_t>'s and basic_istream<wchar_t>'s complete destructors: the stream's vtable word through its vbtable (entry +4) = 0x7099ec, 0x7099dc; BasicIosW_Dtor
// of the virtual base (stream +4, +8).
void __fastcall BasicOstreamW_VbaseDtor(u8 *stream);
void __fastcall BasicIstreamW_VbaseDtor(u8 *stream);
// 0x4f11e0, 0x4f1230 (thiscall on the virtual base (flags), pops 4): the same with the stream freed for flags bit 0. Returns the stream.
u8 *__fastcall BasicOstreamW_DeletingDtor(u8 *ios, void *, u32 flags);
u8 *__fastcall BasicIstreamW_DeletingDtor(u8 *ios, void *, u32 flags);

// 0x4f0110 (thiscall (state, reraise), pops 8): setstate: nothing for state 0; else clear(the old state +4 | state, | 4 without a stream buffer +0x28).
void __fastcall BasicIosW_SetState(u8 *self, void *edx, u32 state, u32 reraise);

// 0x4f1620 (thiscall (state, reraise), pops 8): clear: IosBase_Clear with state, | 4 without a stream buffer +0x28.
void __fastcall BasicIosW_Clear(u8 *self, void *edx, u32 state, u32 reraise);

// 0x4f1560 (thiscall (buffer, isstd, init, most derived), pops 0x10): basic_ostream<wchar_t>: most derived: the vbtable 0x700648 and its ios at +4 (+0x28 0, the vtable). The
// ios (by the vbtable's +4): the vtable 0x7099ec. With init: its init (+0x28 buffer, +0x2c 0, +0x30 the widened ' ', IosBase_Init, without a buffer the bad bit; isstd: IosBase_Addstd). Returns self.
u8 *__fastcall BasicOstreamW_Ctor(u8 *self, void *edx, void *buffer, u32 isstd, u32 init, u32 mostDerived);

// 0x4f1be0 (thiscall (buffer, isstd, most derived), pops 0xc): basic_istream<wchar_t>: as above, the vbtable 0x700650, its ios at +8, the vtable 0x7099dc, +4 (the count) 0, init always.
u8 *__fastcall BasicIstreamW_Ctor(u8 *self, void *edx, void *buffer, u32 isstd, u32 mostDerived);

// 0x4f2180 (thiscall (text), pops 4): bad_cast(text): exception's constructor (of the argument's slot), its vtable 0x709b34. Returns self.
u32 *__fastcall BadCast_Ctor(u32 *self, void *edx, const char *text);

// 0x4f1870 (thiscall (from), pops 4): bad_cast's copy constructor: exception's, the vtable. Returns self.
u32 *__fastcall BadCast_Copy(u32 *self, void *edx, const u32 *from);

// 0x4f1820 (thiscall): bad_cast's destructor: its vtable, then exception's destructor.
void __fastcall BadCast_Dtor(u32 *self);
// 0x4f1850 (thiscall (flags), pops 4): BadCast_Dtor, the object freed for flags bit 0. Returns it.
u32 *__fastcall BadCast_DeletingDtor(u32 *self, void *, u32 flags);
// 0x4f1830 (thiscall; name unconfirmed): a copy of the bad_cast at this thrown with info 0x7193a8.
void __fastcall BadCast_Throw(const u32 *from);

// 0x4f1ad0 (thiscall (c), pops 4): ctype<wchar_t>::do_widen(c): btowc of the sign-extended byte.
u32 __fastcall CtypeW_Widen(void *self, void *edx, u32 c);

// 0x4f1af0 (thiscall (first, last, to), pops 0xc): do_widen of the bytes in turn to the words at to. Returns last.
const char *__fastcall CtypeW_WidenRange(void *self, void *edx, const char *first, const char *last, u16 *to);

// 0x4f19d0, 0x4f1a50 (thiscall (c), pop 4; ctype<wchar_t>::do_tolower, do_toupper by their shape; names unconfirmed): the slot 0x30 (c, 0)'s byte, sign-extended, through _Tolower or _Toupper with the
// locale +8; the answer's byte replaces c's low byte for the slot 0x28, whose answer is returned.
u32 __fastcall CtypeW_ToLower(u8 *self, void *, u32 c);
u32 __fastcall CtypeW_ToUpper(u8 *self, void *, u32 c);
// 0x4f1a10, 0x4f1a90 (thiscall (first, last), pop 8; names unconfirmed): the same maps over the words, in place, with no slots. Return last.
u16 *__fastcall CtypeW_ToLowerRange(u8 *self, void *, u16 *first, u16 *last);
u16 *__fastcall CtypeW_ToUpperRange(u8 *self, void *, u16 *first, u16 *last);
// 0x4f1b30 (thiscall (c, default), pops 8; do_narrow; name unconfirmed): _Wctob of c; the default is not used.
s32 __fastcall CtypeW_Narrow(void *, void *, u32 c, u32);
// 0x4f1b40 (thiscall (first, last, default, to), pops 0x10; name unconfirmed): each word's _Wctob byte to the bytes at to. Returns last.
const u16 *__fastcall CtypeW_NarrowRange(void *, void *, const u16 *first, const u16 *last, u32, char *to);

// 0x4f1ba0 (thiscall): ctype<wchar_t>'s destructor: its vtable 0x709cac, the table +0x10 freed (CRT free) if owned (+0x14), locale::facet's vtable 0x709b44.
void __fastcall CtypeW_Dtor(u8 *self);
// 0x4f1b80 (thiscall (flags), pops 4): CtypeW_Dtor, the object freed for flags bit 0. Returns it.
u8 *__fastcall CtypeW_DeletingDtor(u8 *self, void *, u32 flags);

// 0x699da0: std::locale::_Init: under the lock, on the first call, the classic locale's _Locimp ([0xfb1dd0]) made, its destructor 0x699ee0 given to atexit, its categories +0x10
// 0x3f, its name +0x18 "C"; held for [0xfb1dcc] (a use more, under the lock) and [0xfb1dc8]. Returns it.
void *__cdecl Std_LocaleInit();

}  // extern "C"
