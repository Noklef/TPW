// The game's std::wstring: see wstdstring.h.
#include "wstdstring.h"

#include <cstdint>

#include "buffers.h"
#include "gameheap.h"
#include "wtraits.h"

namespace {

const u8 kFrozen = 0xff;
const u32 kMinSize = 0x1f;
const u32 kMaxSize = 0x7ffffffd;  // max_size (0x4f6080)
const u16 *const kNullStr = reinterpret_cast<const u16 *>(0x00700624);

void xlen() { reinterpret_cast<void(__cdecl *)()>(0x006918f0)(); }  // std::_Xlen: throws length_error
void xran() { reinterpret_cast<void(__cdecl *)()>(0x00691ba0)(); }  // std::_Xran: throws out_of_range

u8 &refs(const u16 *p) { return *reinterpret_cast<u8 *>((uintptr_t)p - 1); }

bool shared(const u16 *p) { return p && refs(p) != 0 && refs(p) != kFrozen; }

const u16 *c_str(const WStdString *s) { return s->ptr ? s->ptr : kNullStr; }

void empty(WStdString *s) {
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
}

// _Tidy(true) inlined: the block loses a count, or is freed when s owns it alone or it is frozen; s emptied.
void tidy(WStdString *s) {
    if (s->ptr) {
        if (shared(s->ptr))
            --refs(s->ptr);
        else
            GameHeap_Free(reinterpret_cast<u8 *>(s->ptr) - 2);
    }
    empty(s);
}

void reset(WStdString *s, u32 release) { CountedBuf_Reset(reinterpret_cast<u8 *>(s), nullptr, release); }

// _Grow(n, true) as inlined into assign and _Split (its _Tidy calls out of line): whether there are n characters to write.
bool growTrim(WStdString *s, u32 n) {
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

WStdString *__fastcall WStdString_AssignSub(WStdString *s, void *, const WStdString *x, u32 pos, u32 n) {
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

u8 __fastcall WStdString_Grow(WStdString *s, void *, u32 n, u8 trim) {
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

void __fastcall WStdString_Copy(WStdString *s, void *, u32 n) {
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

void __fastcall WStdString_Split(WStdString *s, void *) {
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
