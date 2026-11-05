// The C++ library's moneypunct facets, char and wchar_t (MSVC6's own names): what they take from localeconv at the time they are made.
// Layout: +0 vtable, +4 reference count, +8 grouping, +0xc decimal point and thousands separator (one char each, or one u16 each at +0xc and +0xe),
// +0x10 currency symbol, +0x14 plus sign, +0x18 minus sign, +0x1c fraction digits, +0x20 / +0x24 the positive / negative pattern, +0x28 international (a byte).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6a8830 / 0x6a18a0 (thiscall (locinfo, refs, international), pop 12): moneypunct<char> / <wchar_t>'s constructor: the vtable 0x709fdc / 0x709d74 and Init. Returns the facet. The locinfo is not read.
void *__fastcall MoneypunctC_Ctor(u8 *facet, void *, const void *locinfo, u32 refs, u8 international);
void *__fastcall MoneypunctW_Ctor(u8 *facet, void *, const void *locinfo, u32 refs, u8 international);

// 0x6ab880 / 0x6a4f20 (thiscall (locinfo), pops 4): _Init: new copies of localeconv's texts, and the numbers and patterns; a sign position outside 0..4 gives the
// default sign text (0x786de4, 0x7638f4) and pattern (0x709d10), as does an international facet or a precedes or space flag other than 0 or 1 for the pattern.
void __fastcall MoneypunctC_Init(u8 *facet, void *, const void *locinfo);
void __fastcall MoneypunctW_Init(u8 *facet, void *, const void *locinfo);

// 0x6a1e70 (thiscall (out, space, precedes, position), pops 16; name invented, unconfirmed): *out the pattern from 0x709d18, or the default at 0x709d10 for an international facet or a flag or position out of range. Returns out.
u32 *__fastcall Moneypunct_Makepat(u8 *facet, void *, u32 *out, u32 space, u32 precedes, u32 position);
// 0x6a2400 / 0x6a91b0: wchar_t's / char's destructor bodies: the vtable (0x709d74 / 0x709fdc), the four texts freed, the base vtable 0x709b44.
void __fastcall MoneypunctW_Tidy(u32 *self, void *);
void __fastcall MoneypunctC_Tidy(u32 *self, void *);

}  // extern "C"
