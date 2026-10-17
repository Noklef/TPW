#include "liveobjects.h"

#include <cstdint>
#include <initializer_list>

#include "gameheap.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void **list() { return reinterpret_cast<void **>((uintptr_t)0x0087a6d8); }
s32 &count() { return global<s32>(0x0087aed8); }

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void Identity(u8 *m) {
    for (u32 i = 0; i < 0x40; i += 4) at<u32>(m, i) = 0;
    for (u32 i = 0; i < 0x40; i += 0x14) at<u32>(m, 0x3c - i) = 0x3f800000;
}

}  // namespace

extern "C" {

u32 __cdecl LiveObjects_Add(void *object) {
    s32 n = count();
    if (n < 0x200) {
        list()[n] = object;
        count() = n + 1;
    }
    return 1;
}

u32 __cdecl LiveObjects_Remove(void *object) {
    s32 last = count() - 1;
    s32 i = last;
    while (i >= 0 && list()[i] != object)
        --i;
    if (i < 0)
        return 0;
    count() = last;
    list()[i] = list()[last];
    return 1;
}

u8 *__fastcall LiveObject7019f8_Construct(u8 *o, void *, u32 most_derived) {
    if (most_derived) {
        at<u32>(o, 0) = 0x00701a08;
        at<u32>(o, 0x154) = 0x006fdce8;
    }
    Identity(o + 0x10);
    Identity(o + 0x50);
    at<u32>(o, at<u32>(at<u8 *>(o, 0), 4)) = 0x007019f8;
    const u32 offset = at<u32>(at<u8 *>(o, 0), 4);
    at<u32>(o, offset - 4) = offset - 0x154;
    at<s32>(o, 0x90) = -1;
    at<s32>(o, 0x94) = -1;
    for (u32 k : {0x8u, 0x4u, 0x11cu, 0x140u, 0x128u, 0x124u, 0x12cu, 0x130u, 0x134u, 0x138u, 0x13cu}) at<u32>(o, k) = 0;
    for (u32 i = 0; i < 0x21; ++i) at<u32>(o, 0x98 + 4 * i) = 0;
    LiveObjects_Add(o ? o + at<u32>(at<u8 *>(o, 0), 4) : nullptr);
    return o;
}

u8 *__fastcall LiveObject701b30_DeletingDtor(u8 *base, void *, u32 flags) {
    u8 *o = base - 0xa4;
    at<u32>(o, at<u32>(at<u8 *>(o, 0), 4)) = 0x00701b30;
    const u32 offset = at<u32>(at<u8 *>(o, 0), 4);
    at<u32>(o, offset - 4) = offset - 0xa4;
    LiveObjects_Remove(o ? o + at<u32>(at<u8 *>(o, 0), 4) : nullptr);
    at<u32>(o, 0xa4) = 0x006fdce8;
    if (flags & 1)
        GameHeap_Free(o);
    return o;
}
}
