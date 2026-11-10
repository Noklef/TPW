// The screen grab (0x561760 and its steps: 0x561610 takes the front surface's pixels, 0x561370 writes them out): an object with the
// pixels at +0 (a tracked global block), the converted ones at +4 and the pixel count at +8. Reimplementation of the conversion to
// 24-bit pixels, 0x561460. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x561460 (thiscall; name unconfirmed): by the display's depth (0x8bcbe4): 32 bits, the count's pixels are packed in place from 4
// bytes to 3 (bytes 0-2 kept; the pixels pointer re-read for each byte); 24 or 32, the pixels move to +4 (+0 cleared); 16, a tracked
// block of count * 3 + 1 bytes (TrackedGlobal_Alloc) goes to +4 and each u16 pixel becomes a dword stored 3 bytes after the last:
// (pixel & [0x8bcc6c]) << (8 - [0x8bcc70] - [0x8bcc74]) | (pixel & [0x8bcc54]) << (0x18 - [0x8bcc58] - [0x8bcc5c]) | (pixel & [0x8bcc60])
// << (0x10 - [0x8bcc64] - [0x8bcc68]) (shift counts taken mod 32), then the pixels are freed (TrackedGlobal_Free) and +0 cleared; when
// the allocation fails the pixels are freed and +0 cleared, the error (2, 0xb0754000) is pushed (EngineErrors_Push on 0x879ad0) and 0 is
// returned. Any other depth leaves everything. Returns 1.
u32 __fastcall ScreenGrab_ToRgb24(u8 *grab, void *);

// 0x561610 (thiscall; name unconfirmed): +8 = the display's width * height ([0x8bcbd0], [0x8bcbd4]), +0 a tracked block of that many times the depth in bits (sic); none: the error (2, 0xb0754000)
// pushed, 0. The front surface ([[0x879a24]] +0x14) locked with a cleared description (flags 1), its result kept (0x879cd4, a failure logged): not 0, returns 0. Each row copied, depth >> 3 times the
// width in bytes, the source stepping by the description's pitch; the surface unlocked and that result kept: 1 when it is 0.
u32 __fastcall ScreenGrab_TakeFront(u8 *grab, void *);

// 0x5617c0 (thiscall on the grab at 0x8798d0; name; pops 4; name unconfirmed): the file name to grab to next: a name is measured, then
// copied with its 0 to +0xc (0x80 bytes, unchecked, sic) and +0x8c pointed at the copy; a null name leaves both. Returns 1.
u32 __fastcall ScreenGrab_SetName(u8 *grab, void *, const char *name);

// 0x550460 (cdecl; name unconfirmed): the next free grab name: a local FileStream (StreamBase_Ctor, vtable 0x6fd930) closed
// (StreamBase_Close), the counter at 0x879084 raised and "Scr%05ld.tga" of it printed (CRT sprintf, a 0x14-byte buffer) and opened
// (FileStream_OpenPath, mode 0x80000001), again while that succeeds (its result not negative); then the stream closed, the name
// given to the grab at 0x8798d0 (ScreenGrab_SetName) and the stream destroyed (StreamBase_Dtor).
void __cdecl ScreenGrab_NextName();

// 0x561370 (thiscall (path), pops 4; name invented, unconfirmed): the 24-bit pixels at +4 written as a .tga (CRT_fopen with 0x766128): an 18-byte header (type 2, the display's [0x8bcbd0] x [0x8bcbd4], 24 bits), then the rows
// from the last to the first. Returns 1; no file: 0 with kind 2, code 0xb0754002 on the engine's errors.
u32 __fastcall ScreenGrab_WriteTga(u8 *grab, void *, const char *path);

// 0x561760 (thiscall; name invented, unconfirmed): with a name set (+0x8c): the front surface's pixels taken (ScreenGrab_TakeFront), made 24-bit (ScreenGrab_ToRgb24) and written (ScreenGrab_WriteTga), a failure returning 0 at
// that step; then the block +4 freed (TrackedGlobal_Free), it and the name cleared. Else, and then, 1.
u32 __fastcall ScreenGrab_Save(u8 *grab);
}

// Calls by the game's address (the bench's recorders): the C library's files, the engine's errors, the grab's steps and the free.
#include "../core/addrcalls.h"
void ScreenGrab_SetCalls(const AddrCall *table, u32 count);

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(s32, gScreenGrab_Count_879084, 0x00879084, 0x4);
