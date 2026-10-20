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

// 0x5617c0 (thiscall on the grab at 0x8798d0; name; pops 4; name unconfirmed): the file name to grab to next: a name is measured, then
// copied with its 0 to +0xc (0x80 bytes, unchecked, sic) and +0x8c pointed at the copy; a null name leaves both. Returns 1.
u32 __fastcall ScreenGrab_SetName(u8 *grab, void *, const char *name);

// 0x550460 (cdecl; name unconfirmed): the next free grab name: a local FileStream (StreamBase_Ctor, vtable 0x6fd930) closed
// (StreamBase_Close), the counter at 0x879084 raised and "Scr%05ld.tga" of it printed (CRT sprintf, a 0x14-byte buffer) and opened
// (FileStream_OpenPath, mode 0x80000001), again while that succeeds (its result not negative); then the stream closed, the name
// given to the grab at 0x8798d0 (ScreenGrab_SetName) and the stream destroyed (StreamBase_Dtor).
void __cdecl ScreenGrab_NextName();
}
