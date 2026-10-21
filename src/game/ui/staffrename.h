// The staff member's rename dialog (built from the script 0x7572e8 with this handler; its dwords +0xa0: 0 the staff member, 1 an
// object of the TrackedSource kind). Reimplementation of 0x4b6320. The name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b6320 (cdecl (window, message, a, b)): 0x14: the key maps enabled (KeyMaps_EnableAllAndClearFlag), the dword 1's position cleared
// (TrackedSource_Clear), the focus restored (Focus_Restore), then the default handler (Window_CallDefault), whose result is returned.
// 0x100 with a -1: the staff member (dword 0) named from the buffer 0x7cc360 (Staff_SetName), the dword 1's slot 1 called, the window sent
// 4 (Window_SendMessage(4, 0, 0)), 0 returned; with a -2: sent 4, 0 returned. 0x804: sent 4, 0 returned. 0x802: the window posted 0x100
// with -1 (Window_PostMessage(0x100, -1, 0): the confirmation), then the default handler's result. Any other message (0x100 with another
// a too): the default handler's result.
s32 __cdecl StaffRename_Handler(u8 *window, u32 message, s32 a, u32 b);

}  // extern "C"
