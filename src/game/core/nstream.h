// The narrow streams' front ends of the game's C++ library (MSVC6 basic_istream<char>, ctype<char>): the twins of wstream's. The library's lock and the catch (...) block that sets badbit and rethrows are left out, as in wstream.
#pragma once

#include "../../../re/types.h"

struct NStreamEnv {
    void(__stdcall *cxxThrow)(void *object, const void *info);             // _CxxThrowException 0x67d3a0
    void *(__fastcall *locinfoCtor)(void *self, void *, const char *name);  // _Locinfo's constructor 0x698fc0
    void *(__fastcall *locinfoDtor)(void *self);                            // _Locinfo's destructor 0x699100
    const u32 *(__cdecl *getCtype)(u32 *out);                               // _Getctype 0x699bd0
    s32(__cdecl *atExit)(void *fn);                                         // CRT_atexit 0x67a7d0
    void(__fastcall *iosInit)(u8 *ios);                                     // IosBase_Init 0x693960
    void(__fastcall *iosAddstd)(u8 *ios);                                   // IosBase_Addstd 0x693910
};

void NStream_SetEnv(const NStreamEnv *env);

extern "C" {

// 0x5cfd50 (thiscall; names invented, unconfirmed): ctype<char>'s destructor: the table +0x10 freed when +0x14 owns it; vtables 0x709b1c then 0x709b44.
// 0x5cfc80 (cdecl, registered at exit by 0x5cfb60): the table kept at 0xfb1dd8 freed.
// 0x5cfd30 (thiscall (flags), pops 4): the destructor, then freed with bit 0 of the flags. Returns the object.
// 0x5cf9c0 (thiscall on an ios (c), pops 4): basic_ios<char>::widen: the ctype facet looked up as in 0x5d0080 (a share of the locale held), which answers c unchanged (AL).
void __fastcall NCtype_Dtor(u8 *self);
u8 *__fastcall NCtype_DeletingDtor(u8 *self, void *, u32 flags);
void __cdecl NCtype_FreeTable();
u8 __fastcall NIos_Widen(u8 *ios, void *, u32 c);

// 0x5cf900 (thiscall (buffer, addstd, init, most derived), pops 16): basic_ostream<char>'s constructor. Most derived: the vbtable 0x702980, +0x28 0 and the ios vtable at +4 0x709954.
// The ios (through the vbtable) gets vtable 0x70995c and, with init, its buffer, tie 0 and fill ' ', IosBase_Init, IosBase_Clear with badbit when the buffer is null, and with
// addstd IosBase_Addstd. 0x5cfd90 (thiscall (buffer, addstd, most derived), pops 12): basic_istream<char>'s: the vbtable 0x702988, +0x2c 0, 0x709954 at +8; gcount +4 0; vtable 0x70994c; always init.
u8 *__fastcall NOstream_Ctor(u8 *self, void *, u8 *buffer, u32 addstd, u32 init, u32 derived);
u8 *__fastcall NIstream_Ctor(u8 *self, void *, u8 *buffer, u32 addstd, u32 derived);

// 0x5d1190 (thiscall): basic_ostream<char>::opfx: with a good state the tied stream flushed; AL 1 when the state is good then. 0x5d1120 (thiscall on a sentry, the stream at +4): unless
// an exception is uncaught (Fn_6a65e0), the stream flushed when its flags have unitbuf (2). A flush: slot 0x2c of the buffer unless the state has fail or bad; -1 sets badbit by IosBase_Clear.
u8 __fastcall NOstream_Opfx(u8 *self);
void __fastcall NOstream_Sentry(u8 *sentry);

// 0x5cfb60 (thiscall (table, del, refs), pops 12; name invented, unconfirmed): ctype<char>'s constructor: the use count +4, the vtable 0x709b1c, the 4 dwords _Getctype answers at +8. With none kept at
// 0xfb1dd8 yet: +0x10 kept there, 0x5cfc80 registered at exit and the delete flag +0x14 cleared. An owned table left (+0x14 not 0) is freed. Then the table given, the low byte of del in +0x14; with no
// table given the one kept (+0x14 left as it is). Returns the object.
u8 *__fastcall NCtype_Ctor(u8 *self, void *, u8 *table, u32 del, u32 refs);

// 0x5d0080 (thiscall (noskip), pops 4; name invented, unconfirmed): basic_istream<char>::ipfx: a stream whose state is good flushes its tied stream, and without noskip and with the flag skipws (+0xc
// bit 0) skips the units the ctype facet (the id 0xfb1da4; on the classic locale the one kept at 0xf827e0, made with a use on the first use; else bad_cast thrown) marks 0x48 in its table +0x10. Answers
// 1 with the state good, else 0 with failbit set.
u8 __fastcall NIstream_Ipfx(u8 *self, void *, u32 noskip);
// 0x5cfe40 (thiscall (text, count, delimiter), pops 12; basic_istream<char>::getline, by its shape; name invented, unconfirmed): the twin of WIstream_Getline: gcount +4 from 0, NIstream_Ipfx (1); with a positive count, units up to the
// delimiter's low byte (counted, not stored) or count less one; an end of file sets eofbit, a full text failbit, nothing read failbit. The text always ended. Returns self.
u8 *__fastcall NIstream_Getline(u8 *self, void *, char *text, u32 count, u32 delimiter);
// 0x5d3530 (thiscall (text, count), pops 8; basic_istream<char>::read, by its shape; name invented, unconfirmed): gcount +4 from 0, NIstream_Ipfx (1); then the buffer's slot 0x18 (text, count) adds to gcount, and less than count sets eofbit and failbit. Returns self.
u8 *__fastcall NIstream_Read(u8 *self, void *, char *text, u32 count);
// 0x5d0e70 (cdecl (stream, text); operator<<(ostream &, const char *), by its shape; name invented, unconfirmed): the twin of WOstream_PutText: pads with the ios's fill up to its width +0x14 (before the text unless adjusted left, 0x40 of
// the flags' 0x1c0; after it otherwise), the buffer's slot 0x1c writes the text, the width cleared; any failure badbit; a failed NOstream_Opfx badbit alone. Then with no exception uncaught (Fn_6a65e0) and unitbuf (2) the stream is flushed. Returns the stream.
u8 *__cdecl NOstream_PutText(u8 *stream, const char *text);
}
