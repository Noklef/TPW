#include "timeget.h"

#include "stdstring.h"
#include "streambufiter.h"

namespace {

// istreambuf_iterator's equal, operator* and ++ (inlined in the originals, the same steps): the exe's own functions (streambufiter).
bool equal(NumGetIter *it, NumGetIter *x, char) { return StreambufIter_Equal(reinterpret_cast<StreambufIter *>(it), nullptr, reinterpret_cast<StreambufIter *>(x)) != 0; }
bool equal(NumGetIter *it, NumGetIter *x, u16) { return WStreambufIter_Equal(reinterpret_cast<WStreambufIter *>(it), nullptr, reinterpret_cast<WStreambufIter *>(x)) != 0; }
char deref(NumGetIter *it, char) { return static_cast<char>(*StreambufIter_Deref(reinterpret_cast<StreambufIter *>(it), nullptr)); }
u16 deref(NumGetIter *it, u16) { return *WStreambufIter_Deref(reinterpret_cast<WStreambufIter *>(it), nullptr); }
void increment(NumGetIter *it, char) { StreambufIter_Inc(reinterpret_cast<StreambufIter *>(it), nullptr); }
void increment(NumGetIter *it, u16) { WStreambufIter_Inc(reinterpret_cast<WStreambufIter *>(it), nullptr); }

// The count string's operator[] (not const: it freezes the block).
char &at(StdString &s, u32 i) { return *StdString_At(&s, nullptr, i); }

template <typename C> s32 getloctxt(NumGetIter *first, NumGetIter *last, u32 n, const C *names) {
    for (const C *p = names; *p; ++p) {
        if (*p == names[0]) ++n;
    }
    StdString counts;
    const u8 allocator = 0;   // (the original's byte is a stack byte nothing set: never read)
    StdString_CtorFill(&counts, nullptr, n, 0, &allocator);
    s32 answer = -2;
    for (u32 j = 1;; ++j) {
        bool prefix = false;
        for (u32 i = 0, k = 0; i < n; ++i) {
            while (names[k] && names[k] != names[0]) ++k;
            const char step = static_cast<char>(j < 0x7f ? j : 0x7f);
            if (at(counts, i)) {
                k += static_cast<s32>(at(counts, i));
            } else if (names[k += j] == names[0] || !names[k]) {
                at(counts, i) = step;
                answer = static_cast<s32>(i);
            } else if (equal(first, last, C()) || names[k] != deref(first, C())) {
                at(counts, i) = step;
            } else {
                prefix = true;
            }
        }
        if (!prefix || equal(first, last, C())) break;
        increment(first, C());
        answer = -1;
    }
    StdString_Tidy(&counts, nullptr, 1);
    return answer;
}

template <typename C> u32 *get_name(u8 *self, u32 names_at, u32 *out, NumGetIter &first, NumGetIter &last, u32 *state, u8 *tm, u32 field) {
    const s32 index = getloctxt(&first, &last, 0, *reinterpret_cast<const C *const *>(self + names_at));
    if (index < 0) {
        *state |= 2;
    } else {
        *reinterpret_cast<s32 *>(tm + field) = index >> 1;
    }
    const u32 *const from = reinterpret_cast<const u32 *>(&first);
    out[0] = from[0];
    out[1] = from[1];
    return out;
}

}  // namespace

extern "C" {

s32 __cdecl TimeGet_Getloctxt(NumGetIter *first, NumGetIter *last, u32 n, const char *names) { return getloctxt(first, last, n, names); }

s32 __cdecl WTimeGet_Getloctxt(NumGetIter *first, NumGetIter *last, u32 n, const u16 *names) { return getloctxt(first, last, n, names); }

u32 *__fastcall TimeGet_DoGetWeekday(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_name<char>(self, 8, out, first, last, state, tm, 0x18); }

u32 *__fastcall TimeGet_DoGetMonthname(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_name<char>(self, 0xc, out, first, last, state, tm, 0x10); }

u32 *__fastcall WTimeGet_DoGetWeekday(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_name<u16>(self, 8, out, first, last, state, tm, 0x18); }

u32 *__fastcall WTimeGet_DoGetMonthname(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_name<u16>(self, 0xc, out, first, last, state, tm, 0x10); }

}  // extern "C"
