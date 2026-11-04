// The Direct3D setup's viewport: reimplementation of 0x572dd0, called at 0x5733ab.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x572dd0 (thiscall; name invented, unconfirmed): on an object with the IDirect3D3 at +4 and the device at +8. CreateViewport (COM slot 0x18) into +0xc, AddViewport on the device (0x14),
// SetViewport2 (viewport's 0x44) of size 0x2c: at 0, 0, the screen's size 0x8bcbd0, 0x8bcbd4 (signed, as dwords, then as floats for the clip size), clip at 0, 0, z 0 to 1; SetCurrentViewport (the
// device's 0x30). Each result kept at 0x879cd4, a failure's message logged, and any but 0 pushed (kind 3) and 0 returned at once. Returns 1.
u32 __fastcall D3dSetup_CreateViewport(u8 *self);
}
