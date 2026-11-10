// What the C library's start-up does with the command line (MSVC6's own names).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6898f0: parse_cmdline: the words of `command` into `words` (a table ending in null, may be null) and `text` (may be null), counted in *wordCount (with the table's end) and *textSize. The first word ends at
// a closing quote or a blank; in the others 2n backslashes before a quote are n and the quote toggles (a doubled one inside quotes is itself), 2n+1 are n and a quote. The table 0xfb1669 bit 4 marks lead bytes.
void __cdecl CRT_parse_cmdline(const char *command, char **words, char *text, s32 *wordCount, s32 *textSize);

// 0x67f8f0: _cinit: the floating point set-up the pointer 0x77e718 holds (if any), then the C initialisers (0x72e0ec-0x72e0fc) and the C++ ones (0x727000-0x72e0e8).
void __cdecl CRT_cinit();
// 0x67f960: doexit: under the exit lock; a second exit (0xfb13a0 1) terminates the process first. Unless quick: the onexit table (0xfb3cb4-0xfb3cb0) backwards, its
// start read live, then the pre-terminators 0x72e100-0x72e10c; always the terminators 0x72e110-0x72e118. retcaller: unlock and return; else ExitProcess(code).
void __cdecl CRT_doexit(s32 code, s32 quick, s32 retcaller);
void __cdecl CRT_exit(s32 code);     // 0x67f920: doexit(code, 0, 0)
void __cdecl CRT__exit(s32 code);    // 0x67f940: doexit(code, 1, 0)
void __cdecl CRT_lockexit();         // 0x67fa20: lock 0xd
void __cdecl CRT_unlockexit();       // 0x67fa30

// 0x67ee10: WinMainCRTStartup, the exe's entry point: GetVersion's parts to 0xfb1364-0xfb1370, _heap_init and _mtinit (else fast_error_exit), then under __try
// the CRT's start-up, the command line past the program's name, exit(WinMain(...)); __except (_XcptFilter) _exit(code). Its callees go through the table below.
// (Not the original's SEH frame: the compiler's own, not _except_handler3 0x67dbf4 with the scope table 0x708a50.)
void __cdecl CRT_WinMainCRTStartup();

}  // extern "C"

struct AddrCall;
void CRT_SetStartupCalls(const AddrCall *table, u32 count);   // tests/fncmp/crt_entry.py

extern "C" {

}  // extern "C"
