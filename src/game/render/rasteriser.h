// The software rasteriser device's methods (render/device: virtual base table 0x7013c8): reimplementation of the span routine choice
// 0x569f60, the MMX clear 0x569f20, the setting 0x569f30 and the pixel read 0x56a510.
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

// 0x569f30 (stdcall; _, value, pops 8): with a value, 0xfbe038 = value; else 0xfbe044 = the dword at 0x6f1a10;
// returns 1.
u32 __stdcall Setting569f30_Set(u32 unused, u32 value);

// 0x56a510 (stdcall; x, y, out, pops 12): *out = the dword at (y * width + x) in the software
// renderer's buffer (0x879dac: +8 the buffer; width 0x8bcbd0); returns 1.
u32 __stdcall SoftRenderer_PixelAt(s32 x, s32 y, u32 *out);
}
