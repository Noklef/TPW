// The C library's raise and abort (MSVC6's winsig.c and abort.c).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x68f780: raise: SIGINT, SIGBREAK, SIGABRT and SIGTERM through their handlers 0xfb1918, 0xfb191c, 0xfb1920 and 0xfb1924 under the signal lock (1);
// SIGILL, SIGFPE and SIGSEGV through the thread's exception-action table (siglookup), its _pxcptinfoptrs (+0x54) cleared for the call and SIGFPE's
// code (+0x58) _FPE_EXPLICITGEN (0x8c) as the handler's second argument. SIG_IGN: nothing; SIG_DFL: _exit(3). The handler is reset to SIG_DFL first
// (for SIGFPE every floating point entry [0x781508] to [0x781508] + [0x78150c]). -1 for other signals, else 0.
s32 __cdecl CRT_raise(s32 sig);
// 0x68cc30: abort: message 10, raise(SIGABRT), _exit(3).
void __cdecl CRT_abort();
}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(s32, CRT_First_FPE_Indx, 0x00781508, 0x4);
GAME_DATA(s32, CRT_Num_FPE, 0x0078150c, 0x4);
GAME_DATA(u32, CRT_ctrlc_action, 0x00fb1918, 0x4);
GAME_DATA(u32, CRT_ctrlbreak_action, 0x00fb191c, 0x4);
GAME_DATA(u32, CRT_abort_action, 0x00fb1920, 0x4);
GAME_DATA(u32, CRT_term_action, 0x00fb1924, 0x4);
