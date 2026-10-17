// The track types' slots (0x14 records of 0x140 bytes from 0x804370; the first byte of each at 0x804488 + 0x140 * n says it is taken): reimplementation of 0x51fa20, the
// loader TrackType_LoadSam (0x414410) calls. Named by what it does until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x51fa20 (cdecl; stream): -1 when [0x816d40] is 0; otherwise the first of the 0x140-byte slots from 0x804488 whose first byte is 0 gets 0x140 bytes read into the slot's 0x140-byte
// record at 0x804370 (Object_CallSlot8Product with the stream) and its index is returned; -1 when none is free below 0x80c7c8.
s32 __cdecl TrackTypeSlot_ReadIntoFree(u8 *stream);

}  // extern "C"
