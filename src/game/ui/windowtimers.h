// The GUI library's window timers: 0x20-byte nodes from a pool (0x66214f; [0xfaa630] the nodes, [0xfaa62c] their number, the byte
// +0x14 set when free) kept in a list from [0xfaa634]: the window +0, the time last fired +4, the interval +8, the id +0xc, the enabled
// flag +0x10, the next +0x18 and the previous +0x1c. Each fires by posting its window 0x10 (id, n) (0x661fe5). Names unconfirmed.
// Reimplementation of 0x661faa, 0x6620e2 and 0x662500.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x661faa (thiscall; also the pool's element destructor; name unconfirmed): the node unlinked: the previous one's next set to its next and
// the next one's previous to its previous (each when set). EAX is left as its previous when it has a next, else the node.
u8 *__fastcall WindowTimer_Unlink(u8 *node);

// 0x6620e2 (thiscall (window, id), pops 8; name unconfirmed): the last node from this one on (by +0x18, searched recursively from the
// end) whose window and id are the ones given; 0 when none is.
u8 *__fastcall WindowTimer_Find(u8 *node, void *, u32 window, u32 id);

// 0x662500 (thiscall (flags), pops 4; name unconfirmed): the scalar deleting destructor: unlinked (WindowTimer_Unlink), then with flag 1
// given back to the pool (Fn_661f9b: its byte +0x14 set). Returns the node.
u8 *__fastcall WindowTimer_Delete(u8 *node, void *, u32 flags);
}
