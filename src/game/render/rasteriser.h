// Choosing the software rasteriser's span routine: reimplementation of 0x569f60.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x569f60 (stdcall; flags, pops 4): 0xfbe044 = one of the routines at 0x6f19f0-0x6f1a20, by the flags.
// With flag 0x100: 0x200000 picks [0x6f19f8]; otherwise 0xfbe010 = (0x10, or 0xf0 with flag 0x200) / 4 and flag
// 0x800 picks [0x6f19f4], else [0x6f1a20]. Without it: no 0x10000000 picks [0x6f19f0]; with it 0x20000000
// picks [0x6f1a1c], else 0x40000000 [0x6f1a18], else [0x6f1a14]. Returns 1.
u32 __stdcall Rasteriser_ChooseSpan(u32 flags);

// 0x569f20 (cdecl; name unconfirmed): EMMS (the MMX state is cleared for the x87 unit); returns 1.
u32 __cdecl Rasteriser_ClearMmx();
}
