// The wide streams' front ends of the game's C++ library (MSVC6 basic_istream<wchar_t>, basic_ostream<wchar_t>, ctype<wchar_t>; names by their shape, unconfirmed). A stream reaches its
// basic_ios by the virtual base offset at vtable +4; the ios has the state +4, the flags +0xc, the width +0x14, the locale +0x20, the buffer +0x28, the tied stream +0x2c and the fill +0x30.
// The catch (...) blocks that set badbit and rethrow are left out, as the other ports leave out their handlers; so is the library's lock.
#pragma once

#include "../../../re/types.h"

struct WStreamEnv {
    void(__stdcall *cxxThrow)(void *object, const void *info);             // _CxxThrowException 0x67d3a0
    void *(__fastcall *locinfoCtor)(void *self, void *, const char *name);  // _Locinfo's constructor 0x698fc0
    void *(__fastcall *locinfoDtor)(void *self);                            // _Locinfo's destructor 0x699100
    const u32 *(__cdecl *getCtype)(u32 *out);                               // _Getctype 0x699bd0
    u32(__fastcall *ostreamPrefix)(u8 *stream);                             // opfx 0x5d1190
};

void WStream_SetEnv(const WStreamEnv *env);

extern "C" {

// 0x4f7620 (thiscall (refs), pops 4): ctype<wchar_t>'s constructor: the use count +4, the vtable 0x709cac, the 4 dwords _Getctype answers at +8 (under a _Locinfo of "" at 0x75c74c). Returns self.
u8 *__fastcall WCtype_Ctor(u8 *self, void *, u32 refs);

// 0x4f6a30 (cdecl (locale, unused, transparent)): use_facet<ctype<wchar_t>>: the facet of the id 0xfb1df8 (given out on the first use, by the counter 0xfb1dd4). Not in the locale: a transparent call
// on the classic locale has the one kept at 0x7ced3c (made with a use, 0x7ced44 too, on the first use); else bad_cast thrown.
u8 *__cdecl WCtype_UseFacet(u8 **locale, u32 unused, u32 transparent);

// 0x4f1640 (thiscall (c), pops 4): basic_ios<wchar_t>::widen: slot 0x28 of the ctype facet of the ios's locale (a share of it held while it is looked up) with c; returns what that returns.
u32 __fastcall WIos_Widen(u8 *self, void *, u32 c);

// 0x4f1ed0 (thiscall (noskip), pops 4): basic_istream<wchar_t>::ipfx: a stream whose state is good flushes its tied stream, and without noskip and with skipws (flags bit 0) passes the units
// the ctype facet's slot 8 calls space (0x48). 1 if the state is still good, else failbit set and 0.
u8 __fastcall WIstream_Ipfx(u8 *self, void *, u32 noskip);

// 0x4f1ca0 (thiscall (text, count, delimiter), pops 12): basic_istream<wchar_t>::getline: the count +4 cleared; after ipfx(1) up to count - 1 units into text, the delimiter taken and not stored;
// an end of file sets eofbit, a full text failbit, nothing read failbit. The text always ended. Returns self.
u8 *__fastcall WIstream_Getline(u8 *self, void *, u16 *text, s32 count, u32 delimiter);

// 0x4f6be0 (cdecl (stream, text)): operator<<(basic_ostream<wchar_t> &, const wchar_t *): the text padded to the width with the fill (left of it unless flags & 0x1c0 is 0x40), the width cleared;
// a failure sets badbit; opfx failing does so without touching the width. With unitbuf (flags bit 1) the buffer's slot 0x2c flushes. Returns the stream.
u8 *__cdecl WOstream_PutText(u8 *stream, const u16 *text);

}  // extern "C"
