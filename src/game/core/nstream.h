// The narrow streams' front ends of the game's C++ library (MSVC6 basic_istream<char>, ctype<char>): the twins of wstream's. The library's lock and the catch (...) block that sets badbit and rethrows are left out, as in wstream.
#pragma once

#include "../../../re/types.h"

struct NStreamEnv {
    void(__stdcall *cxxThrow)(void *object, const void *info);             // _CxxThrowException 0x67d3a0
    void *(__fastcall *locinfoCtor)(void *self, void *, const char *name);  // _Locinfo's constructor 0x698fc0
    void *(__fastcall *locinfoDtor)(void *self);                            // _Locinfo's destructor 0x699100
    const u32 *(__cdecl *getCtype)(u32 *out);                               // _Getctype 0x699bd0
    s32(__cdecl *atExit)(void *fn);                                         // CRT_atexit 0x67a7d0
};

void NStream_SetEnv(const NStreamEnv *env);

extern "C" {

// 0x5cfb60 (thiscall (table, del, refs), pops 12; name invented, unconfirmed): ctype<char>'s constructor: the use count +4, the vtable 0x709b1c, the 4 dwords _Getctype answers at +8. With none kept at
// 0xfb1dd8 yet: +0x10 kept there, 0x5cfc80 registered at exit and the delete flag +0x14 cleared. An owned table left (+0x14 not 0) is freed. Then the table given, the low byte of del in +0x14; with no
// table given the one kept (+0x14 left as it is). Returns the object.
u8 *__fastcall NCtype_Ctor(u8 *self, void *, u8 *table, u32 del, u32 refs);

// 0x5d0080 (thiscall (noskip), pops 4; name invented, unconfirmed): basic_istream<char>::ipfx: a stream whose state is good flushes its tied stream, and without noskip and with the flag skipws (+0xc
// bit 0) skips the units the ctype facet (the id 0xfb1da4; on the classic locale the one kept at 0xf827e0, made with a use on the first use; else bad_cast thrown) marks 0x48 in its table +0x10. Answers
// 1 with the state good, else 0 with failbit set.
u8 __fastcall NIstream_Ipfx(u8 *self, void *, u32 noskip);
}
