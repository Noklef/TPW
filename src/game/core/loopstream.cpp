#include "loopstream.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *inner(u8 *stream) { return vcall<u8 *>(stream, 0x48); }

}  // namespace

extern "C" {

u32 __fastcall LoopStream_Read(u8 *stream, void *, u8 *buffer, u32 count, u32 *done, u32) {
    bool ended = false;
    *done = 0;
    while (*done < count) {
        u32 got = 0, n;
        if (ended) {
            n = count - *done;
        } else {
            const u32 room = at<u32>(stream, 0x20) - vcall<u32>(inner(stream), 0x34);
            const u32 left = count - *done;
            n = left < room ? left : room;
        }
        u8 *const source = inner(stream);
        if (vcall<u32>(source, 0x1c, buffer + *done, n, &got, 1u) == 1) {
            if (ended) vcall<void>(stream, 0xc);
            return 1;
        }
        if (!ended && vcall<u32>(inner(stream), 0x34) >= at<u32>(stream, 0x20)) {
            u32 loops = at<u32>(stream, 0x14);
            if (loops != 9999) at<u32>(stream, 0x14) = --loops;
            if (!loops)
                ended = true;
            else
                vcall<void>(inner(stream), 0x30, at<u32>(stream, 0x1c));
        }
        *done += got;
    }
    if (ended) vcall<void>(stream, 0xc);
    return 0;
}

}  // extern "C"
