// The EAX 2.0 wrappers of a DirectSound buffer (the class name is invented): +4 the buffer's IKsPropertySet. The property set GUIDs are
// in the exe: 0x70aa50 DSPROPSETID_EAX20_BufferProperties {0306A6A8-B224-11D2-99E5-0000E8D8C722}, 0x70aa60 the listener's (...A6A7).
// Reimplementation of 0x6c4670, 0x6c46b0, 0x6c4760 and 0x6d6cf0; the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c4760 (thiscall (out, unused), pops 8): IKsPropertySet::Get (EAX20 buffer properties, id 1: all parameters, no instance data,
// 0x38 bytes) into a stack block that is also the returned-size out (sic), which is then dropped: *out = 0 when it succeeds, else -1.
// Returns out.
u32 *__fastcall EaxBuffer_QueryAll(u8 *buffer, void *, u32 *out, u32 unused);

// 0x6c4670 (thiscall (out, value), pops 8): without a property set, *out = -1; else its Set (EAX20 buffer properties, id 0xb, no
// instance data, the value's own stack slot, 4 bytes) and *out = its result. Returns out.
u32 *__fastcall EaxBuffer_SetProperty0b(u8 *buffer, void *, u32 *out, u32 value);
// 0x6c46b0 (thiscall (out, properties), pops 8): without a property set, *out = -1; else its Set (EAX20 buffer properties, id 1: all
// parameters, no instance data, 0x38 bytes: the properties' dwords 3, 4, 11, 5-10, 0-2, 12, then 9) and *out = its result. Returns out.
u32 *__fastcall EaxBuffer_SetAll(u8 *buffer, void *, u32 *out, const u32 *properties);

// 0x6d6cf0 (thiscall (key, properties), pops 8): the object's slot 4 (key, &buffer) gives a buffer (none: nothing done), asked
// (QueryInterface) for IKsPropertySet (0x707fe0; failing: nothing more); its Set of the EAX20 listener properties (0x70aa60, id 1: all
// parameters, no instance data, 0x34 bytes: the 12 dwords of properties and 3) and Release. (The buffer is not released, sic.)
void __fastcall EaxListener_SetAll(u8 *self, void *, u32 key, const u32 *properties);

}  // extern "C"
