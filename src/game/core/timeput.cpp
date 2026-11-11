#include "timeput.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "crttime.h"
#include "iosbase.h"
#include "stdstring.h"
#include "streambufiter.h"

namespace {

u32 npos() { return *reinterpret_cast<const u32 *>((uintptr_t)0x006fe5ac); }

// The string's append(need, 0), as the original inlines it.
void append_zeros(StdString &s, u32 need) {
    if (npos() - s.len <= need) Std_Xlen();
    if (need && StdString_Grow(&s, nullptr, s.len + need, 0)) {
        std::memset(s.ptr + s.len, 0, need);
        s.len += need;
        s.ptr[s.len] = 0;
    }
}

template <bool Wide> u32 *put_time(u8 *self, u32 *out, u32 failed, u8 *buffer, const s32 *tm, char format, char modifier) {
    char spec[5] = {'!', '%', 'x', 0, 0};   // (the '!' makes every answer at least one character: 0 only means it did not fit)
    StdString s;
    s.allocator = static_cast<u8>(format);   // (the original's stack slot; never read)
    StdString_Tidy(&s, nullptr, 0);
    if (modifier == 0) {
        spec[2] = format;
    } else {
        spec[2] = modifier;
        spec[3] = format;
    }
    u32 count;
    for (u32 need = 0x10;; need *= 2) {
        append_zeros(s, need);
        char *const *const names = *reinterpret_cast<char *const *const *>(self + 8);
        const u32 size = s.len;
        StdString_Freeze(&s, nullptr);
        count = CRT_Strftime(s.ptr, size, spec, tm, names);
        if (count) break;
    }
    StdString_Freeze(&s, nullptr);
    u32 to[2];
    for (const char *p = s.ptr + 1; --count; ++p) {
        if (Wide)
            failed = WNumPut_Rep(to, failed, buffer, CRT_btowc(static_cast<u32>(*p)), 1)[0];
        else
            failed = NumPut_Rep(to, failed, buffer, static_cast<u8>(*p), 1)[0];
    }
    out[0] = failed;
    out[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(buffer));
    StdString_Tidy(&s, nullptr, 1);
    return out;
}

}  // namespace

extern "C" {

u32 *__fastcall TimePut_DoPut(u8 *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *, const s32 *tm, u32 format, u32 modifier) {
    return put_time<false>(self, out, failed, buffer, tm, static_cast<char>(format), static_cast<char>(modifier));
}

u32 *__fastcall WTimePut_DoPut(u8 *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *, const s32 *tm, u32 format, u32 modifier) {
    return put_time<true>(self, out, failed, buffer, tm, static_cast<char>(format), static_cast<char>(modifier));
}

}  // extern "C"
