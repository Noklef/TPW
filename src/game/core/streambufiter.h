// std::istreambuf_iterator<char> and <wchar_t>'s members, emitted out of line (names by their shape, unconfirmed). An iterator is { streambuf, got: whether val holds the next character, val }.
// A streambuf's +0x1c points at the pointer to its next character, +0x2c at the count left. Its slots +0x10 (underflow) and +0x14 (uflow) answer when nothing waits there.
// Waiting: a pointer that is not null, below itself plus the count (in characters, wrapping: sic). The end: -1 for char, 0xffff in the low word for wchar_t (so a wide 0xffff in the buffer is an end too).
#pragma once

#include "../../../re/types.h"

struct StreambufIter;
struct WStreambufIter;

// All __thiscall (self in ECX), written as __fastcall with an unused EDX.
extern "C" {

// 0x6962d0, 0x69d040: operator*: the next character fetched once (the end: the streambuf dropped, and got set all the same). Returns val's address.
u8 *__fastcall StreambufIter_Deref(StreambufIter *it, void *edx);
u16 *__fastcall WStreambufIter_Deref(WStreambufIter *it, void *edx);

// 0x696330, 0x69d0a0 (x; pops 4): equal(x): both fetch, this one first; 1 when both or neither are at the end, else 0.
u32 __fastcall StreambufIter_Equal(StreambufIter *it, void *edx, StreambufIter *x);
u32 __fastcall WStreambufIter_Equal(WStreambufIter *it, void *edx, WStreambufIter *x);

// 0x698bf0: _Peek: fetches whether or not got is set; got set; at the end the streambuf dropped. Returns the character: at the end the old val.
u8 __fastcall StreambufIter_Peek(StreambufIter *it, void *edx);

// 0x698600 (self the streambuf): sbumpc: the waiting character (as unsigned) and past it, or uflow's answer.
u32 __fastcall Streambuf_Sbumpc(u8 *buffer, void *edx);

// 0x69b7f0 (self the streambuf, ch; pops 4): basic_streambuf<wchar_t>::sputc: at +0x20 and +0x30 the pointer and count to write by: ch put and returned as given, or overflow's answer.
u32 __fastcall WStreambuf_Sputc(u8 *buffer, void *edx, u32 ch);

// 0x69b830 (ch; pops 4): ostreambuf_iterator<wchar_t>::operator= (self { failed byte, streambuf }): sputc inlined; no streambuf or an end (ch itself may be 0xffff): failed set. Returns self.
u8 *__fastcall WOstreambufIter_Assign(u8 *it, void *edx, u32 ch);

// 0x6963f0, 0x69d160: _Inc: past the waiting character, or uflow; got cleared; at the end the streambuf dropped and got set.
void __fastcall StreambufIter_Inc(StreambufIter *it, void *edx);
void __fastcall WStreambufIter_Inc(WStreambufIter *it, void *edx);

// 0x695890, 0x69c5e0: num_put::_Rep: n copies of ch to the iterator { failed, streambuf } passed by value, into out (cdecl). No streambuf or an end: failed's low byte set, and it goes on.
u32 *__cdecl NumPut_Rep(u32 *out, u32 failed, u8 *buffer, u32 ch, u32 n);
u32 *__cdecl WNumPut_Rep(u32 *out, u32 failed, u8 *buffer, u32 ch, u32 n);
}
