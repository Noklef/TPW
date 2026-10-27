// The GUI's named objects: a singly linked list [0xfaa624] of 0x14-byte nodes (+0 the object, +4 its name, 12 characters at most, not
// terminated when that long, +0x10 the next node), the objects deleted with the list when the GUI shuts down (0x658810). Reimplementation
// of 0x661a93, 0x661b3c and 0x661bab. The names are unconfirmed (0x661a93 and 0x661bab have no direct callers).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x661a93 (cdecl (object, name); no direct callers): for an object not 0 and a name not empty (CRT strlen), a new node (Mem_Alloc 0x14;
// none: nothing) with the object and the name's first 12 characters (CRT strncpy: zeros after a shorter one), appended at the list's end
// ([0xfaa624] when empty). 1 in AX when added, else 0 (EAX's upper half the strlen's or the allocation's, kept by the caller's sic: AX only).
u16 __cdecl GuiNamed_Add(u8 *object, const char *name);

// 0x661b3c (cdecl; called by the GUI's shutdown 0x658810): each node from [0xfaa624] on: its object, when not 0, deleted (vtable slot 0
// with 1), the node freed (Mem_Free). [0xfaa624] is left as it was (sic: the GUI's shutdown does not clear it either).
void __cdecl GuiNamed_FreeAll();

// 0x661bab (cdecl (name); no direct callers): the object of the first node whose name +4 equals the name (CRT strcmp: the node's 12
// characters read on past them when not terminated), else 0.
u8 *__cdecl GuiNamed_Find(const char *name);
}
