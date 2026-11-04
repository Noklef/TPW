#include "wcollate.h"

#include <cstdint>
#include <cstring>

#include "buffers.h"
#include "iosbase.h"
#include "stdstring.h"
#include "wtraits.h"

namespace {

u32 npos() { return *reinterpret_cast<const u32 *>((uintptr_t)0x006fef00); }

}  // namespace

extern "C" {

StdWString *__fastcall CollateW_DoTransform(void *, void *, StdWString *out, const u16 *begin, const u16 *end) {
    StdWString s;
    s.allocator = 0;
    CountedBuf_Reset(reinterpret_cast<u8 *>(&s), nullptr, 0);
    const u32 count = static_cast<u32>(static_cast<s32>(reinterpret_cast<const u8 *>(end) - reinterpret_cast<const u8 *>(begin)) >> 1);
    u32 need = count;
    for (;;) {
        if (npos() - s.len <= need) StdStrings_Xlen();
        if (need) {
            const u32 grown = s.len + need;
            if (StdWString_Grow(&s, nullptr, grown, 0)) {
                WTraits_Assign(s.ptr + s.len, 0, need);
                s.len = grown;
                s.ptr[grown] = 0;
            }
        }
        StdWString_Freeze(&s, nullptr);
        const u16 *const stop = s.ptr ? s.ptr + s.len : nullptr;
        StdWString_Freeze(&s, nullptr);
        const u32 room = static_cast<u32>(static_cast<s32>(reinterpret_cast<const u8 *>(stop) - reinterpret_cast<const u8 *>(s.ptr)) >> 1);
        if (count <= room) std::memcpy(s.ptr, begin, count * 2);
        if (s.len >= count) break;
        need = count - s.len;
    }
    if (s.len < count) StdStrings_Xran();
    StdWString_Split(&s, nullptr);
    u32 extra = s.len - count;
    if (npos() < extra) extra = npos();
    if (extra) {
        WTraits_Move(s.ptr + count, s.ptr + count + extra, s.len - count - extra);
        const u32 kept = s.len - extra;
        if (StdWString_Grow(&s, nullptr, kept, 0)) ImeWString_SetLength(reinterpret_cast<u8 *>(&s), nullptr, kept);
    }
    *reinterpret_cast<u8 *>(&out->allocator) = 0;
    out->ptr = nullptr;
    out->len = 0;
    out->res = 0;
    StdWString_AssignSub(out, nullptr, &s, 0, npos());
    if (s.ptr) {
        u8 &owners = reinterpret_cast<u8 *>(s.ptr)[-1];
        if (owners && owners != 0xff) {
            --owners;
        } else {
            IosBase_Env().memFree(s.ptr - 1);
        }
    }
    return out;
}

}  // extern "C"
