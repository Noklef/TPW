// std::locale's facet lookups of the game's C++ library (names are the library's own). The library's lock std::_Lockit is left out, as in its other ports.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x699ce0 (thiscall (id, transparent), pops 8): locale::_Getfacet: the facet at index id of the array +8 of the _Locimp behind self, with an index below the count +0xc. None, a
// transparent call and a transparent _Locimp (byte +0x14) look in the global _Locimp (0xfb1dd0) the same way.
u8 *__fastcall Locale_Getfacet(u8 **self, void *, u32 id, u32 transparent);

// 0x699d50 (thiscall): locale::_Iscloc: 1 when the _Locimp behind self is the classic one (0xfb1dcc), or is transparent and the global one (0xfb1dd0) is.
u8 __fastcall Locale_Iscloc(u8 **self);

}  // extern "C"
