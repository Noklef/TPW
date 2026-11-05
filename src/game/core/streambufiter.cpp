#include "streambufiter.h"

#include <cstdint>

namespace {

template <typename Unit> struct Iter {
    u8 *buffer;
    u8 got;
    Unit value;
};

template <typename Unit> Unit *&next_of(u8 *buffer) { return **reinterpret_cast<Unit ***>(buffer + 0x1c); }

u32 &left_of(u8 *buffer) { return **reinterpret_cast<u32 **>(buffer + 0x2c); }

template <typename Unit> bool waiting(u8 *buffer) {
    const u32 next = static_cast<u32>(reinterpret_cast<uintptr_t>(next_of<Unit>(buffer)));
    return next && next < next + left_of(buffer) * static_cast<u32>(sizeof(Unit));
}

u32 virtual_call(u8 *buffer, u32 offset) {
    const u32 slot = *reinterpret_cast<u32 *>(*reinterpret_cast<uintptr_t *>(buffer) + offset);
    return reinterpret_cast<u32(__fastcall *)(u8 *, void *)>((uintptr_t)slot)(buffer, nullptr);
}

// A character put at the streambuf's writing pointer: its answer as the character, or overflow's.
template <typename Unit> u32 put(u8 *buffer, u32 ch) {
    Unit **const next = *reinterpret_cast<Unit ***>(buffer + 0x20);
    const u32 at = static_cast<u32>(reinterpret_cast<uintptr_t>(*next));
    u32 *const left = *reinterpret_cast<u32 **>(buffer + 0x30);
    if (at && at < at + *left * static_cast<u32>(sizeof(Unit))) {
        --*left;
        Unit **const again = *reinterpret_cast<Unit ***>(buffer + 0x20);
        Unit *const to = *again;
        *again = to + 1;
        *to = static_cast<Unit>(ch);
        return ch;
    }
    const u32 slot = *reinterpret_cast<u32 *>(*reinterpret_cast<uintptr_t *>(buffer) + 4);
    return reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32)>((uintptr_t)slot)(buffer, nullptr, ch);
}

template <typename Unit> bool is_end(u32 c) { return sizeof(Unit) == 1 ? c == 0xffffffffu : static_cast<u16>(c) == 0xffff; }

template <typename Unit> void fetch(Iter<Unit> *it) {
    if (it->got) return;
    if (it->buffer) {
        const u32 c = waiting<Unit>(it->buffer) ? *next_of<Unit>(it->buffer) : virtual_call(it->buffer, 0x10);
        if (!is_end<Unit>(c)) {
            it->value = static_cast<Unit>(c);
            it->got = 1;
            return;
        }
    }
    it->buffer = nullptr;
    it->got = 1;
}

template <typename Unit> u32 equal(Iter<Unit> *it, Iter<Unit> *x) {
    fetch(it);
    fetch(x);
    return !it->buffer == !x->buffer ? 1 : 0;
}

template <typename Unit> void inc(Iter<Unit> *it) {
    u8 *const buffer = it->buffer;
    if (buffer) {
        u32 c;
        if (waiting<Unit>(buffer)) {
            --left_of(buffer);
            Unit *&next = next_of<Unit>(buffer);
            const Unit *const at = next;
            next = next + 1;
            c = *at;
        } else {
            c = virtual_call(buffer, 0x14);
        }
        if (!is_end<Unit>(c)) {
            it->got = 0;
            return;
        }
    }
    it->buffer = nullptr;
    it->got = 1;
}

template <typename Unit> u32 *rep(u32 *out, u32 failed, u8 *buffer, u32 ch, u32 n) {
    for (; n; --n) {
        if (!buffer || is_end<Unit>(put<Unit>(buffer, ch))) failed = (failed & 0xffffff00u) | 1;
    }
    out[0] = failed;
    out[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(buffer));
    return out;
}

Iter<u8> *narrow(StreambufIter *it) { return reinterpret_cast<Iter<u8> *>(it); }
Iter<u16> *wide(WStreambufIter *it) { return reinterpret_cast<Iter<u16> *>(it); }

}  // namespace

extern "C" {

u8 *__fastcall StreambufIter_Deref(StreambufIter *it, void *) {
    fetch(narrow(it));
    return &narrow(it)->value;
}

u16 *__fastcall WStreambufIter_Deref(WStreambufIter *it, void *) {
    fetch(wide(it));
    return &wide(it)->value;
}

u32 __fastcall StreambufIter_Equal(StreambufIter *it, void *, StreambufIter *x) { return equal(narrow(it), narrow(x)); }

u32 __fastcall WStreambufIter_Equal(WStreambufIter *it, void *, WStreambufIter *x) { return equal(wide(it), wide(x)); }

void __fastcall StreambufIter_Inc(StreambufIter *it, void *) { inc(narrow(it)); }

void __fastcall WStreambufIter_Inc(WStreambufIter *it, void *) { inc(wide(it)); }

u8 __fastcall StreambufIter_Peek(StreambufIter *self, void *) {
    Iter<u8> *const it = narrow(self);
    if (it->buffer) {
        const u32 c = waiting<u8>(it->buffer) ? *next_of<u8>(it->buffer) : virtual_call(it->buffer, 0x10);
        if (!is_end<u8>(c)) {
            it->value = static_cast<u8>(c);
            it->got = 1;
            return static_cast<u8>(c);
        }
    }
    const u8 old = it->value;
    it->buffer = nullptr;
    it->got = 1;
    return old;
}

u32 __fastcall Streambuf_Sbumpc(u8 *buffer, void *) {
    if (!waiting<u8>(buffer)) return virtual_call(buffer, 0x14);
    --left_of(buffer);
    u8 *&next = next_of<u8>(buffer);
    const u8 *const at = next;
    next = next + 1;
    return *at;
}

u32 __fastcall WStreambuf_Sputc(u8 *buffer, void *, u32 ch) { return put<u16>(buffer, ch); }

u8 *__fastcall WOstreambufIter_Assign(u8 *it, void *, u32 ch) {
    u8 *const buffer = *reinterpret_cast<u8 **>(it + 4);
    if (!buffer || is_end<u16>(put<u16>(buffer, ch))) it[0] = 1;
    return it;
}

u32 *__cdecl NumPut_Rep(u32 *out, u32 failed, u8 *buffer, u32 ch, u32 n) { return rep<u8>(out, failed, buffer, ch & 0xff, n); }

u32 *__cdecl WNumPut_Rep(u32 *out, u32 failed, u8 *buffer, u32 ch, u32 n) { return rep<u16>(out, failed, buffer, ch, n); }
}
