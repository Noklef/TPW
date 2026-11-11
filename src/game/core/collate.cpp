#include "collate.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "iosbase.h"

namespace {

u32 npos() { return *reinterpret_cast<const u32 *>((uintptr_t)0x006fe5ac); }

}  // namespace

extern "C" {

StdString *__fastcall Collate_DoTransform(void *, void *, StdString *out, const char *first, const char *last) {
    StdString s;
    s.allocator = static_cast<u8>(reinterpret_cast<uintptr_t>(first));
    StdString_Tidy(&s, nullptr, 0);
    const u32 count = static_cast<u32>(last - first);
    u32 need = count;
    for (;;) {
        if (npos() - s.len <= need) Std_Xlen();
        if (need) {
            const u32 grown = s.len + need;
            if (StdString_Grow(&s, nullptr, grown, 0)) {
                std::memset(s.ptr + s.len, 0, need);
                s.len = grown;
                s.ptr[grown] = 0;
            }
        }
        StdString_Freeze(&s, nullptr);
        const char *const stop = s.ptr ? s.ptr + s.len : nullptr;
        StdString_Freeze(&s, nullptr);
        if (count <= static_cast<u32>(stop - s.ptr)) std::memcpy(s.ptr, first, count);
        if (s.len >= count) break;
        need = count - s.len;
    }
    if (s.len < count) Std_Xran();
    StdString_Split(&s, nullptr);
    u32 extra = s.len - count;
    if (npos() < extra) extra = npos();
    if (extra) {
        CRT_memmove(s.ptr + count, s.ptr + count + extra, s.len - count - extra);
        const u32 kept = s.len - extra;
        if (StdString_Grow(&s, nullptr, kept, 0)) StdString_Eos(&s, nullptr, kept);
    }
    *reinterpret_cast<u8 *>(&out->allocator) = static_cast<u8>(s.allocator);
    out->ptr = nullptr;
    out->len = 0;
    out->res = 0;
    StdString_AssignSub(out, nullptr, &s, 0, npos());
    StdString_Tidy(&s, nullptr, 1);
    return out;
}

}  // extern "C"
