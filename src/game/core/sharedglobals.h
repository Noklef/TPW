// The counted shared objects kept in globals (each set by a setter that gives it a use and its release to atexit): their releases.
// Reimplementation of 0x4f7a10, 0x699650, 0x699770, 0x699890, 0x6a5ae0, 0x6a5c00, 0x6a5d20, 0x6a5e40, 0x6a5f60, 0x6a6080, 0x6a61a0,
// 0x6a62c0, 0x6a6360, 0x6a6400, 0x6a64a0, 0x6ac0b0, 0x6ac1d0, 0x6ac2f0, 0x6ac410, 0x6ac530, 0x6ac650, 0x6ac6f0, 0x6ac790. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// (cdecl; each at exit) the counted object the global points to loses a use (as Counted_Release: none when it has none or ~0) and is
// deleted (slot 0 with 1) when it has none left; the global is cleared. (A null global faults.) The STL lock the originals take
// around it (0x691de0 / 0x691ea0, twice) is left out, as in core/globals' Global7ced44_Release.
void __cdecl Global7ced48_Release();   // 0x4f7a10
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
