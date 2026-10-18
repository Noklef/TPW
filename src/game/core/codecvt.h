// The char-to-char conversion facet of the C++ library (vtable 0x709b90, codecvt-like: its other slots are 0x5d1400, Stub_One and
// 0x5d1420): reimplementation of its length slot 0x5d1440.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d1440 (a virtual slot whose this is unused, so stdcall; state, from, to, limit, pops 16): to - from, at most limit (unsigned).
u32 __stdcall Span_Limited(u32 unused, u32 from, u32 to, u32 limit);
}
