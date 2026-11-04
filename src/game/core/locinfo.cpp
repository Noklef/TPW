#include "locinfo.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "crtlocale.h"
#include "stdstring.h"

namespace {

u32 &word(u32 address) { return *reinterpret_cast<u32 *>((uintptr_t)address); }

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

const u32 lock_state = 0x00fb1968;
const u32 lock_section = 0x00fb1950;

// A string of the structure, empty with its allocator's byte.
void set_empty(StdString *s, u8 allocator) {
    *reinterpret_cast<u8 *>(&s->allocator) = allocator;
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
}

// The text in s, when there is room for it.
void assign(StdString *s, const char *text) {
    const u32 length = static_cast<u32>(std::strlen(text));
    if (!StdString_Grow(s, nullptr, length, 1)) return;
    std::memcpy(s->ptr, text, length);
    s->len = length;
    s->ptr[length] = 0;
}

// The string's block: a count of the other owners less one, or freed by the last (0xff: frozen, freed too); the string emptied.
void release(StdString *s) {
    if (s->ptr) {
        u8 &owners = reinterpret_cast<u8 *>(s->ptr)[-1];
        if (owners && owners != 0xff) {
            --owners;
        } else {
            CrtInner().memFree(&owners);
        }
    }
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
}

}  // namespace

void *__fastcall Lockit_Ctor(void *self) {
    u32 state = word(lock_state);
    if (state != 2) {
        if (!state) {
            const u32 was = import_slot<u32(__stdcall *)(u32, u32)>(0x006fd200)(lock_state, 1);
            if (!was) {
                import_slot<void(__stdcall *)(u32)>(0x006fd144)(lock_section);
                CRT_atexit(reinterpret_cast<void *>((uintptr_t)0x00691e70));
                state = 2;
                word(lock_state) = state;
            } else if (was == 2) {
                state = 2;
                word(lock_state) = state;
            } else {
                state = word(lock_state);
            }
        }
        while (state == 1) {
            import_slot<void(__stdcall *)(u32)>(0x006fd1d4)(1);
            state = word(lock_state);
        }
        if (state != 2) return self;
    }
    import_slot<void(__stdcall *)(u32)>(0x006fd140)(lock_section);
    return self;
}

void __fastcall Lockit_Dtor(void *) {
    if (word(lock_state) == 2) import_slot<void(__stdcall *)(u32)>(0x006fd130)(lock_section);
}

void *__fastcall Locinfo_Ctor(void *self, void *, const char *name) {
    StdString *const strings = static_cast<StdString *>(self);
    const u8 low = static_cast<u8>(reinterpret_cast<uintptr_t>(name));
    set_empty(&strings[0], low);
    set_empty(&strings[1], low);
    *reinterpret_cast<u8 *>(&strings[2].allocator) = low;
    StdString_Tidy(&strings[2], nullptr, 0);
    set_empty(&strings[3], low);
    assign(&strings[2], CRT_setlocale(0, nullptr));
    const char *now = name ? CRT_setlocale(0, name) : nullptr;
    if (!now) now = "*";
    assign(&strings[3], now);
    return self;
}

void __fastcall Locinfo_Dtor(void *self) {
    StdString *const strings = static_cast<StdString *>(self);
    if (strings[2].len) {
        CRT_setlocale(0, strings[2].ptr ? strings[2].ptr : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0));
    }
    release(&strings[3]);
    release(&strings[2]);
    release(&strings[1]);
    release(&strings[0]);
}
