#include "stdwstring.h"

#include <cstdint>

#include "buffers.h"
#include "gameheap.h"
#include "stdstring.h"
#include "trivial.h"
#include "wtraits.h"

namespace {

const u8 kFrozen = 0xff;
const u32 kMinSize = 0x1f;
const u32 kMaxSize = 0x7ffffffd;  // the allocator's 0x7fffffff less 2
const u16 *const kNullStr = reinterpret_cast<const u16 *>(0x006fd6c4);
const u32 kNpos = 0xffffffff;  // wstring::npos (the read-only 0x6fef00)

u8 &refs(const u16 *p) { return *reinterpret_cast<u8 *>((uintptr_t)p - 1); }

bool shared(const u16 *p) { return p && refs(p) != 0 && refs(p) != kFrozen; }

const u16 *c_str(const StdWString *s) { return s->ptr ? s->ptr : kNullStr; }

u8 *bytes(StdWString *s) { return reinterpret_cast<u8 *>(s); }

// _Tidy(true) (the counted buffer reset): releases s's block and empties s.
void tidy(StdWString *s) { CountedBuf_Reset(bytes(s), nullptr, 1); }

// A vector<wstring>: the allocator (a byte), first, last, end.
struct WVector {
    u32 allocator;
    StdWString *first;
    StdWString *last;
    StdWString *end;
};

// The strings from first up to to: 0 for a null first, else the byte span shifted signed (the vector's size and capacity).
u32 count(const StdWString *first, const StdWString *to) { return first ? (u32)((s32)((uintptr_t)to - (uintptr_t)first) >> 4) : 0; }

// The same without the null check.
u32 span(const StdWString *first, const StdWString *to) { return (u32)((s32)((uintptr_t)to - (uintptr_t)first) >> 4); }

// p moved on by k strings (wrapping).
StdWString *advance(const StdWString *p, u32 k) { return reinterpret_cast<StdWString *>((uintptr_t)p + (k << 4)); }

// _Grow(n, true) as inlined into the assigns (with no max_size check of its own): whether there
// are characters to write.
bool grow_trim(StdWString *s, u32 n) {
    if (shared(s->ptr)) {
        if (!n) {
            --refs(s->ptr);
            CountedBuf_Reset(bytes(s), nullptr, 0);
            return false;
        }
        StdWString_Copy(s, nullptr, n);
        return true;
    }
    if (!n) {
        tidy(s);
        return false;
    }
    if (kMinSize < s->res || s->res < n) {
        tidy(s);
        StdWString_Copy(s, nullptr, n);
    }
    return true;
}

// erase(p, m) as inlined into assign: shifts the tail down over [p, p + m). check: the inlined
// copy's p > len test (left out of erase(0, m), where it cannot fail).
void erase(StdWString *s, u32 p, u32 m, bool check) {
    if (check && s->len < p) StdStrings_Xran();
    StdWString_Split(s, nullptr);
    const u32 left = s->len - p;
    if (left < m) m = left;
    if (m) {
        WTraits_Move(s->ptr + p, s->ptr + p + m, left - m);
        const u32 n = s->len - m;
        if (StdWString_Grow(s, nullptr, n, 0)) ImeWString_SetLength(bytes(s), nullptr, n);
    }
}

}  // namespace

extern "C" {

void __fastcall StdWString_Copy(StdWString *s, void *, u32 n) {
    u32 res = n | kMinSize;
    if (res > kMaxSize) res = n;
    const s32 count = (s32)(res + 2);  // allocator::allocate: a negative count asks for 0
    u8 *block = static_cast<u8 *>(GameHeap_Alloc((u32)(count < 0 ? 0 : count) * 2));
    u16 *chars = reinterpret_cast<u16 *>(block + 2);
    for (u32 i = 0; i < s->len; ++i) chars[i] = s->ptr[i];  // sic: not clamped to res
    const u32 len = s->len;
    if (s->ptr) {
        if (shared(s->ptr))
            --refs(s->ptr);
        else
            GameHeap_Free(reinterpret_cast<u8 *>(s->ptr) - 2);
    }
    s->ptr = chars;
    refs(chars) = 0;
    s->len = len;
    s->res = res;
    chars[len] = 0;
}

StdWString *__fastcall StdWString_AssignSub(StdWString *s, void *, const StdWString *x, u32 pos, u32 n) {
    if (x->len < pos) StdStrings_Xran();
    const u32 rest = x->len - pos;
    if (rest < n) n = rest;
    if (s == x) {
        erase(s, pos + n, kNpos, true);
        erase(s, 0, pos, false);
    } else if (n && n == x->len && refs(c_str(x)) < kFrozen - 1) {
        tidy(s);
        s->ptr = const_cast<u16 *>(c_str(x));
        s->len = x->len;
        s->res = x->res;
        ++refs(s->ptr);
    } else {
        if (Const7ffffffd_4f6080() < n) StdStrings_Xlen();
        if (grow_trim(s, n)) {
            const u16 *src = c_str(x) + pos;
            for (u32 i = 0; i < n; ++i) s->ptr[i] = src[i];
            s->len = n;
            s->ptr[n] = 0;
        }
    }
    return s;
}

StdWString *__fastcall StdWString_AssignFill(StdWString *s, void *, u32 n, u32 ch) {
    if (n == kNpos) StdStrings_Xlen();
    if (n > kMaxSize) StdStrings_Xlen();
    if (grow_trim(s, n)) {
        for (u32 i = 0; i < n; ++i) s->ptr[i] = (u16)ch;
        s->len = n;
        s->ptr[n] = 0;
    }
    return s;
}

void __fastcall StdWString_Freeze(StdWString *s, void *) {
    if (shared(s->ptr)) {
        const u32 n = s->len;
        if (n > kMaxSize) StdStrings_Xlen();
        StdWString_Grow(s, nullptr, n, 0);  // (inlined; its own max_size check is the one above)
    }
    if (s->ptr) refs(s->ptr) = kFrozen;
}

u8 __fastcall StdWString_Grow(StdWString *s, void *, u32 n, u8 trim) {
    if (n > kMaxSize) StdStrings_Xlen();
    if (shared(s->ptr)) {
        if (!n) {
            --refs(s->ptr);
            s->ptr = nullptr;
            s->len = 0;
            s->res = 0;
            return 0;
        }
        StdWString_Copy(s, nullptr, n);
        return 1;
    }
    if (!n) {
        if (trim)
            tidy(s);
        else if (s->ptr) {
            s->len = 0;
            s->ptr[0] = 0;
        }
        return 0;
    }
    if (trim && (kMinSize < s->res || s->res < n)) {
        tidy(s);
        StdWString_Copy(s, nullptr, n);
    } else if (!trim && s->res < n) {
        StdWString_Copy(s, nullptr, n);
    }
    return 1;
}

void __fastcall StdWString_Split(StdWString *s, void *) {
    u16 *p = s->ptr;
    if (!shared(p)) return;
    --refs(p);  // _Tidy(true) on a shared block
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
    u32 n = 0;  // wcslen (0x67acd0)
    while (p[n]) ++n;
    // assign(p, n), inlined
    if (n > kMaxSize) StdStrings_Xlen();
    if (grow_trim(s, n)) {
        for (u32 i = 0; i < n; ++i) s->ptr[i] = p[i];
        s->len = n;
        s->ptr[n] = 0;
    }
}

StdWString *__fastcall StdWString_CtorCopy(StdWString *s, void *, const StdWString *x) {
    *bytes(s) = *reinterpret_cast<const u8 *>(x);
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
    return StdWString_AssignSub(s, nullptr, x, 0, kNpos);
}

StdWString *__fastcall StdWString_CtorFill(StdWString *s, void *, u32 n, u32 ch, const u8 *allocator) {
    *bytes(s) = *allocator;
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
    return StdWString_AssignFill(s, nullptr, n, ch);
}

StdWString *__fastcall StdWString_CtorChars(StdWString *s, void *, const u16 *chars, const u8 *allocator) {
    *bytes(s) = *allocator;
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
    u32 n = 0;  // wcslen (0x67acd0)
    while (chars[n]) ++n;
    // assign(chars, n), inlined
    if (n > kMaxSize) StdStrings_Xlen();
    if (grow_trim(s, n)) {
        for (u32 i = 0; i < n; ++i) s->ptr[i] = chars[i];
        s->len = n;
        s->ptr[n] = 0;
    }
    return s;
}

StdWString *__fastcall StdWStrings_UninitCopy(void *, void *, const StdWString *first, const StdWString *last, StdWString *dest) {
    for (; first != last; ++first, dest = reinterpret_cast<StdWString *>((uintptr_t)dest + sizeof(StdWString)))
        if (dest) StdWString_CtorCopy(dest, nullptr, first);
    return dest;
}

StdWString *__fastcall StdWString_AppendSub(StdWString *s, void *, const StdWString *x, u32 pos, u32 n) {
    if (x->len < pos) StdStrings_Xran();
    const u32 rest = x->len - pos;
    if (rest < n) n = rest;
    if (kNpos - s->len <= n) StdStrings_Xlen();
    if (!n) return s;
    const u32 total = s->len + n;
    if (StdWString_Grow(s, nullptr, total, 0)) {  // (inlined)
        const u16 *src = c_str(x) + pos;
        for (u32 i = 0; i < n; ++i) s->ptr[s->len + i] = src[i];
        s->len = total;
        s->ptr[total] = 0;
    }
    return s;
}

StdWString *__fastcall StdWString_Erase(StdWString *s, void *, u32 pos, u32 n) {
    erase(s, pos, n, true);
    return s;
}

void __cdecl StdWString_Construct(StdWString *p, const StdWString *x) {
    if (p) StdWString_CtorCopy(p, nullptr, x);
}

StdWString *__cdecl StdWStrings_Copy(const StdWString *first, const StdWString *last, StdWString *dest) {
    for (; first != last; ++first, ++dest) StdWString_AssignSub(dest, nullptr, first, 0, kNpos);
    return dest;
}

StdWString *__fastcall StdWStringVector_Erase(u8 *vector, void *, StdWString *first, const StdWString *last) {
    StdWString *const end = *reinterpret_cast<StdWString **>(vector + 8);
    StdWString *dest = StdWStrings_Copy(last, end, first);
    for (StdWString *p = dest; p != end; ++p) tidy(p);
    *reinterpret_cast<StdWString **>(vector + 8) = dest;
    return first;
}

u8 *__fastcall StdWStringVector_Assign(u8 *vector, void *, const u8 *x) {
    if (vector == x) return vector;
    WVector *const v = reinterpret_cast<WVector *>(vector);
    const WVector *const xv = reinterpret_cast<const WVector *>(x);
    const StdWString *from = xv->first;
    if (count(xv->first, xv->last) <= count(v->first, v->last)) {
        StdWString *dest = v->first;
        for (const StdWString *const end = xv->last; from != end; ++from, ++dest) StdWString_AssignSub(dest, nullptr, from, 0, kNpos);
        for (StdWString *p = dest, *const stop = v->last; p != stop; ++p) CountedBuf_Deleter(bytes(p), nullptr, 0);
        v->last = v->first + count(xv->first, xv->last);
    } else if (count(xv->first, xv->last) <= count(v->first, v->end)) {
        const StdWString *const mid = from + count(v->first, v->last);
        StdWString *dest = v->first;
        for (; from != mid; ++from, ++dest) StdWString_AssignSub(dest, nullptr, from, 0, kNpos);
        dest = v->last;
        for (const StdWString *const end = xv->last; from != end; ++from, ++dest) StdWString_Construct(dest, from);
        v->last = v->first + count(xv->first, xv->last);
    } else {
        for (StdWString *p = v->first, *const stop = v->last; p != stop; ++p) tidy(p);
        GameHeap_Free(v->first);
        const s32 n = xv->first ? (s32)((uintptr_t)xv->last - (uintptr_t)xv->first) >> 4 : 0;
        v->first = static_cast<StdWString *>(GameHeap_Alloc((u32)(n < 0 ? 0 : n) << 4));
        v->last = v->end = StdWStrings_UninitCopy(vector, nullptr, xv->first, xv->last, v->first);  // (inlined)
    }
    return vector;
}

void __fastcall StdWStringVector_InsertN(u8 *vector, void *, StdWString *position, u32 n, const StdWString *x) {
    WVector *const v = reinterpret_cast<WVector *>(vector);
    StdWString *const last = v->last;
    if (span(last, v->end) < n) {
        const u32 size = count(v->first, last);
        const u32 grown = (n < size ? size : n) + size;
        StdWString *const block = static_cast<StdWString *>(GameHeap_Alloc((u32)((s32)grown < 0 ? 0 : grown) << 4));
        StdWString *to = block;
        for (const StdWString *from = v->first; from != position; ++from, ++to)
            if (to) StdWString_CtorCopy(to, nullptr, from);
        StdWString *p = to;
        for (u32 k = n; k; --k, ++p)
            if (p) StdWString_CtorCopy(p, nullptr, x);
        to = advance(to, n);
        for (const StdWString *from = position, *const end = v->last; from != end; ++from, ++to)
            if (to) StdWString_CtorCopy(to, nullptr, from);
        for (StdWString *q = v->first, *const stop = v->last; q != stop; ++q) tidy(q);
        GameHeap_Free(v->first);
        v->end = advance(block, grown);
        const u32 old = count(v->first, v->last);  // (the freed block's pointers)
        v->first = block;
        v->last = advance(block, n + old);
        return;
    }
    const u32 after = span(position, last);
    if (after < n) {
        StdWString *to = advance(position, n);
        for (const StdWString *from = position; from != last; ++from, ++to)
            if (to) StdWString_CtorCopy(to, nullptr, from);
        StdWString *p = v->last;
        for (u32 k = n - span(position, v->last); k; --k, ++p)
            if (p) StdWString_CtorCopy(p, nullptr, x);
        for (StdWString *q = position, *const stop = v->last; q != stop; ++q) StdWString_AssignSub(q, nullptr, x, 0, kNpos);
        v->last = advance(v->last, n);
        return;
    }
    if (!n) return;
    StdWString *to = last;
    for (const StdWString *from = advance(last, 0 - n); from != last; ++from, ++to)
        if (to) StdWString_CtorCopy(to, nullptr, from);
    StdWString *e = v->last;
    for (const StdWString *from = advance(e, 0 - n); from != position;) {  // (copy_backward)
        --from;
        --e;
        StdWString_AssignSub(e, nullptr, from, 0, kNpos);
    }
    for (StdWString *q = position, *const stop = advance(position, n); q != stop; ++q) StdWString_AssignSub(q, nullptr, x, 0, kNpos);
    v->last = advance(v->last, n);
}
}

// The second copy (_Nullstr 0x700624, npos 0x700628): its own helpers, as its code inlines them.
namespace second {

namespace {

const u8 kFrozen = 0xff;
const u32 kMinSize = 0x1f;
const u32 kMaxSize = 0x7ffffffd;  // max_size (0x4f6080)
const u16 *const kNullStr = reinterpret_cast<const u16 *>(0x00700624);

void xlen() { reinterpret_cast<void(__cdecl *)()>(0x006918f0)(); }  // std::_Xlen: throws length_error
void xran() { reinterpret_cast<void(__cdecl *)()>(0x00691ba0)(); }  // std::_Xran: throws out_of_range

u8 &refs(const u16 *p) { return *reinterpret_cast<u8 *>((uintptr_t)p - 1); }

bool shared(const u16 *p) { return p && refs(p) != 0 && refs(p) != kFrozen; }

const u16 *c_str(const StdWString *s) { return s->ptr ? s->ptr : kNullStr; }

void empty(StdWString *s) {
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
}

// _Tidy(true) inlined: the block loses a count, or is freed when s owns it alone or it is frozen; s emptied.
void tidy(StdWString *s) {
    if (s->ptr) {
        if (shared(s->ptr))
            --refs(s->ptr);
        else
            GameHeap_Free(reinterpret_cast<u8 *>(s->ptr) - 2);
    }
    empty(s);
}

void reset(StdWString *s, u32 release) { CountedBuf_Reset(reinterpret_cast<u8 *>(s), nullptr, release); }

// _Grow(n, true) as inlined into assign and _Split (its _Tidy calls out of line): whether there are n characters to write.
bool growTrim(StdWString *s, u32 n) {
    if (shared(s->ptr)) {
        if (n == 0) {
            --refs(s->ptr);
            reset(s, 0);
            return false;
        }
    } else if (n == 0) {
        reset(s, 1);
        return false;
    } else if (s->res > kMinSize || s->res < n) {
        reset(s, 1);
    } else {
        return true;
    }
    WStdString_Copy(s, nullptr, n);
    return true;
}

void copyChars(u16 *to, const u16 *from, u32 n) {
    for (u32 k = 0; k < n; ++k)
        to[k] = from[k];
}

}  // namespace

extern "C" {

StdWString *__fastcall WStdString_AssignSub(StdWString *s, void *, const StdWString *x, u32 pos, u32 n) {
    if (x->len < pos)
        xran();
    const u32 rest = x->len - pos;
    if (rest < n)
        n = rest;
    if (s == x) {
        // erase(pos + n, npos), then erase(0, pos)
        u32 at = pos + n, m = *reinterpret_cast<const u32 *>(0x00700628);
        if (s->len < at)
            xran();
        WStdString_Split(s, nullptr);
        if (s->len - at < m)
            m = s->len - at;
        if (m) {
            WTraits_Move(s->ptr + at, s->ptr + at + m, s->len - m - at);
            const u32 k = s->len - m;
            if (WStdString_Grow(s, nullptr, k, 0)) {
                s->len = k;
                s->ptr[k] = 0;
            }
        }
        m = pos;
        WStdString_Split(s, nullptr);
        if (s->len < m)
            m = s->len;
        if (m) {
            WTraits_Move(s->ptr, s->ptr + m, s->len - m);
            const u32 k = s->len - m;
            if (WStdString_Grow(s, nullptr, k, 0))
                ImeWString_SetLength(reinterpret_cast<u8 *>(s), nullptr, k);
        }
        return s;
    }
    if (n != 0 && n == x->len) {
        const u16 *from = c_str(x);
        if (refs(from) < kFrozen - 1) {
            tidy(s);
            s->ptr = const_cast<u16 *>(c_str(x));
            s->len = x->len;
            s->res = x->res;
            ++refs(s->ptr);
            return s;
        }
    }
    if (kMaxSize < n)
        xlen();
    if (growTrim(s, n)) {
        copyChars(s->ptr, c_str(x) + pos, n);
        s->len = n;
        s->ptr[n] = 0;
    }
    return s;
}

u8 __fastcall WStdString_Grow(StdWString *s, void *, u32 n, u8 trim) {
    if (n > kMaxSize)
        xlen();
    u16 *const p = s->ptr;
    if (shared(p)) {
        if (n != 0) {
            WStdString_Copy(s, nullptr, n);
            return 1;
        }
        --refs(p);
        empty(s);
        return 0;
    }
    if (n == 0) {
        if (trim) {
            tidy(s);
        } else if (p) {
            s->len = 0;
            *p = 0;
        }
        return 0;
    }
    if (trim) {
        if (s->res > kMinSize || s->res < n) {
            tidy(s);
            WStdString_Copy(s, nullptr, n);
        }
    } else if (s->res < n) {
        WStdString_Copy(s, nullptr, n);
    }
    return 1;
}

void __fastcall WStdString_Copy(StdWString *s, void *, u32 n) {
    u32 res = n | kMinSize;
    if (res > kMaxSize)
        res = n;
    s32 count = (s32)(res + 2);  // allocator::allocate: a negative count asks for 0
    if (count < 0)
        count = 0;
    u16 *const block = static_cast<u16 *>(GameHeap_Alloc((u32)count * 2));
    if (s->len)
        copyChars(block + 1, s->ptr, s->len);  // sic: not clamped to res
    const u32 len = s->len;
    if (s->ptr) {
        if (shared(s->ptr))
            --refs(s->ptr);
        else
            GameHeap_Free(reinterpret_cast<u8 *>(s->ptr) - 2);
    }
    s->ptr = block + 1;
    refs(s->ptr) = 0;
    s->len = len;
    s->res = res;
    s->ptr[len] = 0;
}

void __fastcall WStdString_Split(StdWString *s, void *) {
    u16 *const p = s->ptr;
    if (!shared(p))
        return;
    --refs(p);
    empty(s);
    u32 n = 0;  // wcslen (0x67acd0)
    while (p[n])
        ++n;
    if (n > kMaxSize)
        xlen();
    if (growTrim(s, n)) {
        copyChars(s->ptr, p, n);
        s->len = n;
        s->ptr[n] = 0;
    }
}
}

}  // namespace second
