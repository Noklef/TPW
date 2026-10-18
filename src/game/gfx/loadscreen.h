// The loading screen: reimplementation of 0x5879c0 and 0x586d70.
//
// The loading screen is the object at 0x8c4748 (its static initializer 0x586d40: +0 = 0, +4 = -1, +8 = 500; 0x586d70 its atexit
// destructor). 0x587380 shows it from a folder ("data\init", 0x54dea9): it counts its shows at 0x8c478c (0x587963), sets bit 0x40000000 of
// 0x8bcbc8 and takes timeGetTime at 0x8c4780 when shown first (0x587d0d). +0x10 and +0x14 point to tracked blocks (a dword handle each,
// as TrackedBlock_Dtor's, core/gameheap).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5879c0 (thiscall; name unconfirmed): once shown ([0x8c478c] not 0), timeGetTime (the import at 0x6fd3ec) is asked until 3000 ms
// have passed since [0x8c4780] (unsigned). Then the blocks at +0x10 and +0x14, each when set: its handle freed (TrackedGlobal_Free) and
// cleared when set, the block freed (GameHeap_Free) and the field cleared. Bit 0x40000000 of 0x8bcbc8 is cleared. Returns 1.
u32 __fastcall LoadScreen_Close(u8 *screen);

// 0x586d70 (cdecl; the atexit destructor of the loading screen at 0x8c4748; name unconfirmed): LoadScreen_Close's steps on it, but
// returning nothing.
void __cdecl LoadScreen_StaticDtor();
}
