// A stream buffer's initialiser (MSVC6 std::basic_filebuf<wchar_t>'s _Init, by its shape; name unconfirmed) and the locale reference it holds at +0x4c: reimplementation of
// 0x4f5990 and 0x4f5a50. The buffer's +0xc, +0x1c, +0x20, +0x10, +0x2c and +0x30 point at its six get and put slots (+4, +0x14, +0x18, +8, +0x24, +0x28).
#pragma once

#include "../../../re/types.h"

// What the locale reference calls outside it, so that it can run outside the game (the benches point it at a recorder).
struct WFileBufEnv {
    u32(__cdecl *localeInit)();   // 0x699da0
    s32(__cdecl *fclose)(void *file);   // CRT_fclose
    u32(__cdecl *fwrite)(const void *data, u32 size, u32 count, u32 file);   // 0x67b6d0
    u32(__cdecl *fputwc)(u32 c, u32 file);   // 0x67c9f0
    u32(__cdecl *ungetwc)(u32 c, u32 file);   // 0x67cca0
    s32(__cdecl *ungetc)(u32 c, u32 file);   // 0x67cbe0
    s32(__cdecl *fgetc)(u32 file);   // 0x67cb90
    u32(__cdecl *fgetwc)(u32 file);   // 0x67cdf0
    s32(__cdecl *mbrtowc)(u16 *unit, const char *text, u32 n, u8 *state, const u32 *conversion);   // CRT_Mbrtowc
    s32(__cdecl *wcrtomb)(char *to, u32 unit, void *state, const u32 *conversion);   // CRT_Wcrtomb
    void(__stdcall *cxxThrow)(void *object, const void *info);   // _CxxThrowException 0x67d3a0
    void *(__fastcall *locinfoCtor)(void *self, void *, const char *name);   // _Locinfo's constructor 0x698fc0
    void *(__fastcall *locinfoDtor)(void *self);   // _Locinfo's destructor 0x699100
    u8 **(__fastcall *addfac)(u8 **self, void *, u8 *facet, u32 id, u32 category);   // locale::_Addfac 0x693f50
};

extern "C" {
// The calls go to env (nullptr: the game's).
void WFileBuf_SetEnv(const WFileBufEnv *env);

// 0x4f5a50 (thiscall; name invented): *ref = 0x699da0's answer; the counted object [0xfb1dd0] gets a use (its +4 raised unless it is -1). Returns ref. The STL locks left out.
u32 *__fastcall LocaleRef_Ctor(u32 *ref);

// 0x4f5990 (thiscall (file, which), pops 8; name invented): byte +0x48 = which is 1. With which 0: the locale reference +0x4c, when set, loses a use (Counted_Release; what that answers is
// deleted through its slot 0 with 1), LocaleRef_Ctor on it, +0x44 = 0. Then the six slot pointers set and the slots zeroed, +0x50 = file, +0x40 and +0x3c = [0xfb34b4], +0x38 = 0.
void __fastcall WFileBuf_Init(u8 *buf, void *, u32 file, s32 which);

// 0x4f3720 (thiscall (file), pops 4; an SEH frame, dropped; basic_filebuf<wchar_t>'s constructor, by its shape; name unconfirmed): the streambuf part: its locale reference +0x34 (0x699da0's answer, the global's counted object [0xfb1dd0]
// gets a use), the six slot pointers, three slots zeroed, vtable 0x7099a4, Wstreambuf_Setg (0, 0, 0). Then LocaleRef_Ctor on +0x4c, vtable 0x70996c, byte +0x48 = 0; WFileBuf_Init (file, 0), inlined: the reference +0x4c just made loses a use (deleted at the last),
// made again, the pointers, +0x44 = 0, Wstreambuf_Setp (0, 0), Wstreambuf_Setg (0, 0, 0), +0x50 = file, +0x40 and +0x3c = [0xfb34b4], +0x38 = 0. Returns the buffer.
u8 *__fastcall WFileBuf_Ctor(u8 *buf, void *, u32 file);

// 0x4f08c0 (thiscall; an SEH frame, dropped; basic_filebuf<wchar_t>'s destructor, by its shape; name unconfirmed): vtable 0x70996c. Byte +0x48 and a file +0x50 whose fclose answers 0: WFileBuf_Init (0, 2)
// (the original has it inline). The string +0x44, when set, tidied and freed. The locale references +0x4c, then (after vtable 0x7099a4) +0x34, each when set lose a use, deleted (slot 0 with 1) at the last.
void __fastcall WFileBuf_Dtor(u8 *buf);

// 0x4f0540 (thiscall on the virtual base at stream +0x58; an SEH frame, dropped; basic_ofstream<wchar_t>'s, by its shape; name unconfirmed): the stream's vtable word through its vbtable (entry +4) = 0x70062c;
// the buffer at stream +4 as WFileBuf_Dtor; the same word = 0x7099ec.
void __fastcall WOfstream_VbaseDtor(u8 *ios);

// 0x4f10d0 (the same, the virtual base at stream +0x5c; basic_ifstream<wchar_t>'s): 0x700638, the buffer at stream +8, 0x7099dc.
void __fastcall WIfstream_VbaseDtor(u8 *ios);

// 0x4ef390, 0x4ef860 (thiscall on the stream; names unconfirmed): the complete destructors: the Vbase ones above, then basic_ios's on the virtual base at +0x58, +0x5c.
void __fastcall WOfstream_Dtor(u8 *stream);
void __fastcall WIfstream_Dtor(u8 *stream);
// 0x4f1280, 0x4f12f0 (thiscall on the virtual base (flags), pop 4; names unconfirmed): the above, the stream freed for flags bit 0. Return the stream.
u8 *__fastcall WOfstream_DeletingDtor(u8 *ios, void *, u32 flags);
u8 *__fastcall WIfstream_DeletingDtor(u8 *ios, void *, u32 flags);
// 0x4f12d0 (thiscall (flags), pops 4; name unconfirmed): WFileBuf_Dtor, the buffer freed for flags bit 0. Returns it.
u8 *__fastcall WFileBuf_DeletingDtor(u8 *buf, void *, u32 flags);

// basic_filebuf<wchar_t>'s virtuals, by their shape (slots 1, 2 and 5 of the vtable 0x70996c; names unconfirmed), thiscall. A u16 answer, 0xffff for eof.
// 0x4f0a10 overflow (c), pops 4: eof answers 0; room in the put area (+0x20's pointer, +0x30's count): stored. No file +0x50: eof. No conversion +0x38: fputwc. Otherwise the string +0x44 emptied and made 8
// bytes, the conversion's slot 5 (state +0x40, c as from, the string as to) answers: 0, 1 with some taken: fwrite (the bytes it left, size them, count 1), whose answer must equal the size (sic); 3: fputwc; others eof.
u16 __fastcall WFileBuf_Overflow(u8 *buf, void *, u32 c);
// 0x4f0c50 pbackfail (c), pops 4: a get area with something behind (+0x1c's pointer past +0xc's), and eof or the same char: the pointer back, the count +0x2c up; the answer c, or 0 for eof. Otherwise no file or
// eof: eof. No conversion: ungetwc. Otherwise the string's bytes, last first, through ungetc: any failure or an empty string is eof; then the string emptied, +0x40 = +0x3c, the answer c.
u16 __fastcall WFileBuf_PutBack(u8 *buf, void *, u32 c);
// 0x4f0e60 uflow: a char in the get area taken. No file: eof. No conversion: fgetwc. Otherwise +0x3c = +0x40, the string emptied, bytes from fgetc appended one at a time, each followed by the conversion's slot 4
// (the string as from, a local as to) from the state +0x40 = +0x3c: 0 answers the local; 1 reads on; 3 with two bytes or more answers the string's first word, with fewer reads on; others, or no byte, eof.
u16 __fastcall WFileBuf_Uflow(u8 *buf);

// 0x4f5d60 (thiscall (refs), pops 4; codecvt<wchar_t, char, mbstate_t>'s constructor, by its shape; name unconfirmed): the use count +4, the vtable 0x709ce4, _Getcvt's block at +8 (under a _Locinfo of "" at 0x75c74c). Returns self.
u8 *__fastcall CodecvtW_Ctor(u8 *self, void *, u32 refs);

// 0x4f7180 (cdecl (&out, locale by value, facet); name invented, unconfirmed): the id 0xfb1e00 given out on the first use; locale::_Addfac(facet, id, 2) on the argument; out = what that answers, with a use; the argument's
// locale, when set, loses a use (deleted through slot 0 at the last). Returns &out.
u8 **__cdecl Locale_WithCodecvt(u8 **out, u8 *locale, u8 *facet);

// 0x4f5ad0 (thiscall; basic_filebuf<wchar_t>'s _Initcvt, by its shape; name unconfirmed): +0x38 = the conversion facet of the stream's locale +0x34 (see use_codecvt); the reference +0x4c gets Locale_WithCodecvt of itself and it
// when they differ; the slot 4 (always_noconv) of the facet saying so clears +0x38; a string +0x44 made, empty, when there is none.
void __fastcall WFileBuf_InitConversion(u8 *buf);

// 0x4f3920 (thiscall (name, mode), pops 8; basic_filebuf<wchar_t>::open, by its shape; name unconfirmed): 0 with a file +0x50 or when __Fiopen fails. Otherwise WFileBuf_Init (file, 1) and WFileBuf_InitConversion (both inline in part in the original); returns self.
u8 *__fastcall WFileBuf_Open(u8 *buf, void *, const char *name, u32 mode);

// 0x4f5dd0 (thiscall (state, from, from_end, &from_next, to, to_end, &to_next), pops 28; codecvt::do_in, by its shape; name unconfirmed): one _Mbrtowc per unit, by the block +8. 0 when all is done or something was and the output is
// full, 1 (partial) when nothing was, 2 for an error. -2 takes all the input and answers as far as it got; an answer of 0 (a zero byte) takes the string and its zero.
u32 __fastcall CodecvtW_In(u8 *self, void *, u8 *state, const u8 *from, const u8 *from_end, const u8 **from_next, u16 *to, u16 *to_end, u16 **to_next);

// 0x4f5e80 (the same arguments, wide in, bytes out; codecvt::do_out, by its shape; name unconfirmed): one _Wcrtomb per unit, into the output with room for _mb_cur_max bytes, else into a local whose bytes go on if they fit (if not,
// the state put back and partial answered). The answers as above; an answer of 0 or less is an error.
u32 __fastcall CodecvtW_Out(u8 *self, void *, u32 *state, const u16 *from, const u16 *from_end, const u16 **from_next, u8 *to, u8 *to_end, u8 **to_next);

// 0x4f5fb0 (thiscall (state, first, last, most), pops 16; codecvt::do_length, by its shape; name unconfirmed): how many units from first, up to last, fit in most bytes: _Wcrtomb on each until
// it answers 0 or less or more than is left. Most is only tested for 0 once, before the loop.
s32 __fastcall CodecvtW_Length(u8 *self, void *, void *state, const u16 *first, const u16 *last, u32 most);
}
