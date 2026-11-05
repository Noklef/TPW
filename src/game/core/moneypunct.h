// The C++ library's moneypunct facets, char and wchar_t (MSVC6's own names): what they take from localeconv at the time they are made.
// Layout: +0 vtable, +4 reference count, +8 grouping, +0xc decimal point and thousands separator (one char each, or one u16 each at +0xc and +0xe),
// +0x10 currency symbol, +0x14 plus sign, +0x18 minus sign, +0x1c fraction digits, +0x20 / +0x24 the positive / negative pattern, +0x28 international (a byte).
#pragma once

#include "../../../re/types.h"
#include "stdstring.h"
#include "stdwstring.h"

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

// The facets' strings by value (thiscall, pops 4; the allocator byte the low byte of out, sic): 0x695b10 do_grouping (numpunct and moneypunct, the text at +8); moneypunct's do_curr_symbol +0x10, do_positive_sign +0x14, do_negative_sign +0x18: narrow 0x6a8a50, 0x6a8b20, 0x6a8bf0, wide 0x69c900, 0x69c9d0, 0x6a1b20.
StdString *__fastcall Facet_Grouping(const u8 *self, void *, StdString *out);
StdString *__fastcall MoneypunctC_CurrSymbol(const u8 *self, void *, StdString *out);
StdString *__fastcall MoneypunctC_PositiveSign(const u8 *self, void *, StdString *out);
StdString *__fastcall MoneypunctC_NegativeSign(const u8 *self, void *, StdString *out);
StdWString *__fastcall MoneypunctW_CurrSymbol(const u8 *self, void *, StdWString *out);
StdWString *__fastcall MoneypunctW_PositiveSign(const u8 *self, void *, StdWString *out);
StdWString *__fastcall MoneypunctW_NegativeSign(const u8 *self, void *, StdWString *out);
// 0x6a73f0, 0x6a0370 (thiscall, pops 0x14; the third slot of the vtables 0x709fac, 0x709c0c: messages::do_get, by its place, names invented, unconfirmed): which 0 the text at +8, 1 that at +0xc, others a copy of the fallback.
StdString *__fastcall Facet709fac_Get(const u8 *self, void *, StdString *out, u32, u32, u32 which, const StdString *fallback);
StdWString *__fastcall WFacet709c0c_Get(const u8 *self, void *, StdWString *out, u32, u32, u32 which, const StdWString *fallback);

}  // extern "C"
