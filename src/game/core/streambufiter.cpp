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
}
