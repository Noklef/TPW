#include "countedpair.h"

#include <cstdint>

#include "buffers.h"
#include "gameheap.h"
#include "trivial.h"
#include "wtraits.h"

namespace {

const char *const kNarrowNull = reinterpret_cast<const char *>(0x006fe5b0);  // the narrow _Nullstr
const u16 *const kWideNull = reinterpret_cast<const u16 *>(0x006fd6c4);     // the wide _Nullstr

// assign(const char *): strlen, _Grow(n, true), the characters copied (REP MOVSD, then the 0-3 bytes left), _Eos(n).
void assignNarrow(StdString *s, const char *from) {
    u32 n = 0;
    while (from[n])
        ++n;
    if (!StdString_Grow(s, nullptr, n, 1))
        return;
    char *to = s->ptr;
    const char *src = from;
    for (u32 i = n >> 2; i; --i, to += 4, src += 4)
        *reinterpret_cast<u32 *>(to) = *reinterpret_cast<const u32 *>(src);
    for (u32 i = n & 3; i; --i)
        *to++ = *src++;
    s->len = n;
    s->ptr[n] = 0;
}

u32 wideLength(const u16 *from) {
    u32 n = 0;
    while (from[n])
        ++n;
    return n;
}

// The rest of assign(const wchar_t *) once the length is checked: _Grow(n, true) inlined (a shared block: one count less and, with n 0,
// _Tidy(false), else _Copy(n); an own one: n 0 _Tidy(true), else replaced (_Tidy(true), _Copy(n)) when its capacity is above 31 or below n),
// then the characters copied and _Eos(n).
void assignWide(StdWString *s, const u16 *from, u32 n) {
    u8 *const object = reinterpret_cast<u8 *>(s);
    u8 *const chars = reinterpret_cast<u8 *>(s->ptr);
    if (chars && chars[-1] != 0 && chars[-1] != 0xff) {
        if (n == 0) {
            chars[-1] -= 1;
            CountedBuf_Reset(object, nullptr, 0);
            return;
        }
        StdWString_Copy(s, nullptr, n);
    } else {
        if (n == 0) {
            CountedBuf_Reset(object, nullptr, 1);
            return;
        }
        if (s->res > 31 || s->res < n) {
            CountedBuf_Reset(object, nullptr, 1);
            StdWString_Copy(s, nullptr, n);
        }
    }
    WTraits_Copy(s->ptr, from, n);
    s->len = n;
    s->ptr[n] = 0;
}

}  // namespace

extern "C" {

CountedPair *__fastcall CountedPair_CtorWide(CountedPair *p, void *, StdWString s) {
    *reinterpret_cast<u8 *>(&p->narrow.allocator) = static_cast<u8>(s.allocator);
    p->narrow.ptr = nullptr;
    p->narrow.len = 0;
    p->narrow.res = 0;
    *reinterpret_cast<u8 *>(&p->wide.allocator) = static_cast<u8>(s.allocator);
    p->wide.ptr = nullptr;
    p->wide.len = 0;
    p->wide.res = 0;
    StdWString_AssignSub(&p->wide, nullptr, &s, 0, 0xffffffff);  // (npos, the read-only 0x6fef00)
    p->value = 0;
    if (s.ptr) {
        u8 *const count = reinterpret_cast<u8 *>(s.ptr) - 1;
        if (*count == 0 || *count == 0xff)
            GameHeap_Free(reinterpret_cast<u8 *>(s.ptr) - 2);
        else
            --*count;
    }
    return p;
}

CountedPair *__fastcall CountedPair_CopyCtor(CountedPair *p, void *, const CountedPair *x) {
    const u8 low = static_cast<u8>(reinterpret_cast<uintptr_t>(x));  // (the allocators' copies: the byte of x's address, sic)
    const char *const narrow = x->narrow.ptr ? x->narrow.ptr : kNarrowNull;
    p->narrow.ptr = nullptr;
    *reinterpret_cast<u8 *>(&p->narrow.allocator) = low;
    p->narrow.len = 0;
    p->narrow.res = 0;
    assignNarrow(&p->narrow, narrow);
    const u16 *const wide = x->wide.ptr ? x->wide.ptr : kWideNull;
    *reinterpret_cast<u8 *>(&p->wide.allocator) = low;
    p->wide.ptr = nullptr;
    p->wide.len = 0;
    p->wide.res = 0;
    const u32 n = wideLength(wide);
    if (Const7ffffffd_4f6080() < n)
        StdStrings_Xlen();
    assignWide(&p->wide, wide, n);
    p->value = x->value;
    return p;
}

CountedPair *__fastcall CountedPair_Assign(CountedPair *p, void *, const CountedPair *x) {
    assignNarrow(&p->narrow, x->narrow.ptr ? x->narrow.ptr : kNarrowNull);
    const u16 *const wide = x->wide.ptr ? x->wide.ptr : kWideNull;
    const u32 n = wideLength(wide);
    if (n > 0x7ffffffd)
        StdStrings_Xlen();
    assignWide(&p->wide, wide, n);
    p->value = x->value;
    return p;
}
}
