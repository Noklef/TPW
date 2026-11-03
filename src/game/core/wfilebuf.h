// A stream buffer's initialiser (MSVC6 std::basic_filebuf<wchar_t>'s _Init, by its shape; name unconfirmed) and the locale reference it holds at +0x4c: reimplementation of
// 0x4f5990 and 0x4f5a50. The buffer's +0xc, +0x1c, +0x20, +0x10, +0x2c and +0x30 point at its six get and put slots (+4, +0x14, +0x18, +8, +0x24, +0x28).
#pragma once

#include "../../../re/types.h"

// What the locale reference calls outside it, so that it can run outside the game (the benches point it at a recorder).
struct WFileBufEnv {
    u32(__cdecl *localeInit)();   // 0x699da0
    s32(__cdecl *fclose)(void *file);   // CRT_fclose
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
}
