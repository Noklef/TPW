// A stream buffer's initialiser (MSVC6 std::basic_filebuf<wchar_t>'s _Init, by its shape; name unconfirmed) and the locale reference it holds at +0x4c: reimplementation of
// 0x4f5990 and 0x4f5a50. The buffer's +0xc, +0x1c, +0x20, +0x10, +0x2c and +0x30 point at its six get and put slots (+4, +0x14, +0x18, +8, +0x24, +0x28).
#pragma once

#include "../../../re/types.h"

// What the locale reference calls outside it, so that it can run outside the game (the benches point it at a recorder).
struct WFileBufEnv {
    u32(__cdecl *localeInit)();   // 0x699da0 (agent B's, unported)
};

extern "C" {
// The calls go to env (nullptr: the game's).
void WFileBuf_SetEnv(const WFileBufEnv *env);

// 0x4f5a50 (thiscall; name invented): *ref = 0x699da0's answer; the counted object [0xfb1dd0] gets a use (its +4 raised unless it is -1). Returns ref. The STL locks left out.
u32 *__fastcall LocaleRef_Ctor(u32 *ref);

// 0x4f5990 (thiscall (file, which), pops 8; name invented): byte +0x48 = which is 1. With which 0: the locale reference +0x4c, when set, loses a use (Counted_Release; what that answers is
// deleted through its slot 0 with 1), LocaleRef_Ctor on it, +0x44 = 0. Then the six slot pointers set and the slots zeroed, +0x50 = file, +0x40 and +0x3c = [0xfb34b4], +0x38 = 0.
void __fastcall WFileBuf_Init(u8 *buf, void *, u32 file, s32 which);
}
