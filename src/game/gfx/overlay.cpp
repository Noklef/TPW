#include "overlay.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

const u8 *__fastcall Overlay_ShowFrame(u8 *overlay, void *, u32 flags, const u8 *desc) {
    const u8 *holder = at<const u8 *>(overlay, 4);
    if (!(flags & 1) && !(flags & 0x80000)) return at<u8 *>(overlay, 0x7c);
    if (flags & 0x80000) at<u16>(overlay, 0xa) = at<u16>(desc, 0xa);
    if (flags & 1) at<u16>(overlay, 0xc) = at<u16>(desc, 4);
    u16 n = (u16)(at<u16>(overlay, 0xc) + at<u16>(overlay, 0xa));
    u8 *node = at<u8 *>(at<const u8 *>(holder, 4), 0x78);
    for (; n != 0 && node; --n) node = at<u8 *>(node, 0xc);
    if (u8 *old = at<u8 *>(overlay, 0x7c)) at<u32>(old, 0) |= 0x8010;
    at<u8 *>(overlay, 0x7c) = node;
    if (node) at<u32>(node, 0) &= ~0x8010u;
    return node;
}
}
