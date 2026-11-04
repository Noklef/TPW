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
}
