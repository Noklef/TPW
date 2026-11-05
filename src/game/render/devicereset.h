// The display device's reset.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

void DeviceReset_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x5774c0 (name invented, unconfirmed): in render mode 1 (0x8bd524) the texture cache released, slot 5 of [0x8bd52c], the display object at [0x879a24] released and
// restored, slot 6 of [0x8bd52c], the cache's format remade (-1) and slot 1 of [0x879ab0]; in other modes only the display's two. Then 0x47ea00 and 1. A step that returns 0 ends
// it: the cache released, the device destroyed, the display's 0x5640a0, the error (3, 0xb0754014) pushed, 0.
u32 __cdecl Engine_ResetDevice();

}
