#include "soundnames.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 *__fastcall SoundNames_Save(u8 *self, void *, u32 *out, u8 *stream) {
    const u32 start = vcall<u32>(stream, 0x18);
    u32 count = at<u32>(self, 0x64);   // (the original keeps it in its stream argument's slot)
    vcall<u32>(stream, 0xc, &count, 4u);
    vcall<u32>(stream, 0xc, &count, 4u);   // (twice, sic)
    u32 word = 0;   // (one local: the zero dwords, then each element's length byte in its low byte)
    for (u32 i = 0; i < count; ++i) {
        word = 0;
        vcall<u32>(stream, 0xc, &word, 4u);
    }
    u8 **cur = at<u8 **>(self, 0x44);
    u8 **blockEnd = at<u8 **>(self, 0x40);
    u8 ***node = at<u8 ***>(self, 0x48);
    while (cur != at<u8 **>(self, 0x54)) {   // (the end iterator read again each time)
        u8 *const element = *cur;
        word = (word & ~0xffu) | (element ? (u8)(at<u8>(element, 0x10) + 1) : 0u);
        vcall<u32>(stream, 0xc, &word, 1u);
        if ((u8)word) vcall<u32>(stream, 0xc, at<u8 *>(element, 0x14), (u32)(u8)word);
        if (++cur == blockEnd) {
            cur = node[1];
            ++node;
            blockEnd = cur + 0x400;
        }
    }
    const u32 end = vcall<u32>(stream, 0x18);
    vcall<u32>(stream, 0x10, start, 0u);
    u32 size = end - start - 8;
    vcall<u32>(stream, 0xc, &size, 4u);
    vcall<u32>(stream, 0x10, end, 0u);
    *out = 0;
    return out;
}

}  // extern "C"
