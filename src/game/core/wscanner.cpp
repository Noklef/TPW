#include "wscanner.h"

#include <cstdint>

#include "buffers.h"
#include "gameheap.h"
#include "wtraits.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u16 *const kNullStr = reinterpret_cast<u16 *>(0x006fd6c4);  // _Nullstr

u8 &refs(const u16 *p) { return *reinterpret_cast<u8 *>((uintptr_t)p - 1); }

// std::wstring::assign(text, 0, npos) inlined (text a by-value argument): the text's block shared when it is whole, not empty and its
// count byte is below 0xfe; else _Grow(n, true), the characters copied and the end set.
void assign(StdWString *s, StdWString &text) {
    const u32 npos = *reinterpret_cast<const u32 *>(0x006fef00);
    const u32 n = npos >= text.len ? text.len : npos;
    if (s == &text) {
        StdWString_Erase(s, nullptr, n, npos);
        StdWString_Erase(s, nullptr, 0, 0);
    } else if (n != 0 && n == text.len && refs(text.ptr ? text.ptr : kNullStr) < 0xfe) {
        CountedBuf_Reset(reinterpret_cast<u8 *>(s), nullptr, 1);
        u16 *const ptr = text.ptr ? text.ptr : kNullStr;
        s->ptr = ptr;
        s->len = text.len;
        s->res = text.res;
        ++refs(ptr);
    } else if (StdWString_Grow(s, nullptr, n, 1)) {
        WTraits_Copy(s->ptr, text.ptr ? text.ptr : kNullStr, n);
        ImeWString_SetLength(reinterpret_cast<u8 *>(s), nullptr, n);
    }
}

// The by-value text destroyed: its count byte down by one, or (0 or 0xff) its block freed.
void destroy(StdWString &text) {
    if (text.ptr) {
        const u8 count = refs(text.ptr);
        if (count != 0 && count != 0xff)
            refs(text.ptr) = count - 1;
        else
            GameHeap_Free(reinterpret_cast<u8 *>(text.ptr) - 2);
    }
}

}  // namespace

extern "C" {

void __fastcall WScanner_SetText(u8 *scanner, void *, StdWString text, u32 context, const u8 *flags) {
    at<u16>(scanner, 0x3d) = at<u16>(flags, 0);
    at<u32>(scanner, 0x40) = context;
    at<u8>(scanner, 0x3f) = flags[2];
    StdWString *const s = reinterpret_cast<StdWString *>(scanner + 4);
    assign(s, text);
    if (s->ptr && refs(s->ptr) != 0 && refs(s->ptr) != 0xff)
        StdWString_Grow(s, nullptr, s->len, 0);
    if (s->ptr)
        refs(s->ptr) = 0xff;
    u16 *const chars = s->ptr;
    at<u32>(scanner, 0x38) = 0;
    at<u16 *>(scanner, 0x34) = chars;
    destroy(text);
}

u8 *__fastcall WKeywordPair_Ctor(u8 *pair, void *, StdWString key, u32 v0, u32 v1, u32 v2, u32 v3) {
    StdWString *const s = reinterpret_cast<StdWString *>(pair);
    at<u8>(pair, 0) = static_cast<u8>(key.allocator);
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
    assign(s, key);
    at<u32>(pair, 0x10) = v0;
    at<u32>(pair, 0x14) = v1;
    at<u32>(pair, 0x18) = v2;
    at<u32>(pair, 0x1c) = v3;
    destroy(key);
    return pair;
}

}  // extern "C"
