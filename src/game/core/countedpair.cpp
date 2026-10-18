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

// The vector's counts: (a - b) / 0x24, signed (the compiler's size() and capacity arithmetic).
s32 count36(u32 a, u32 b) { return (s32)(a - b) / 0x24; }

CountedPair *pairAt(u32 address) { return reinterpret_cast<CountedPair *>((uintptr_t)address); }

// _Ucopy: [first, last) copy-constructed from to on (a null slot skipped: placement new's check). Returns the end.
u32 ucopy(u32 first, u32 last, u32 to) {
    for (; first != last; first += 0x24, to += 0x24)
        if (to)
            CountedPair_CopyCtor(pairAt(to), nullptr, pairAt(first));
    return to;
}

// _Ufill: n copies of x constructed from to on (null slots skipped). Returns the end.
u32 ufill(u32 to, u32 n, const CountedPair *x) {
    for (; n; --n, to += 0x24)
        if (to)
            CountedPair_CopyCtor(pairAt(to), nullptr, x);
    return to;
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

void __fastcall CountedPairVector_InsertN(u8 *v, void *, u32 p, u32 m, const CountedPair *x) {
    u32 &first = *reinterpret_cast<u32 *>(v + 4), &last = *reinterpret_cast<u32 *>(v + 8), &end = *reinterpret_cast<u32 *>(v + 0xc);
    if ((u32)count36(end, last) < m) {  // a new block: the head, the copies, the tail; the old elements destroyed and their block freed
        const u32 size = first ? (u32)count36(last, first) : 0;
        const s32 n = (s32)(size + (m < size ? size : m));
        const u32 s = (u32)(uintptr_t)GameHeap_Alloc((u32)(n < 0 ? 0 : n) * 0x24);
        const u32 q = ufill(ucopy(first, p, s), m, x);
        ucopy(p, last, q);
        for (u32 e = first; e != last; e += 0x24)
            CountedPair_Clear(reinterpret_cast<u8 *>((uintptr_t)e));
        GameHeap_Free(reinterpret_cast<void *>((uintptr_t)first));
        end = s + (u32)n * 0x24;
        const u32 old = first ? (u32)count36(last, first) : 0;
        first = s;
        last = s + (m + old) * 0x24;
        return;
    }
    const u32 bytes = m * 0x24;
    if ((u32)count36(last, p) < m) {  // past the end: the tail moved up, the copies beyond it, then the old tail's slots assigned
        ucopy(p, last, p + bytes);
        ufill(last, m - (u32)count36(last, p), x);
        for (u32 e = p; e != last; e += 0x24)
            CountedPair_Assign(pairAt(e), nullptr, x);
        last += bytes;
        return;
    }
    if (m == 0)
        return;
    ucopy(last - bytes, last, last);  // the last m moved up, the rest shifted back (copy_backward), the gap assigned
    for (u32 from = last - bytes, to = last; from != p;) {
        from -= 0x24;
        to -= 0x24;
        CountedPair_Assign(pairAt(to), nullptr, pairAt(from));
    }
    for (u32 e = p; e != p + bytes; e += 0x24)
        CountedPair_Assign(pairAt(e), nullptr, x);
    last += bytes;
}

u32 __stdcall CountedPair_UninitCopy(u32 first, u32 last, u32 to) { return ucopy(first, last, to); }

void __stdcall CountedPair_UninitFill(u32 to, u32 n, const CountedPair *x) { ufill(to, n, x); }

void __cdecl CountedPair_Construct(CountedPair *p, const CountedPair *x) {
    if (p)
        CountedPair_CopyCtor(p, nullptr, x);
}

void __cdecl CountedPairVector_Construct(u8 *v, const u8 *x) {
    if (!v)
        return;
    v[0] = x[0];
    const u32 xFirst = *reinterpret_cast<const u32 *>(x + 4);
    s32 n = xFirst ? count36(*reinterpret_cast<const u32 *>(x + 8), xFirst) : 0;
    if (n < 0)
        n = 0;
    const u32 s = (u32)(uintptr_t)GameHeap_Alloc((u32)n * 0x24);
    *reinterpret_cast<u32 *>(v + 4) = s;
    const u32 e = ucopy(*reinterpret_cast<const u32 *>(x + 4), *reinterpret_cast<const u32 *>(x + 8), s);  // (x's ends read again, sic)
    *reinterpret_cast<u32 *>(v + 8) = e;
    *reinterpret_cast<u32 *>(v + 0xc) = e;
}

s32 __fastcall Vector36_Size(const u8 *vector) {
    const u32 begin = *reinterpret_cast<const u32 *>(vector + 4);
    if (!begin)
        return 0;
    return (s32)(*reinterpret_cast<const u32 *>(vector + 8) - begin) / 36;
}
}
