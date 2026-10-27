// The counted shared objects kept in globals (each set by a setter that gives it a use and its release to atexit): their releases and setters.
// Reimplementation of 0x4f7a10, 0x699650, 0x699770, 0x699890, 0x6a5ae0, 0x6a5c00, 0x6a5d20, 0x6a5e40, 0x6a5f60, 0x6a6080, 0x6a61a0,
// 0x5d1670, 0x6a62c0, 0x6a6360, 0x6a6400, 0x6a64a0, 0x6ac0b0, 0x6ac1d0, 0x6ac2f0, 0x6ac410, 0x6ac530, 0x6ac650, 0x6ac6f0, 0x6ac790, and of the
// setters 0x4f7770, 0x4f77f0, 0x5d1460, 0x5d14e0, 0x6995d0, 0x6996f0, 0x699810, 0x6a5a60, 0x6a5b80, 0x6a5ca0, 0x6a5dc0, 0x6a5ee0, 0x6a6000, 0x6a6120,
// 0x6a6240, 0x6ac030, 0x6ac150, 0x6ac270, 0x6ac390, 0x6ac4b0, 0x6ac5d0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

struct SharedGlobalsEnv {
    s32(__cdecl *atExit)(void(__cdecl *fn)());   // crt_atexit (0x67a7d0)
};
// The setters' atexit goes to env (nullptr: the game's).
void SharedGlobals_SetEnv(const SharedGlobalsEnv *env);

extern "C" {

// (cdecl; each at exit) the counted object the global points to loses a use (as Counted_Release: none when it has none or ~0) and is
// deleted (slot 0 with 1) when it has none left; the global is cleared. (A null global faults.) The STL lock the originals take
// around it (0x691de0 / 0x691ea0, twice) is left out, as in core/globals' Global7ced44_Release.
void __cdecl Global7ced48_Release();   // 0x4f7a10
void __cdecl Globalf827e8_Release();   // 0x5d1670

// (cdecl) the global set to the counted object given, which gains a use (none when it has ~0; a null one faults after the global is
// set), its release (above; 0x4f7970 Global7ced44_Release / 0x5d15d0 Globalf827dc_Release of core/globals for 0x7ced44 / 0xf827dc) given to atexit (0x67a7d0); returns the object.
// The STL lock is left out as in the releases.
u8 *__cdecl Global7ced44_Set(u8 *shared);   // 0x4f7770
u8 *__cdecl Global7ced48_Set(u8 *shared);   // 0x4f77f0
u8 *__cdecl Globalf827dc_Set(u8 *shared);   // 0x5d1460
u8 *__cdecl Globalf827e8_Set(u8 *shared);   // 0x5d14e0
u8 *__cdecl Globalfb1dc0_Set(u8 *shared);   // 0x6995d0
u8 *__cdecl Globalfb1dbc_Set(u8 *shared);   // 0x6996f0
u8 *__cdecl Globalfb1dc4_Set(u8 *shared);   // 0x699810
u8 *__cdecl Globalfb1e54_Set(u8 *shared);   // 0x6a5a60
u8 *__cdecl Globalfb1e30_Set(u8 *shared);   // 0x6a5b80
u8 *__cdecl Globalfb1e58_Set(u8 *shared);   // 0x6a5ca0
u8 *__cdecl Globalfb1e34_Set(u8 *shared);   // 0x6a5dc0
u8 *__cdecl Globalfb1e38_Set(u8 *shared);   // 0x6a5ee0
u8 *__cdecl Globalfb1e5c_Set(u8 *shared);   // 0x6a6000
u8 *__cdecl Globalfb1e60_Set(u8 *shared);   // 0x6a6120
u8 *__cdecl Globalfb1e64_Set(u8 *shared);   // 0x6a6240
u8 *__cdecl Globalfb1ecc_Set(u8 *shared);   // 0x6ac030
u8 *__cdecl Globalfb1eb4_Set(u8 *shared);   // 0x6ac150
u8 *__cdecl Globalfb1eb8_Set(u8 *shared);   // 0x6ac270
u8 *__cdecl Globalfb1ed0_Set(u8 *shared);   // 0x6ac390
u8 *__cdecl Globalfb1ed4_Set(u8 *shared);   // 0x6ac4b0
u8 *__cdecl Globalfb1ed8_Set(u8 *shared);   // 0x6ac5d0
void __cdecl Globalfb1dc0_Release();   // 0x699650
void __cdecl Globalfb1dbc_Release();   // 0x699770
void __cdecl Globalfb1dc4_Release();   // 0x699890
void __cdecl Globalfb1e54_Release();   // 0x6a5ae0
void __cdecl Globalfb1e30_Release();   // 0x6a5c00
void __cdecl Globalfb1e58_Release();   // 0x6a5d20
void __cdecl Globalfb1e34_Release();   // 0x6a5e40
void __cdecl Globalfb1e38_Release();   // 0x6a5f60
void __cdecl Globalfb1e5c_Release();   // 0x6a6080
void __cdecl Globalfb1e60_Release();   // 0x6a61a0
void __cdecl Globalfb1e64_Release();   // 0x6a62c0
void __cdecl Globalfb1e3c_Release();   // 0x6a6360
void __cdecl Globalfb1e44_Release();   // 0x6a6400
void __cdecl Globalfb1e4c_Release();   // 0x6a64a0
void __cdecl Globalfb1ecc_Release();   // 0x6ac0b0
void __cdecl Globalfb1eb4_Release();   // 0x6ac1d0
void __cdecl Globalfb1eb8_Release();   // 0x6ac2f0
void __cdecl Globalfb1ed0_Release();   // 0x6ac410
void __cdecl Globalfb1ed4_Release();   // 0x6ac530
void __cdecl Globalfb1ed8_Release();   // 0x6ac650
void __cdecl Globalfb1ebc_Release();   // 0x6ac6f0
void __cdecl Globalfb1ec4_Release();   // 0x6ac790
}
