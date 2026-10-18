#include "stdstring.h"

#include <cstdint>
#include <cstring>

#include "movs.h"

namespace {

const u8 kFrozen = 0xff;
const u32 kMinSize = 0x1f;
const u32 kMaxSize = 0xfffffffd;  // the allocator's 0xffffffff less 2
const char *const kNullStr = reinterpret_cast<const char *>(0x006fe5b0);

const StdStringEnv kGameEnv = {
    reinterpret_cast<void *(__cdecl *)(u32)>(0x0067acc0),
    reinterpret_cast<void(__cdecl *)(void *)>(0x0067acb0),
    reinterpret_cast<void(__cdecl *)()>(0x006918f0),
    reinterpret_cast<void(__cdecl *)()>(0x00691ba0),
};
const StdStringEnv *g_env = &kGameEnv;

u8 &refs(const char *p) { return *reinterpret_cast<u8 *>((uintptr_t)p - 1); }

bool shared(const char *p) { return p && refs(p) != 0 && refs(p) != kFrozen; }

const char *c_str(const StdString *s) { return s->ptr ? s->ptr : kNullStr; }

void empty(StdString *s) {
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
}

// erase(p, m) as inlined into assign: shifts the tail down over [p, p + m).
void erase(StdString *s, u32 p, u32 m) {
    if (s->len < p) g_env->xran();
    StdString_Split(s, nullptr);
    const u32 left = s->len - p;
    if (left < m) m = left;
    if (m) {
        std::memmove(s->ptr + p, s->ptr + p + m, left - m);  // memmove (0x67bb90)
        const u32 n = s->len - m;
        if (StdString_Grow(s, nullptr, n, 0)) StdString_Eos(s, nullptr, n);
    }
}

}  // namespace

void StdStrings_SetEnv(const StdStringEnv *e) { g_env = e ? e : &kGameEnv; }

void StdStrings_Xlen() { g_env->xlen(); }

void StdStrings_Xran() { g_env->xran(); }

extern "C" {

void __fastcall StdString_Dtor(StdString *s, void *) { StdString_Tidy(s, nullptr, 1); }

StdString *__fastcall StdString_AssignSub(StdString *s, void *, const StdString *x, u32 pos, u32 n) {
    if (x->len < pos) g_env->xran();
    const u32 rest = x->len - pos;
    if (rest < n) n = rest;
    if (s == x) {
        erase(s, pos + n, 0xffffffff);  // npos (0x6fe5ac)
        erase(s, 0, pos);
    } else if (n && n == x->len && refs(c_str(x)) < kFrozen - 1) {
        StdString_Tidy(s, nullptr, 1);
        s->ptr = const_cast<char *>(c_str(x));
        s->len = x->len;
        s->res = x->res;
        ++refs(s->ptr);
    } else {
        if (StdString_MaxSize(s, nullptr) < n) g_env->xlen();
        if (StdString_Grow(s, nullptr, n, 1)) {
            Rep_Movs(s->ptr, c_str(x) + pos, n);
            StdString_Eos(s, nullptr, n);
        }
    }
    return s;
}

void __fastcall StdString_Tidy(StdString *s, void *, u8 built) {
    if (built && s->ptr) {
        if (refs(s->ptr) == 0 || refs(s->ptr) == kFrozen)
            g_env->free(s->ptr - 1);
        else
            --refs(s->ptr);
    }
    empty(s);
}

StdString *__fastcall StdString_AssignChars(StdString *s, void *, const char *chars, u32 n) {
    if (n > kMaxSize) g_env->xlen();
    if (StdString_Grow(s, nullptr, n, 1)) {
        Rep_Movs(s->ptr, chars, n);
        StdString_Eos(s, nullptr, n);
    }
    return s;
}

void __fastcall StdString_Eos(StdString *s, void *, u32 n) {
    s->len = n;
    s->ptr[n] = 0;
}

u8 __fastcall StdString_Grow(StdString *s, void *, u32 n, u8 trim) {
    if (n > kMaxSize) g_env->xlen();
    if (shared(s->ptr)) {
        if (!n) {
            --refs(s->ptr);
            empty(s);
            return 0;
        }
        StdString_Copy(s, nullptr, n);
        return 1;
    }
    if (!n) {
        if (trim)
            StdString_Tidy(s, nullptr, 1);
        else if (s->ptr)
            StdString_Eos(s, nullptr, 0);
        return 0;
    }
    if (trim && (kMinSize < s->res || s->res < n)) {
        StdString_Tidy(s, nullptr, 1);
        StdString_Copy(s, nullptr, n);
    } else if (!trim && s->res < n) {
        StdString_Copy(s, nullptr, n);
    }
    return 1;
}

u32 __fastcall StdString_MaxSize(const StdString *, void *) { return kMaxSize; }

void __fastcall StdString_Copy(StdString *s, void *, u32 n) {
    u32 res = n | kMinSize;
    if (res > kMaxSize) res = n;
    const u32 size = res + 2;  // allocator::allocate: a negative count asks for 0
    char *block = static_cast<char *>(g_env->alloc((s32)size < 0 ? 0 : size));
    if (s->len) Rep_Movs(block + 1, s->ptr, s->len);  // sic: not clamped to res
    const u32 len = s->len;
    StdString_Tidy(s, nullptr, 1);
    s->ptr = block + 1;
    refs(s->ptr) = 0;
    s->res = res;
    StdString_Eos(s, nullptr, len);
}

void __fastcall StdString_Split(StdString *s, void *) {
    char *p = s->ptr;
    if (!shared(p)) return;
    --refs(p);  // _Tidy(true) on a shared block
    empty(s);
    StdString_AssignChars(s, nullptr, p, (u32)std::strlen(p));
}
}
