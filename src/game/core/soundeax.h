// The EAX 2.0 wrappers of a DirectSound buffer (the class name is invented): +4 the buffer's IKsPropertySet. The property set GUIDs are
// in the exe: 0x70aa50 DSPROPSETID_EAX20_BufferProperties {0306A6A8-B224-11D2-99E5-0000E8D8C722}, 0x70aa60 the listener's (...A6A7).
// Reimplementation of 0x6c4760; the name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c4760 (thiscall (out, unused), pops 8): IKsPropertySet::Get (EAX20 buffer properties, id 1: all parameters, no instance data,
// 0x38 bytes) into a stack block that is also the returned-size out (sic), which is then dropped: *out = 0 when it succeeds, else -1.
// Returns out.
u32 *__fastcall EaxBuffer_QueryAll(u8 *buffer, void *, u32 *out, u32 unused);

}  // extern "C"
